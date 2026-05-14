// ============================================================
//  PAMI Robot — 3s moteurs (avec obstacle + odométrie) + servo
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
//    1. Boot : 3 flashs + 1.5 s de stabilisation
//    2. Les 2 moteurs avancent pendant 3 s (rampe + obstacle stop)
//    3. Stop définitif des moteurs (la pose finale est conservée)
//    4. Servo SG90 oscille doucement jusqu'à coupure d'alim
//
//  L'odométrie est mise à jour en continu pendant la phase moteur.

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

// ISR statiques attachées à chaque encodeur (IRAM pour ESP32)
void IRAM_ATTR isrEncoderLeft()  { encoderLeft.handleInterrupt();  }
void IRAM_ATTR isrEncoderRight() { encoderRight.handleInterrupt(); }

// =========== Helpers ===========
void rampUp(int target_speed, int duration_ms) {
    const int steps = 20;
    int step_delay = duration_ms / steps;
    for (int i = 1; i <= steps; ++i) {
        int s = (target_speed * i) / steps;
        drivetrain.drive(s);
        odometry.update();          // suit la pose même en rampe
        delay(step_delay);
    }
}

// Avance pendant duration_ms en s'arrêtant si obstacle proche.
// L'odométrie est mise à jour à chaque tick.
void driveForwardWithObstacle(int speed, unsigned long duration_ms) {
    unsigned long elapsed_moving = 0;
    unsigned long last_tick = millis();
    bool moving = false;

    while (elapsed_moving < duration_ms) {
        unsigned long now = millis();
        unsigned long dt = now - last_tick;
        last_tick = now;

        long dist = sonar.readCm();
        bool obstacle = (dist > 0 && dist < Config::DIST_STOP_CM);

        if (obstacle) {
            if (moving) {
                drivetrain.stop();
                moving = false;
            }
            digitalWrite(Pins::LED, (millis() / 100) % 2);
        } else {
            if (!moving) {
                drivetrain.drive(speed);
                moving = true;
            }
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
    // 🛡️ Désactive le détecteur de brown-out
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    pinMode(Pins::LED, OUTPUT);

    drivetrain.begin();
    servo.begin();
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

    // === Avance 3 s ===
    odometry.resetPose();
    rampUp(180, 400);
    driveForwardWithObstacle(Config::SPEED_CRUISE, 3000);    // 3 secondes
    drivetrain.stop();
    digitalWrite(Pins::LED, LOW);

    // À ce point, l'odométrie contient :
    //   odometry.getX(), getY(), getTheta(), getDistanceM()

    delay(500);
}

// =========== loop ===========
void loop() {
    digitalWrite(Pins::LED, HIGH);
    sweep(0, 90, /*step=*/2, /*step_delay=*/25);
    delay(300);

    digitalWrite(Pins::LED, LOW);
    sweep(90, 0, /*step=*/2, /*step_delay=*/25);
    delay(300);
}
