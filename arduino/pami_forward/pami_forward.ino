// ============================================================
//  PAMI Robot — 5s moteurs + servo en boucle infinie
// ============================================================
//
//  Câblage:
//    Moteur DROIT  : IN1=D19, IN2=D21
//    Moteur GAUCHE : IN1=D18, IN2=TX2 (GPIO17)
//    Servo SG90    : D26
//    LED interne   : D2
//
//  Séquence:
//    1. Boot : 3 flashs LED
//    2. Délai 1 s (sécurité)
//    3. Les 2 moteurs avancent ensemble pendant 5 s
//    4. Moteurs stop définitif
//    5. Servo SG90 oscille 0° <-> 90° (500 ms chaque), jusqu'à coupure d'alim

#include "Pins.h"
#include "Motor.h"
#include "Drivetrain.h"
#include "Servo.h"

// =========== Instances ===========
Motor motorLeft (Pins::LEFT_IN1,  Pins::LEFT_IN2,  /*inverted=*/true);
Motor motorRight(Pins::RIGHT_IN1, Pins::RIGHT_IN2, /*inverted=*/false);
Drivetrain drivetrain(motorLeft, motorRight);

// SG90 : pulse 1000 µs (0°) -> 2000 µs (180°)
Servo servo(Pins::SERVO, /*pulse_min_us=*/1000, /*pulse_max_us=*/2000);

// =========== setup ===========
void setup() {
    pinMode(Pins::LED, OUTPUT);

    drivetrain.begin();
    servo.begin();
    drivetrain.stop();

    // 3 flashs = boot OK
    for (int i = 0; i < 3; ++i) {
        digitalWrite(Pins::LED, HIGH); delay(120);
        digitalWrite(Pins::LED, LOW);  delay(120);
    }

    // Délai de sécurité
    delay(1000);

    // === Les 2 moteurs avancent 5 s ===
    digitalWrite(Pins::LED, HIGH);
    drivetrain.drive(Config::SPEED_CRUISE);
    delay(5000);
    drivetrain.stop();
    digitalWrite(Pins::LED, LOW);

    // Petite pause avant le servo
    delay(500);
}

// =========== loop ===========
// Servo en boucle infinie. La loop() est appelée à répétition par Arduino,
// donc l'oscillation continue indéfiniment.
void loop() {
    servo.setAngle(0);
    digitalWrite(Pins::LED, HIGH);
    delay(500);

    servo.setAngle(90);
    digitalWrite(Pins::LED, LOW);
    delay(500);
}
