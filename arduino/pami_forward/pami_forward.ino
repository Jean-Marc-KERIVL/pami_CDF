// ============================================================
//  PAMI Robot — 4s moteurs (PID + obstacle + odométrie) + servo
// ============================================================
//
//  Câblage:
//    Moteur DROIT  : IN1=D19, IN2=D21
//    Moteur GAUCHE : IN1=D18, IN2=TX2 (GPIO17)
//    Sonar HC-SR04 : TRIG=D27, ECHO=D14
//    Servo SG90    : D26
//    Encoder GAUCHE: A=D23, B=D22
//    Encoder DROIT : A=D4,  B=D15
//    LED interne   : D2
//
//  Séquence:
//    1. Boot : 3 flashs + 1.5 s stabilisation
//    2. Avance 4 s en ligne droite (PID sur asymétrie d'encodeurs)
//       - stop temporaire si obstacle < 10 cm
//       - ralentit / accélère un moteur pour compenser l'autre
//    3. Stop définitif des moteurs
//    4. Servo SG90 oscille lentement (jusqu'à coupure d'alim)

#include "Pins.h"
#include "Motor.h"
#include "Drivetrain.h"
#include "UltrasonicSensor.h"
#include "Servo.h"
#include "Encoder.h"
#include "Odometry.h"

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// =========== Instances ===========
Motor motorLeft (Pins::LEFT_IN1,  Pins::LEFT_IN2,  /*inverted=*/true);
Motor motorRight(Pins::RIGHT_IN1, Pins::RIGHT_IN2, /*inverted=*/false);
Drivetrain drivetrain(motorLeft, motorRight);

UltrasonicSensor sonar(Pins::US_TRIG, Pins::US_ECHO, Config::ECHO_TIMEOUT_US);
Servo servo(Pins::SERVO, /*pulse_min_us=*/1000, /*pulse_max_us=*/2000);

Encoder encoderLeft (Pins::ENCODER1_A, Pins::ENCODER1_B);
Encoder encoderRight(Pins::ENCODER2_A, Pins::ENCODER2_B);

Odometry odometry(encoderLeft, encoderRight,
                  Config::WHEEL_DIAMETER_M,
                  Config::WHEEL_BASE_M,
                  Config::ENCODER_PULSES_PER_REV);

void IRAM_ATTR isrEncoderLeft()  { encoderLeft.handleInterrupt();  }
void IRAM_ATTR isrEncoderRight() { encoderRight.handleInterrupt(); }

// =========== Paramètres PID ===========
//   error = nb pulses encoder droit - nb pulses encoder gauche
//   correction > 0 -> on booste le gauche et on ralentit le droit
constexpr float PID_KP = 0.70f;     // augmenté: le robot déviait à gauche
constexpr float PID_KI = 0.03f;
constexpr float PID_KD = 0.25f;
constexpr int   PID_MAX_CORRECTION = 80;     // limite l'effet du PID
constexpr float PID_INTEGRAL_LIMIT = 200.0f;

// =========== Helpers ===========
void rampUp(int target_speed, int duration_ms) {
    const int steps = 20;
    int step_delay = duration_ms / steps;
    for (int i = 1; i <= steps; ++i) {
        int s = (target_speed * i) / steps;
        drivetrain.drive(s);
        odometry.update();
        delay(step_delay);
    }
}

