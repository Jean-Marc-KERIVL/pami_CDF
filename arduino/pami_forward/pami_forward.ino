// ============================================================
//  PAMI Robot — Match simple : avance tout droit
// ============================================================
//
//  ⚠️ Pas de Serial : LEFT_IN2 est sur TX (GPIO1).
//  Retour visuel uniquement via la LED interne D2.
//
//  Câblage:
//    Moteur DROIT  : IN1=D19, IN2=D21
//    Moteur GAUCHE : IN1=D18, IN2=TX (GPIO1)
//    Ultrasons     : TRIG=D27, ECHO=D14
//    Starter       : D32 (INPUT_PULLUP) — GND par défaut, retiré pour démarrer
//    LED interne   : D2
//
//  Séquence:
//    Au boot:
//      - starter en place (LOW au boot) -> WAIT_STARTER, puis 90 s, puis FORWARD
//      - starter retiré (HIGH au boot)  -> départ IMMÉDIAT (skip 90 s)
//    Puis: avance 10 s -> STOP définitif
//
//  Sécurité obstacle:
//    distance < 10 cm pendant FORWARD -> moteurs coupés (timer pausé)
//    Reprise dès que > 13 cm.

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
    FORWARD,
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

// Starter en pull-up : LOW = en place (GND), HIGH = retiré
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
    servo.begin();         // position neutre (90°)
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
        current_phase = Phase::FORWARD;
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
                current_phase    = Phase::FORWARD;
                phase_elapsed_ms = 0;
            }
            break;

        case Phase::FORWARD:
            if (blocked) {
                drivetrain.stop();
                digitalWrite(Pins::LED, (millis() / 100) % 2);   // clignote vite
            } else {
                drivetrain.drive(Config::SPEED_CRUISE);
                phase_elapsed_ms += dt;
                digitalWrite(Pins::LED, HIGH);
            }
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
            // LED suit le servo
            digitalWrite(Pins::LED, (millis() / Config::SERVO_TOGGLE_PERIOD_MS) % 2);
            break;
    }

    delay(Config::LOOP_PERIOD_MS);
}
