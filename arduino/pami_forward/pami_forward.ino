// ============================================================
//  PAMI Robot — Forward avec vitesse adaptative HC-SR04
//  Architecture C++ : Motor / Drivetrain / UltrasonicSensor
// ============================================================
//
//  Câblage (voir Pins.h):
//    Moteur DROIT  : IN1=D19, IN2=D21
//    Moteur GAUCHE : IN1=D18, IN2=D23   ⚠️ ex-TX, à déplacer si pas déjà fait
//    Ultrasons     : TRIG=D27, ECHO=D14
//    LED interne   : D2
//
//  Comportement:
//    - >60 cm : pleine vitesse
//    - 15..60 cm : vitesse proportionnelle (linéaire)
//    - <15 cm : STOP
//    - Log temps réel sur Serial USB (115200 baud)

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

// =========== Logique ===========
static int distanceToSpeed(long dist_cm) {
    if (dist_cm < 0)                       return Config::SPEED_MAX;   // rien -> plein
    if (dist_cm <  Config::DIST_STOP_CM)   return 0;                   // proche -> stop
    if (dist_cm >= Config::DIST_FULL_CM)   return Config::SPEED_MAX;   // loin -> plein
    long s = map(dist_cm,
                 Config::DIST_STOP_CM, Config::DIST_FULL_CM,
                 Config::SPEED_MIN,    Config::SPEED_MAX);
    return constrain((int)s, Config::SPEED_MIN, Config::SPEED_MAX);
}

static void printStatus(long dist, int speed) {
    int bars = map(speed, 0, 255, 0, 20);
    char bar[22] = {0};
    for (int i = 0; i < 20; ++i) bar[i] = (i < bars) ? '#' : '-';
    if (dist < 0) {
        Serial.printf("dist=---  cm | speed=%3d/255 [%s] | %s\n",
                      speed, bar, speed == 0 ? "STOP" : "AVANCE");
    } else {
        Serial.printf("dist=%3ld cm | speed=%3d/255 [%s] | %s\n",
                      dist, speed, bar, speed == 0 ? "STOP" : "AVANCE");
    }
}

// =========== Arduino setup ===========
void setup() {
    Serial.begin(Config::SERIAL_BAUD);
    delay(200);
    Serial.println();
    Serial.println("==================================");
    Serial.println(" PAMI Robot - Forward + HC-SR04   ");
    Serial.println(" Motors: R(D19/D21)  L(D18/D23)   ");
    Serial.println(" Sonar : TRIG=D27 ECHO=D14        ");
    Serial.println("==================================");

    pinMode(Pins::LED, OUTPUT);
    digitalWrite(Pins::LED, HIGH);

    drivetrain.begin();
    sonar.begin();

    Serial.printf("Démarrage dans %lu ms...\n", Config::STARTUP_DELAY_MS);
    delay(Config::STARTUP_DELAY_MS);
    digitalWrite(Pins::LED, LOW);
    Serial.println("GO!");
}

// =========== Arduino loop ===========
void loop() {
    static unsigned long t_last_print = 0;

    long dist  = sonar.readCm();
    int  speed = distanceToSpeed(dist);

    if (speed == 0) {
        drivetrain.stop();
        digitalWrite(Pins::LED, (millis() / 100) % 2);
    } else {
        drivetrain.drive(speed);
        digitalWrite(Pins::LED, speed >= Config::SPEED_MAX
                                 ? HIGH
                                 : (millis() / (300 - speed)) % 2);
    }

    if (millis() - t_last_print >= Config::PRINT_PERIOD_MS) {
        t_last_print = millis();
        printStatus(dist, speed);
    }

    delay(Config::LOOP_PERIOD_MS);
}
