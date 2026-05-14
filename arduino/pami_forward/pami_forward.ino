// ============================================================
//  PAMI Robot — 5s moteurs (avec obstacle stop) + servo en boucle
// ============================================================
//
//  Câblage:
//    Moteur DROIT  : IN1=D19, IN2=D21
//    Moteur GAUCHE : IN1=D18, IN2=TX2 (GPIO17)
//    Sonar HC-SR04 : TRIG=D27, ECHO=D14
//    Servo SG90    : D26
//    LED interne   : D2
//
//  Séquence:
//    1. Boot : 3 flashs + 1.5 s de stabilisation
//    2. Les 2 moteurs avancent (ramp-up) pendant 5 s
//       -> si obstacle < 10 cm : moteurs coupés (timer pausé)
//    3. Stop définitif des moteurs
//    4. Servo SG90 balaie 0° <-> 90° jusqu'à coupure d'alim

#include "Pins.h"
#include "Motor.h"
#include "Drivetrain.h"
#include "UltrasonicSensor.h"
#include "Servo.h"

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// =========== Instances ===========
Motor motorLeft (Pins::LEFT_IN1,  Pins::LEFT_IN2,  /*inverted=*/true);
Motor motorRight(Pins::RIGHT_IN1, Pins::RIGHT_IN2, /*inverted=*/false);
Drivetrain drivetrain(motorLeft, motorRight);

UltrasonicSensor sonar(Pins::US_TRIG, Pins::US_ECHO, Config::ECHO_TIMEOUT_US);
Servo servo(Pins::SERVO, /*pulse_min_us=*/1000, /*pulse_max_us=*/2000);

// Démarrage progressif des moteurs pour limiter le pic de courant
void rampUp(int target_speed, int duration_ms) {
    const int steps = 20;
    int step_delay = duration_ms / steps;
    for (int i = 1; i <= steps; ++i) {
        int s = (target_speed * i) / steps;
        drivetrain.drive(s);
        delay(step_delay);
    }
}

// Avance pendant duration_ms, en s'arrêtant si obstacle proche.
// Le temps "à l'arrêt sur obstacle" n'est pas compté.
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
            digitalWrite(Pins::LED, (millis() / 100) % 2);   // clignote rapide
        } else {
            if (!moving) {
                drivetrain.drive(speed);
                moving = true;
            }
            elapsed_moving += dt;
            digitalWrite(Pins::LED, HIGH);
        }

        delay(30);
    }
    drivetrain.stop();
}

// Balayage doux du servo (pas de mouvement brutal)
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

void setup() {
    // 🛡️ Désactive le détecteur de brown-out (évite reset sur pic de courant)
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    pinMode(Pins::LED, OUTPUT);

    drivetrain.begin();
    servo.begin();
    sonar.begin();
    drivetrain.stop();

    // 3 flashs = boot OK
    for (int i = 0; i < 3; ++i) {
        digitalWrite(Pins::LED, HIGH); delay(120);
        digitalWrite(Pins::LED, LOW);  delay(120);
    }

    delay(1500);   // stabilisation alim

    // === Démarrage progressif puis avance 5 s (stop sur obstacle) ===
    rampUp(180, 400);
    driveForwardWithObstacle(Config::SPEED_CRUISE, 5000);
    drivetrain.stop();
    digitalWrite(Pins::LED, LOW);

    delay(500);
}

void loop() {
    // Servo : balayage doux mais plus rapide que la version précédente
    // step=4, delay=18 ms -> ~400 ms pour 90°
    digitalWrite(Pins::LED, HIGH);
    sweep(0, 90, /*step=*/4, /*step_delay=*/18);
    delay(200);

    digitalWrite(Pins::LED, LOW);
    sweep(90, 0, /*step=*/4, /*step_delay=*/18);
    delay(200);
}
