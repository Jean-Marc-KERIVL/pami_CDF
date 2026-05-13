// ============================================================
//  PAMI Scanner — Détecte automatiquement sur quels pins
//  est branché le capteur HC-SR04
// ============================================================
//
//  Comment ça marche:
//    Pour chaque paire de pins candidats (TRIG, ECHO):
//      - Envoie un pulse 10µs sur le pin TRIG candidat
//      - Mesure la durée d'écho sur le pin ECHO candidat
//      - Si on obtient une distance cohérente (5-300 cm),
//        c'est probablement la bonne paire.
//
//  ⚠️ Important:
//    - Avant de flasher: DÉBRANCHE les moteurs (sécurité)
//    - VCC du HC-SR04 sur 5V (pas 3.3V !)
//    - GND commun ESP32 et HC-SR04
//    - Pose un obstacle ~20 cm devant le capteur pendant le scan

#include <Arduino.h>

// Pins à tester (on évite TX=1, RX=3, LED=2, et les pins en INPUT-only 34/35/36/39)
const int CANDIDATE_PINS[] = {
    4, 5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33
};
constexpr int N_PINS = sizeof(CANDIDATE_PINS) / sizeof(CANDIDATE_PINS[0]);

// Critères pour valider un écho
constexpr long  MIN_DIST_CM        = 3;
constexpr long  MAX_DIST_CM        = 400;
constexpr unsigned long ECHO_TO_US = 30000;   // ~5 m

long measure(int trig, int echo) {
    pinMode(trig, OUTPUT);
    pinMode(echo, INPUT);

    digitalWrite(trig, LOW);
    delayMicroseconds(3);
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);

    unsigned long d = pulseIn(echo, HIGH, ECHO_TO_US);
    if (d == 0) return -1;
    return (long)(d * 0.0343f / 2.0f);
}

void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.println();
    Serial.println("========================================");
    Serial.println(" PAMI Scanner - HC-SR04 pin detection");
    Serial.println("========================================");
    Serial.println(" Pose un obstacle ~15-30 cm devant");
    Serial.println(" le capteur pendant le scan.");
    Serial.println();
    Serial.print(" Pins testés: ");
    for (int i = 0; i < N_PINS; ++i) {
        Serial.print("D"); Serial.print(CANDIDATE_PINS[i]); Serial.print(" ");
    }
    Serial.println();
    Serial.println("----------------------------------------");
}

void loop() {
    Serial.println();
    Serial.println(">>> Nouveau scan <<<");

    int n_hits = 0;

    for (int i = 0; i < N_PINS; ++i) {
        for (int j = 0; j < N_PINS; ++j) {
            if (i == j) continue;

            int trig = CANDIDATE_PINS[i];
            int echo = CANDIDATE_PINS[j];

            // Trois mesures pour valider
            long d1 = measure(trig, echo);
            delay(15);
            long d2 = measure(trig, echo);
            delay(15);
            long d3 = measure(trig, echo);
            delay(15);

            // Critère: au moins 2 mesures sur 3 dans [3..400 cm] et cohérentes
            int valid = 0;
            long sum = 0;
            if (d1 >= MIN_DIST_CM && d1 <= MAX_DIST_CM) { valid++; sum += d1; }
            if (d2 >= MIN_DIST_CM && d2 <= MAX_DIST_CM) { valid++; sum += d2; }
            if (d3 >= MIN_DIST_CM && d3 <= MAX_DIST_CM) { valid++; sum += d3; }

            if (valid >= 2) {
                long avg = sum / valid;
                Serial.printf("  ✅  TRIG=D%-2d  ECHO=D%-2d  -> %ld cm  (d1=%ld d2=%ld d3=%ld)\n",
                              trig, echo, avg, d1, d2, d3);
                n_hits++;
            }
        }
    }

    if (n_hits == 0) {
        Serial.println("  ❌  Aucune paire détectée.");
        Serial.println("       Vérifie l'alimentation 5V du HC-SR04");
        Serial.println("       et que le capteur est bien branché.");
    } else {
        Serial.printf("  >> %d paire(s) candidate(s) détectée(s)\n", n_hits);
    }

    delay(2000);
}
