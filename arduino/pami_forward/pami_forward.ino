// ============================================================
//  PAMI Robot — TEST COURT : 2 moteurs 2s + servo SG90
// ============================================================
//
//  ⚠️ Pas de Serial : LEFT_IN2 est sur TX (GPIO1).
//
//  Câblage:
//    Moteur DROIT  : IN1=D19, IN2=D21
//    Moteur GAUCHE : IN1=D18, IN2=TX (GPIO1)
//    Servo SG90    : D26
//    LED interne   : D2
//
//  Séquence (au boot):
//    1. Délai 1 s (sécurité)
//    2. Les 2 moteurs avancent ensemble pendant 5 s
//    3. Stop définitif des moteurs (jusqu'à coupure d'alim)
//    4. Le servo SG90 oscille 0° <-> 90° en boucle (jusqu'à coupure d'alim)

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
    servo.begin();          // position neutre (90°)
    drivetrain.stop();

    // 3 flashs = boot OK
    for (int i = 0; i < 3; ++i) {
        digitalWrite(Pins::LED, HIGH); delay(120);
        digitalWrite(Pins::LED, LOW);  delay(120);
    }

    // Délai de sécurité avant départ
    delay(1000);

    // === ÉTAPE 1 : les 2 moteurs avancent 5 s ===
    digitalWrite(Pins::LED, HIGH);
    drivetrain.drive(Config::SPEED_CRUISE);
    delay(5000);
    drivetrain.stop();
    digitalWrite(Pins::LED, LOW);

    // Petite pause avant le servo
    delay(500);
}

// =========== loop ===========
void loop() {
    // === ÉTAPE 2 : servo s'agite indéfiniment (jusqu'à coupure d'alim) ===
    //  amplitude 0° -> 90° toutes les 350 ms
    servo.toggleUpDown(350, 0, 90);
    digitalWrite(Pins::LED, (millis() / 350) % 2);
    delay(20);
}
