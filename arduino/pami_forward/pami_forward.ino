/*
 * PAMI Robot - Test "Tout droit"
 * --------------------------------
 * Hardware:
 *   - ESP32
 *   - Module MJKDZ (L298N dual H-bridge)
 *   - 2 moteurs DC
 *
 * Câblage:
 *   Moteur DROIT: IN1=D19, IN2=D21
 *   Moteur GAUCHE: IN3=D18, IN4=TX (GPIO1)
 *
 * Comportement:
 *   1. Attendre 2 secondes au démarrage (sécurité)
 *   2. Avancer tout droit pendant 3 secondes
 *   3. S'arrêter
 *   4. Attendre 2 secondes
 *   5. Recommencer en boucle
 *
 * IMPORTANT: GPIO1 = TX0 (Serial USB).
 * Pendant le flash, débrancher D18/TX. Après flash, brancher la carte.
 * Pas de Serial.print() actif car TX est utilisé pour le moteur.
 */

// =================== CONFIG PINS ===================
// Moteur droit (via MJKDZ IN1/IN2)
const int RIGHT_IN1 = 19;   // D19
const int RIGHT_IN2 = 21;   // D21

// Moteur gauche (via MJKDZ IN3/IN4)
const int LEFT_IN1  = 18;   // D18
const int LEFT_IN2  = 1;    // TX = GPIO1

// =================== PARAMÈTRES ===================
const unsigned long STARTUP_DELAY_MS = 2000;
const unsigned long FORWARD_TIME_MS  = 3000;
const unsigned long STOP_TIME_MS     = 2000;

// LED interne pour feedback visuel
const int LED_PIN = 2;

// =================== MOTOR HELPERS ===================
void rightMotor(int in1, int in2) {
  digitalWrite(RIGHT_IN1, in1);
  digitalWrite(RIGHT_IN2, in2);
}

void leftMotor(int in1, int in2) {
  digitalWrite(LEFT_IN1, in1);
  digitalWrite(LEFT_IN2, in2);
}

void forward() {
  // Les deux roues tournent dans le même sens "avant"
  // Si une roue va dans le mauvais sens, inverse IN1/IN2 sur cette roue
  rightMotor(HIGH, LOW);
  leftMotor(HIGH, LOW);
}

void backward() {
  rightMotor(LOW, HIGH);
  leftMotor(LOW, HIGH);
}

void stopAll() {
  rightMotor(LOW, LOW);
  leftMotor(LOW, LOW);
}

// =================== SETUP / LOOP ===================
void setup() {
  pinMode(RIGHT_IN1, OUTPUT);
  pinMode(RIGHT_IN2, OUTPUT);
  pinMode(LEFT_IN1,  OUTPUT);
  pinMode(LEFT_IN2,  OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // Sécurité: tout à l'arrêt
  stopAll();

  // Délai de sécurité au démarrage
  digitalWrite(LED_PIN, HIGH);
  delay(STARTUP_DELAY_MS);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  // Avancer
  digitalWrite(LED_PIN, HIGH);
  forward();
  delay(FORWARD_TIME_MS);

  // Stop
  digitalWrite(LED_PIN, LOW);
  stopAll();
  delay(STOP_TIME_MS);
}
