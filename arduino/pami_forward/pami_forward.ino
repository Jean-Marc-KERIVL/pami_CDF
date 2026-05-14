// ============================================================
//  PAMI Robot — Séquence match
// ============================================================
//
//  ⚠️ Pas de Serial.print : LEFT_IN2 est sur TX (GPIO1).
//  Utiliser le retour visuel via la LED interne (D2).
//
//  Câblage:
//    Moteur DROIT  : IN1=D19, IN2=D21
//    Moteur GAUCHE : IN1=D18, IN2=TX (GPIO1)
//    Ultrasons     : TRIG=D27, ECHO=D14
//    Servo         : D26 (réservé)
//    Starter       : D32 (INPUT_PULLUP) — GND par défaut, retiré pour démarrer
//    LED interne   : D2
//
//  Séquence:
//    Au boot:
//      - starter en place (LOW) -> WAIT_STARTER, puis 90 s, puis avance
//      - starter retiré (HIGH)  -> départ IMMÉDIAT (skip 90 s)
//    Puis: avance 10 s -> demi-tour -> avance 10 s -> stop
//
//  Sécurité obstacle:
//    distance < 10 cm pendant FORWARD_1/2 -> moteurs coupés
//    (le timer de phase est pausé ; reprend dès que l'obstacle est > 13 cm)
//    Pendant le demi-tour : obstacle ignoré

#include "Pins.h"
#include "Motor.h"
#include "Drivetrain.h"
#include "UltrasonicSensor.h"
#include "Servo.h"

// =========== Instances globales ===========
Motor motorLeft (Pins::LEFT_IN1,  Pins::LEFT_IN2,  /*inverted=*/true);
Motor motorRight(Pins::RIGHT_IN1, Pins::RIGHT_IN2, /*inverted=*/false);
Drivetrain drivetrain(motorLeft, motorRight);

UltrasonicSensor sonar(Pins::US_TRIG, Pins::US_ECHO, Config::ECHO_TIMEOUT_US);
Servo servo(Pins::SERVO);

// =========== State machine ===========
enum class Phase {
    WAIT_STARTER,
    COUNTDOWN,
    FORWARD_1,
    UTURN,
    FORWARD_2,
    DONE
};

static Phase         current_phase    = Phase::WAIT_STARTER;
static unsigned long phase_elapsed_ms = 0;
static bool          blocked          = false;

// Hystérésis sur la détection d'obstacle
static bool obstacleDetected(long dist_cm) {
    if (dist_cm < 0) return false;
    if (blocked) return dist_cm < (Config::DIST_STOP_CM + Config::DIST_HYST_CM);
    return dist_cm < Config::DIST_STOP_CM;
}

// Starter en pull-up : LOW = en place, HIGH = retiré
static bool starterReleased() {
    return digitalRead(Pins::STARTER) == HIGH;
}

// Auto-test moteurs au démarrage : 600 ms par moteur
static void motorSelfTest() {
    drivetrain.tankDrive(180, 0);    // moteur gauche seul
    delay(600);
    drivetrain.stop();
    delay(300);

    drivetrain.tankDrive(0, 180);    // moteur droit seul
    delay(600);
    drivetrain.stop();
    delay(300);

    drivetrain.drive(180);           // les deux
    delay(600);
    drivetrain.stop();
    delay(300);
}

// =========== setup ===========
void setup() {
    pinMode(Pins::LED, OUTPUT);
    pinMode(Pins::STARTER, INPUT_PULLUP);

    drivetrain.begin();
    sonar.begin();
    servo.begin();           // initialise le servo (position neutre 90°)
    drivetrain.stop();

    // 3 flashs rapides = boot OK
    for (int i = 0; i < 3; ++i) {
        digitalWrite(Pins::LED, HIGH);
        delay(120);
        digitalWrite(Pins::LED, LOW);
        delay(120);
    }

    delay(500);   // stabilisation pull-up
    motorSelfTest();

    // Choix de la phase initiale
    if (starterReleased()) {
        // Pas de starter au boot -> départ immédiat
        current_phase = Phase::FORWARD_1;
    } else {
        // Starter en place au boot -> attente
        current_phase = Phase::WAIT_STARTER;
    }
    phase_elapsed_ms = 0;
}

// =========== loop ===========
void loop() {
    static unsigned long t_last_tick = millis();

    unsigned long now = millis();
    unsigned long dt  = now - t_last_tick;
    t_last_tick = now;

    long dist = sonar.readCm();
    blocked = obstacleDetected(dist);
    bool safe = (current_phase == Phase::UTURN) || !blocked;

    switch (current_phase) {

        case Phase::WAIT_STARTER:
            drivetrain.stop();
            digitalWrite(Pins::LED, (millis() / 500) % 2);   // lent
            if (starterReleased()) {
                current_phase    = Phase::COUNTDOWN;
                phase_elapsed_ms = 0;
            }
            break;

        case Phase::COUNTDOWN:
            drivetrain.stop();
            digitalWrite(Pins::LED, (millis() / 200) % 2);   // rapide
            phase_elapsed_ms += dt;
            if (phase_elapsed_ms >= Config::WAIT_AFTER_STARTER_MS) {
                current_phase    = Phase::FORWARD_1;
                phase_elapsed_ms = 0;
            }
            break;

        case Phase::FORWARD_1:
            if (safe) {
                drivetrain.drive(Config::SPEED_CRUISE);
                phase_elapsed_ms += dt;
            } else {
                drivetrain.stop();
            }
            digitalWrite(Pins::LED, safe ? HIGH : ((millis() / 100) % 2));
            if (phase_elapsed_ms >= Config::FORWARD_DURATION_MS) {
                current_phase    = Phase::UTURN;
                phase_elapsed_ms = 0;
            }
            break;

        case Phase::UTURN:
            // Demi-tour : obstacle ignoré
            drivetrain.turnInPlace(Config::SPEED_TURN);
            phase_elapsed_ms += dt;
            digitalWrite(Pins::LED, (millis() / 80) % 2);    // très rapide
            if (phase_elapsed_ms >= Config::UTURN_DURATION_MS) {
                drivetrain.stop();
                current_phase    = Phase::FORWARD_2;
                phase_elapsed_ms = 0;
            }
            break;

        case Phase::FORWARD_2:
            if (safe) {
                drivetrain.drive(Config::SPEED_CRUISE);
                phase_elapsed_ms += dt;
            } else {
                drivetrain.stop();
            }
            digitalWrite(Pins::LED, safe ? HIGH : ((millis() / 100) % 2));
            if (phase_elapsed_ms >= Config::FORWARD_DURATION_MS) {
                current_phase = Phase::DONE;
            }
            break;

        case Phase::DONE:
            drivetrain.stop();
            // Servo oscille haut/bas indéfiniment
            servo.toggleUpDown(Config::SERVO_TOGGLE_PERIOD_MS,
                               Config::SERVO_ANGLE_LOW,
                               Config::SERVO_ANGLE_HIGH);
            // LED suit le servo : ON quand haut, OFF quand bas
            digitalWrite(Pins::LED, (millis() / Config::SERVO_TOGGLE_PERIOD_MS) % 2);
            break;
    }

    delay(Config::LOOP_PERIOD_MS);
}
