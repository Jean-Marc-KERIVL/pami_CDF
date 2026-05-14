// ============================================================
//  PAMI Robot — Séquence match
// ============================================================
//
//  Comportement:
//    1. WAIT_STARTER : attend que le starter soit retiré (D32 -> HIGH)
//    2. COUNTDOWN    : attend 90 secondes
//    3. FORWARD_1    : avance pendant 10 s
//    4. UTURN        : demi-tour (~1.5 s, à calibrer)
//    5. FORWARD_2    : avance pendant 10 s
//    6. DONE         : arrêt définitif
//
//  Sécurité obstacle:
//    À tout moment (sauf pendant UTURN), si distance < 10 cm
//    -> moteurs coupés, le temps de phase est PAUSÉ.
//    Quand l'obstacle dégage (>13 cm), reprise normale.

#include "Pins.h"
#include "Motor.h"
#include "Drivetrain.h"
#include "UltrasonicSensor.h"

// =========== Instances globales ===========
Motor motorLeft (Pins::LEFT_IN1,  Pins::LEFT_IN2,  /*inverted=*/true);
Motor motorRight(Pins::RIGHT_IN1, Pins::RIGHT_IN2, /*inverted=*/false);
Drivetrain drivetrain(motorLeft, motorRight);

UltrasonicSensor sonar(Pins::US_TRIG, Pins::US_ECHO, Config::ECHO_TIMEOUT_US);

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
static unsigned long phase_elapsed_ms = 0;   // temps effectif dans la phase
static bool          blocked          = false;

const char* phaseName(Phase p) {
    switch (p) {
        case Phase::WAIT_STARTER: return "WAIT_STARTER";
        case Phase::COUNTDOWN:    return "COUNTDOWN";
        case Phase::FORWARD_1:    return "FORWARD_1";
        case Phase::UTURN:        return "UTURN";
        case Phase::FORWARD_2:    return "FORWARD_2";
        case Phase::DONE:         return "DONE";
    }
    return "?";
}

// Distance courante et détection d'obstacle avec hystérésis
static bool obstacleDetected(long dist_cm) {
    if (dist_cm < 0) return false;
    if (blocked) return dist_cm < (Config::DIST_STOP_CM + Config::DIST_HYST_CM);
    return dist_cm < Config::DIST_STOP_CM;
}

// Lit le starter (avec pull-up): LOW = en place, HIGH = retiré
static bool starterReleased() {
    return digitalRead(Pins::STARTER) == HIGH;
}

// =========== setup ===========
void setup() {
    Serial.begin(Config::SERIAL_BAUD);
    delay(200);
    Serial.println();
    Serial.println("=================================");
    Serial.println(" PAMI Match Sequence");
    Serial.println("=================================");

    pinMode(Pins::LED, OUTPUT);
    pinMode(Pins::STARTER, INPUT_PULLUP);

    drivetrain.begin();
    sonar.begin();
    drivetrain.stop();
}

// =========== loop ===========
void loop() {
    static unsigned long t_last_tick  = millis();
    static unsigned long t_last_print = 0;

    unsigned long now = millis();
    unsigned long dt  = now - t_last_tick;
    t_last_tick = now;

    long dist = sonar.readCm();
    blocked = obstacleDetected(dist);

    // Pendant le demi-tour, on IGNORE l'obstacle (sinon on reste bloqué)
    bool ignore_obstacle = (current_phase == Phase::UTURN);
    bool safe = ignore_obstacle || !blocked;

    // ===== State machine =====
    switch (current_phase) {

        case Phase::WAIT_STARTER:
            drivetrain.stop();
            digitalWrite(Pins::LED, (millis() / 500) % 2);  // clignote lent
            if (starterReleased()) {
                Serial.println(">> Starter retiré, départ du compte à rebours 90s");
                current_phase    = Phase::COUNTDOWN;
                phase_elapsed_ms = 0;
            }
            break;

        case Phase::COUNTDOWN:
            drivetrain.stop();
            digitalWrite(Pins::LED, (millis() / 200) % 2);  // clignote rapide
            phase_elapsed_ms += dt;
            if (phase_elapsed_ms >= Config::WAIT_AFTER_STARTER_MS) {
                Serial.println(">> 90s écoulés - AVANCE 1");
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
                Serial.println(">> 10s écoulés - DEMI-TOUR");
                current_phase    = Phase::UTURN;
                phase_elapsed_ms = 0;
            }
            break;

        case Phase::UTURN:
            // Demi-tour sur place. On IGNORE l'obstacle.
            drivetrain.turnInPlace(Config::SPEED_TURN);
            phase_elapsed_ms += dt;
            digitalWrite(Pins::LED, (millis() / 80) % 2);
            if (phase_elapsed_ms >= Config::UTURN_DURATION_MS) {
                Serial.println(">> Demi-tour fini - AVANCE 2");
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
                Serial.println(">> Terminé - STOP");
                current_phase = Phase::DONE;
            }
            break;

        case Phase::DONE:
            drivetrain.stop();
            digitalWrite(Pins::LED, LOW);
            break;
    }

    // ===== Log périodique =====
    if (now - t_last_print >= Config::PRINT_PERIOD_MS) {
        t_last_print = now;
        Serial.printf("[%s] elapsed=%lu ms | dist=%ld cm | blocked=%d | starter=%s\n",
                      phaseName(current_phase),
                      phase_elapsed_ms,
                      dist,
                      (int)blocked,
                      starterReleased() ? "OUT" : "IN");
    }

    delay(Config::LOOP_PERIOD_MS);
}