// Avance pendant duration_ms en ligne droite via PID,
// s'arrête si obstacle proche (timer pausé).
void driveForwardPID(int base_speed, unsigned long duration_ms) {
    // Reset des encodeurs pour partir de 0
    encoderLeft.reset();
    encoderRight.reset();
    odometry.init();

    unsigned long elapsed_moving = 0;
    unsigned long last_tick = millis();
    bool moving = false;

    long  last_error      = 0;
    float error_integral  = 0.0f;

    while (elapsed_moving < duration_ms) {
        unsigned long now = millis();
        unsigned long dt  = now - last_tick;
        last_tick = now;

        long dist = sonar.readCm();
        bool obstacle = (dist > 0 && dist < Config::DIST_STOP_CM);

        if (obstacle) {
            if (moving) {
                drivetrain.stop();
                moving = false;
            }
            digitalWrite(Pins::LED, (millis() / 100) % 2);
            // Reset PID quand on s'arrête pour ne pas accumuler d'integral
            error_integral = 0.0f;
            last_error = 0;
        } else {
            // === PID asymétrie d'encodeurs ===
            long error = encoderRight.getCount() - encoderLeft.getCount();
            error_integral += error * (dt / 1000.0f);
            if (error_integral >  PID_INTEGRAL_LIMIT) error_integral =  PID_INTEGRAL_LIMIT;
            if (error_integral < -PID_INTEGRAL_LIMIT) error_integral = -PID_INTEGRAL_LIMIT;
            long derivative = error - last_error;
            last_error = error;

            float correction_f = PID_KP * error
                               + PID_KI * error_integral
                               + PID_KD * derivative;
            int correction = (int)correction_f;
            if (correction >  PID_MAX_CORRECTION) correction =  PID_MAX_CORRECTION;
            if (correction < -PID_MAX_CORRECTION) correction = -PID_MAX_CORRECTION;

            int left_speed  = base_speed + correction;
            int right_speed = base_speed - correction;
            left_speed  = constrain(left_speed,  0, 255);
            right_speed = constrain(right_speed, 0, 255);

            drivetrain.tankDrive(left_speed, right_speed);
            moving = true;
            elapsed_moving += dt;
            digitalWrite(Pins::LED, HIGH);
        }

        odometry.update();
        delay(20);
    }
    drivetrain.stop();
}

// Balayage doux du servo
void sweep(int from_deg, int to_deg, int step_deg, int step_delay_ms) {
    int step = (to_deg > from_deg) ? step_deg : -step_deg;
    int a = from_deg;
    while ((step > 0 && a < to_deg) || (step < 0 && a > to_deg)) {
        servo.setAngle(a);
        a += step;
        delay(step_delay_ms);
    }
    servo.setAngle(to_deg);
}

// =========== setup ===========
void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    pinMode(Pins::LED, OUTPUT);

    drivetrain.begin();
    servo.begin();                // pas de position initiale forcée
    sonar.begin();

    encoderLeft .begin(isrEncoderLeft);
    encoderRight.begin(isrEncoderRight);
    odometry.init();

    drivetrain.stop();

    // 3 flashs = boot OK
    for (int i = 0; i < 3; ++i) {
        digitalWrite(Pins::LED, HIGH); delay(120);
        digitalWrite(Pins::LED, LOW);  delay(120);
    }

    delay(1500);

    // === Avance 4 s en ligne droite (PID) ===
    rampUp(180, 400);
    driveForwardPID(Config::SPEED_CRUISE, 4000);     // 4 secondes
    drivetrain.stop();
    digitalWrite(Pins::LED, LOW);

    delay(500);
}

// =========== loop ===========
// Servo : incréments relatifs +90 puis -90, très doucement pour
// limiter les appels de courant.
//   step=1 toutes les 70 ms -> ~6.3 s par mouvement de 90°
void loop() {
    static int current_angle = 90;          // position interne (arbitraire au boot)

    // === +90 ticks ===
    digitalWrite(Pins::LED, HIGH);
    int target = constrain(current_angle + 90, 0, 180);
    sweep(current_angle, target, /*step=*/1, /*step_delay=*/70);
    current_angle = target;
    delay(800);                              // pause à l'extrémité

    // === -90 ticks ===
    digitalWrite(Pins::LED, LOW);
    target = constrain(current_angle - 90, 0, 180);
    sweep(current_angle, target, /*step=*/1, /*step_delay=*/70);
    current_angle = target;
    delay(800);                              // pause à l'autre extrémité
}
