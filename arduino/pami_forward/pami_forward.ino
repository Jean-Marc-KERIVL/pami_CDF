// ============================================================
//  PAMI Robot — Starter + 4s moteurs (PID + obstacle + odométrie) + servo
// ============================================================
//
//  Câblage:
//    Moteur DROIT  : IN1=D19, IN2=D21
//    Moteur GAUCHE : IN1=D18, IN2=TX2 (GPIO17)
//    Sonar HC-SR04 : TRIG=D27, ECHO=D14
//    Servo SG90    : D26
//    Starter       : D32 (INPUT_PULLUP, GND = en place, libre = HIGH)
//    Encoder GAUCHE: A=D23, B=D22
//    Encoder DROIT : A=D4,  B=D15
//    LED interne   : D2
//
//  Séquence:
//    1. Boot : 3 flashs + 1 s stabilisation
//    2. Servo va à la position initiale (SERVO_INIT_ANGLE)
//    3. Logique starter :
//       - Starter EN PLACE au boot -> attente retrait + 90 s
//       - Starter LIBRE au boot    -> départ DIRECT (skip 90 s)
//    4. Avance 4 s en ligne droite (PID sur encodeurs, obstacle stop)
//    5. Stop définitif des moteurs
//    6. Servo SG90 oscille lentement (jusqu'à coupure d'alim)

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

// =========== Position initiale du servo ===========
constexpr int   SERVO_INIT_ANGLE = 0;        // position au démarrage

// =========== Starter ===========
constexpr unsigned long STARTER_COUNTDOWN_MS = 90000UL;  // 90 s
inline bool starterReleased() {
    return digitalRead(Pins::STARTER) == HIGH;
}

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
    servo.begin();
    servo.setAngle(SERVO_INIT_ANGLE);   // position initiale au démarrage
    sonar.begin();
    pinMode(Pins::STARTER, INPUT_PULLUP);

    encoderLeft .begin(isrEncoderLeft);
    encoderRight.begin(isrEncoderRight);
    odometry.init();

    drivetrain.stop();

    // 3 flashs = boot OK
    for (int i = 0; i < 3; ++i) {
        digitalWrite(Pins::LED, HIGH); delay(120);
        digitalWrite(Pins::LED, LOW);  delay(120);
    }

    delay(1000);

    // === Logique starter ===
    // Lecture stable (debounce)
    bool starter_in_at_boot = !starterReleased();

    if (starter_in_at_boot) {
        // Attente du retrait du starter (LED clignote lent)
        while (!starterReleased()) {
            digitalWrite(Pins::LED, (millis() / 500) % 2);
            delay(30);
        }
        // Compte à rebours 90 s (LED clignote rapide)
        unsigned long start_ts = millis();
        while (millis() - start_ts < STARTER_COUNTDOWN_MS) {
            digitalWrite(Pins::LED, (millis() / 200) % 2);
            delay(30);
        }
    }
    // Sinon : départ DIRECT

    // === Avance 4 s en ligne droite (PID) ===
    rampUp(180, 400);
    driveForwardPID(Config::SPEED_CRUISE, 4000);
    drivetrain.stop();
    digitalWrite(Pins::LED, LOW);

    delay(500);
}

// =========== loop ===========
// Servo : incréments relatifs +TICKS puis -TICKS, lentement.
// step=1 toutes les 70 ms (doux, peu d'appel de courant).
// TICKS = 220 -> au-delà des 180° physiques, le servo reste en butée
// quelques pas (effet "pause" + retour plus lent à l'autre extrémité).
constexpr int   SERVO_TICKS      = 220;
constexpr int   SERVO_STEP       = 1;
constexpr int   SERVO_STEP_DELAY = 70;     // ms entre 2 ticks
constexpr int   SERVO_PAUSE_MS   = 400;

void loop() {
    static int current_angle = 0;            // arbitraire au démarrage

    // === +TICKS (vers le haut) ===
    digitalWrite(Pins::LED, HIGH);
    int target = constrain(current_angle + SERVO_TICKS, 0, 180);
    sweep(current_angle, target, SERVO_STEP, SERVO_STEP_DELAY);
    current_angle = target;
    delay(SERVO_PAUSE_MS);

    // === -TICKS (vers le bas) ===
    digitalWrite(Pins::LED, LOW);
    target = constrain(current_angle - SERVO_TICKS, 0, 180);
    sweep(current_angle, target, SERVO_STEP, SERVO_STEP_DELAY);
    current_angle = target;
    delay(SERVO_PAUSE_MS);
}
