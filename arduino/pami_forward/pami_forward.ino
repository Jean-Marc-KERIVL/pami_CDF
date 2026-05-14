// ============================================================
//  PAMI Robot — TEST SERVO seul
// ============================================================
//
//  Le servo SG90 (D26) oscille 0° <-> 90° en boucle infinie.
//  Les moteurs de roues ne sont pas activés.
//
//  Câblage:
//    Servo SG90    : D26
//    LED interne   : D2

#include "Pins.h"
#include "Servo.h"

// SG90 : pulse 1000 µs (0°) -> 2000 µs (180°)
Servo servo(Pins::SERVO, /*pulse_min_us=*/1000, /*pulse_max_us=*/2000);

// =========== setup ===========
void setup() {
    pinMode(Pins::LED, OUTPUT);

    servo.begin();          // position initiale 90°

    // 3 flashs = boot OK
    for (int i = 0; i < 3; ++i) {
        digitalWrite(Pins::LED, HIGH); delay(120);
        digitalWrite(Pins::LED, LOW);  delay(120);
    }

    // Démarrage à 0° avant la boucle
    servo.setAngle(0);
    delay(500);
}

// =========== loop ===========
void loop() {
    // Oscillation 0° <-> 90° toutes les 500 ms, indéfiniment
    servo.setAngle(0);
    digitalWrite(Pins::LED, HIGH);
    delay(500);

    servo.setAngle(90);
    digitalWrite(Pins::LED, LOW);
    delay(500);
}
