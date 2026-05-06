# 🤖 PAMI Robot - Système de Contrôle ESP32

Système de contrôle complet pour robot PAMI basé sur **ESP32** avec:
- **Moteurs**: SERVO42D NEMA17 Closed Loop Stepper Motor Driver
- **Pompe à air**: Contrôle via relay GPIO
- **Capteurs**: Limit switches, mesure tension batteries (INA219)
- **Indicateurs**: LED strips PWM
- **Communication**: UART (moteurs), I2C (INA219)

## 🏗️ Architecture

```
pami_robot/
├── robot.js              # Classe Robot de base (mouvements simples)
├── esp32-pins.js         # Configuration des pins ESP32
├── servo-motor.js        # Contrôle moteurs SERVO42D (UART)
├── air-pump.js          # Contrôle pompe à air (GPIO relay)
├── limit-switches.js    # Capteurs de fin de course (GPIO input)
├── led-control.js       # Contrôle LED strips (PWM)
├── power-management.js  # Mesure batterie (I2C INA219)
├── esp32-robot.js       # Système intégré PAMI_Robot
└── index.js             # Exemples simples
```

## 📦 Installation

```bash
npm install
```

## 🚀 Utilisation ESP32

### Mode test
```bash
npm start
```

### Système complet ESP32
```javascript
const PAMI_Robot = require('./esp32-robot');

const robot = new PAMI_Robot();

// Démarrer
robot.startup();

// Mouvements
robot
  .moveForward(0.5)   // Avance 0.5m
  .turnRight(90)      // Tourne 90°
  .moveForward(0.3);

// Pompe à air
robot
  .activatePump()     // Activer
  .pumpPulse(100)     // Pulse 100ms
  .deactivatePump();  // Désactiver

// Tâches composées
robot
  .grabObject()       // Saisir objet
  .pickAndDrop()      // Sequence complète
  .homeRobot();       // Retour origine

// Diagnostics
robot.diagnostics();

// Arrêt
robot.shutdown();
```

### Contrôle de base
```javascript
const Robot = require('./robot');

const robot = new Robot();

// Mouvements
robot
  .forward(0.3)      // Avance 0.3m
  .turnRight(90)     // Tourne 90° à droite
  .moveLeft(0.3)     // Va à gauche 0.3m
  .armUp()           // Lève le bras
  .grabBlock()       // Saisit un bloc
  .turnAround()      // Demi-tour (180°)
  .forward(0.3)
  .placeBlock();     // Place le bloc
```

## 🎮 Fonctions disponibles

### Mouvements avant/arrière
- `forward(distance)` - Avance (défaut: 0.3m)
- `backward(distance)` - Recule (défaut: 0.3m)

### Rotations
- `turnRight(angle)` - Tourne à droite (défaut: 90°)
- `turnLeft(angle)` - Tourne à gauche (défaut: 90°)
- `turnAround()` - Demi-tour (180°)

### Mouvements latéraux
- `moveRight(distance)` - Va à droite (défaut: 0.3m)
- `moveLeft(distance)` - Va à gauche (défaut: 0.3m)

### Contrôle du bras
- `armUp()` - Lève le bras
- `armDown()` - Abaisse le bras
- `armToggle()` - Bascule le bras haut/bas

### Commandes combinées
- `grabBlock()` - Saisit un bloc
- `placeBlock()` - Place un bloc

### Utilitaires
- `getStatus()` - Récupère position, direction, état du bras et historique
- `reset()` - Réinitialise le robot
- `executeCommand(text)` - Exécute une commande textuelle

## 📝 Format des commandes texte

```javascript
robot.executeCommand('avance 0.3');   // Avance 0.3m
robot.executeCommand('droite 90');    // Tourne 90° à droite
robot.executeCommand('brasup');       // Bras vers le haut
robot.executeCommand('saisir');       // Saisir bloc
robot.executeCommand('demitour');     // Demi-tour
```

## 📊 Exemple complet

```javascript
const Robot = require('./robot');
const robot = new Robot();

// Créer un carré
robot
  .forward(0.3)
  .turnRight(90)
  .forward(0.3)
  .turnRight(90)
  .forward(0.3)
  .turnRight(90)
  .forward(0.3);

// Afficher l'état final
console.log(robot.getStatus());
```

## 🔄 Chaînage de commandes

Toutes les fonctions retournent `this`, ce qui permet le chaînage :

```javascript
robot
  .forward(0.3)
  .turnRight(90)
  .armUp()
  .grabBlock()
  .turnAround()
  .forward(0.5)
  .armDown();
```

## 📍 Système de coordonnées

- **Position X/Y** : Coordonnées cartésiennes
- **Direction** : 0°=Nord, 90°=Est, 180°=Sud, 270°=Ouest
- **Bras** : 0=bas, 1=haut

## 🔌 Modules ESP32

### Moteurs SERVO42D (servo-motor.js)
```javascript
const { DualServoMotor } = require('./servo-motor');
const motors = new DualServoMotor();

motors.enableAll();
motors.motor1.moveSteps(100);      // 100 steps
motors.motor2.setSpeed(1000);      // 1000 steps/s
motors.turnInPlace(50, 1);         // Tourner sur place
```

### Pompe à air (air-pump.js)
```javascript
const AirPump = require('./air-pump');
const pump = new AirPump();

pump.activate();                    // Activer
pump.deactivate();                  // Désactiver
pump.pulse(100);                    // Pulse 100ms
pump.burst(3, 200, 100);           // 3 rafales de 100ms chacune
pump.activateFor(2000);            // Activer 2s
```

### Capteurs de limite (limit-switches.js)
```javascript
const { LimitSwitches } = require('./limit-switches');
const switches = new LimitSwitches();

switches.readAll();                 // Lire tous les capteurs
switches.get(1).isPressed();        // Vérifier un capteur
await switches.waitForAny();        // Attendre un déclenchement
```

### LEDs (led-control.js)
```javascript
const { LEDStrips } = require('./led-control');
const leds = new LEDStrips();

leds.get(1).on();                   // Allumer LED 1
leds.setBrightnessAll(128);         // Luminosité globale
leds.blinkAll(5, 200);              // Clignoter
leds.breatheAll(2000);              // Respiration
leds.rainbow();                     // Effet arc-en-ciel
```

### Gestion alimentation (power-management.js)
```javascript
const { PowerManagement } = require('./power-management');
const power = new PowerManagement();

power.readAllVoltages();            // Lire tensions
power.checkSystemHealth();          // Vérifier santé
power.battery1.checkLowVoltage();  // Alerte basse tension
power.estimateRuntime(100);        // Autonomie (Wh/h)
```

## 📍 Configuration des pins ESP32

Voir `esp32-pins.js` pour la configuration complète:
- **RX/TX**: 16/17 (UART moteurs)
- **I2C**: 21/22 (SDA/SCL - INA219 batteries)
- **Limit switches**: 34, 35, 36 (INPUT ONLY)
- **LEDs**: 12, 13, 27 (PWM)
- **Pompe**: 14 (GPIO)
- **Urgence**: 32 (Bouton)

## 🔗 Intégration matériel

Pour intégrer avec l'ESP32 réel:
1. Remplacer les simulations par du code Arduino/C++
2. Utiliser les vraies fonctions GPIO/UART/I2C
3. Garder la même interface JavaScript

Exemple adaptateur:
```javascript
class ESP32Motor extends ServoMotor {
  _sendCommand(cmd) {
    // Code réel: Serial1.print(`M${this.motorId}:${cmd}\n`);
    super._sendCommand(cmd);
  }
}
```

## 📄 License

MIT
