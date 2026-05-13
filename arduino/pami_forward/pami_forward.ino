/*
 * PAMI Robot - Avancer avec vitesse adaptative
 * --------------------------------------------
 * Hardware:
 *   - ESP32
 *   - Module MJKDZ (L298N dual H-bridge)
 *   - 2 moteurs DC
 *   - Capteur ultrasons HC-SR04
 *
 * Câblage moteurs (via MJKDZ):
 *   DROIT:  IN1=D19, IN2=D21
 *   GAUCHE: IN1=D18, IN2=TX (GPIO1)   [inversé en logiciel]
 *
 * Câblage capteur:
 *   TRIG = D27
 *   ECHO = D14   ⚠️ HC-SR04 sort 5V — idéalement diviseur 1k/2k vers ECHO
 *   VCC  = 5V, GND = GND
 *
 * Comportement:
 *   - distance > 60 cm : pleine vitesse
 *   - 15 → 60 cm       : vitesse proportionnelle (plus c'est près, plus c'est lent)
 *   - distance < 15 cm : ARRÊT
 *   - Mesure régulière, asservissement temps réel
 */

// =================== CONFIG PINS ===================
const int RIGHT_IN1 = 19;
const int RIGHT_IN2 = 21;
const int LEFT_IN1  = 18;
const int LEFT_IN2  = 1;     // TX (GPIO1)

const int TRIG_PIN  = 27;
const int ECHO_PIN  = 14;

const int LED_PIN   = 2;     // LED interne ESP32

// =================== PARAMÈTRES ===================
const int PWM_FREQ        = 1000;   // 1 kHz (bon pour L298N)
const int PWM_RES         = 8;      // 8 bits (0-255)

const int DIST_STOP_CM    = 15;     // < 15 cm -> stop
const int DIST_FULL_CM    = 60;     // > 60 cm -> pleine vitesse
const int SPEED_MIN       = 90;     // vitesse mini pour démarrer le moteur
const int SPEED_MAX       = 255;    // vitesse max

const unsigned long STARTUP_DELAY_MS = 2000;
const unsigned long LOOP_PERIOD_MS   = 50;   // mesure 20 fois/sec
const unsigned long ECHO_TIMEOUT_US  = 25000; // ~4 m max

// =================== HELPERS PWM ===================
void motorWrite(int in1, int in2, int duty_in1, int duty_in2) {
  ledcWrite(in1, duty_in1);
  ledcWrite(in2, duty_in2);
}

void forwardSpeed(int speed) {
  // Moteur DROIT: avance = IN1 PWM, IN2 = 0
  motorWrite(RIGHT_IN1, RIGHT_IN2, speed, 0);
  // Moteur GAUCHE (inversé): avance = IN1 = 0, IN2 PWM
  motorWrite(LEFT_IN1, LEFT_IN2, 0, speed);
}

void stopAll() {
  motorWrite(RIGHT_IN1, RIGHT_IN2, 0, 0);
  motorWrite(LEFT_IN1,  LEFT_IN2,  0, 0);
}

// =================== ULTRASONS ===================
// Retourne la distance en cm. -1 si pas d'écho (rien dans la portée).
long readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(3);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);
  if (duration == 0) return -1;
  // Vitesse du son ≈ 343 m/s ⇒ 0.0343 cm/µs, aller-retour donc /2
  return (long)(duration * 0.0343f / 2.0f);
}

// Convertit la distance en consigne de vitesse 0..255
int distanceToSpeed(long dist_cm) {
  if (dist_cm < 0)              return SPEED_MAX;   // rien détecté → plein
  if (dist_cm < DIST_STOP_CM)   return 0;           // trop près → stop
  if (dist_cm >= DIST_FULL_CM)  return SPEED_MAX;   // loin → plein

  // Interpolation linéaire entre DIST_STOP (-> SPEED_MIN) et DIST_FULL (-> SPEED_MAX)
  long s = map(dist_cm, DIST_STOP_CM, DIST_FULL_CM, SPEED_MIN, SPEED_MAX);
  if (s < SPEED_MIN) s = SPEED_MIN;
  if (s > SPEED_MAX) s = SPEED_MAX;
  return (int)s;
}

// =================== SETUP ===================
void setup() {
  // Pins capteur
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  // LED interne
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  // PWM sur les 4 pins moteur (ESP32 Arduino core v3+)
  ledcAttach(RIGHT_IN1, PWM_FREQ, PWM_RES);
  ledcAttach(RIGHT_IN2, PWM_FREQ, PWM_RES);
  ledcAttach(LEFT_IN1,  PWM_FREQ, PWM_RES);
  ledcAttach(LEFT_IN2,  PWM_FREQ, PWM_RES);

  // Sécurité: tout à 0
  stopAll();

  // Délai sécurité au démarrage (LED fixe)
  delay(STARTUP_DELAY_MS);
  digitalWrite(LED_PIN, LOW);
}

// =================== LOOP ===================
void loop() {
  long dist = readDistanceCm();
  int speed = distanceToSpeed(dist);

  if (speed == 0) {
    stopAll();
    // LED clignote rapidement = obstacle proche
    digitalWrite(LED_PIN, (millis() / 100) % 2);
  } else {
    forwardSpeed(speed);
    // LED proportionnelle: allumée fixe = pleine vitesse
    digitalWrite(LED_PIN, speed >= SPEED_MAX ? HIGH : ((millis() / (300 - speed)) % 2));
  }

  delay(LOOP_PERIOD_MS);
}
