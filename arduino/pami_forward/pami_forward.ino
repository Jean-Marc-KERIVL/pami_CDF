// ============================================================
//  PAMI Robot — Avance + arrêt automatique sur obstacle HC-SR04
//  Architecture C++ : Motor / Drivetrain / UltrasonicSensor
// ============================================================
//
//  Câblage (voir Pins.h):
//    Moteur DROIT  : IN1=D19, IN2=D21
//    Moteur GAUCHE : IN1=D18, IN2=D23
//    Ultrasons     : TRIG=D27, ECHO=D14
//    LED interne   : D2
//
//  Comportement:
//    - distance >= 20 cm OU pas d'écho : avance à SPEED_CRUISE
//    - distance <  20 cm               : STOP (LED clignote)

#include "Pins.h"
#include "Motor.h"
#include "Drivetrain.h"
#include "UltrasonicSensor.h"

// =========== Instances globales ===========
// Moteur gauche déclaré "inverted" car monté en miroir physiquement
Motor motorLeft (Pins::LEFT_IN1,  Pins::LEFT_IN2,  /*inverted=*/true);
Motor motorRight(Pins::RIGHT_IN1, Pins::RIGHT_IN2, /*inverted=*/false);
Drivetrain drivetrain(motorLeft, motorRight);

UltrasonicSensor sonar(Pins::US_TRIG, Pins::US_ECHO, Config::ECHO_TIMEOUT_US);

// =========== État ===========
enum class State { CRUISING, BLOCKED };

// Décide entre AVANCE et STOP avec un peu d'hystérésis pour éviter
// les oscillations rapides à la frontière du seuil.
static bool isObstacleClose(long dist_cm, State current) {
    if (dist_cm < 0) return false; // pas d'écho -> chemin libre

    if (current == State::CRUISING) {
        // On avance: on s'arrête dès qu'on voit l'obstacle au seuil
        return dist_cm < Config::DIST_STOP_CM;
    } else {
        // On est arrêté: on repart seulement si on est nettement plus loin
        return dist_cm < (Config::DIST_STOP_CM + 5);  // +5 cm marge
    }
}

// =========== Arduino setup ===========
void setup() {
    pinMode(Pins::LED, OUTPUT);
    digitalWrite(Pins::LED, HIGH);

    drivetrain.begin();
    sonar.begin();

    // Sécurité au démarrage
    delay(Config::STARTUP_DELAY_MS);
    digitalWrite(Pins::LED, LOW);
}

// =========== Arduino loop ===========
void loop() {
    static State state = State::CRUISING;

    long dist = sonar.readCm();

    if (isObstacleClose(dist, state)) {
        // Obstacle proche → STOP
        if (state != State::BLOCKED) {
            drivetrain.stop();
            state = State::BLOCKED;
        }
        // LED clignote rapidement pour signaler le stop
        digitalWrite(Pins::LED, (millis() / 120) % 2);
    } else {
        // Chemin libre → AVANCE
        if (state != State::CRUISING) {
            state = State::CRUISING;
        }
        drivetrain.drive(Config::SPEED_CRUISE);
        digitalWrite(Pins::LED, HIGH);
    }

    delay(Config::LOOP_PERIOD_MS);
}
