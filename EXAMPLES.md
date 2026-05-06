# 📚 Exemples d'utilisation PAMI Robot

## Démarrage rapide

### 1. Robot simple (sans ESP32)
```javascript
const Robot = require('./robot');
const robot = new Robot();

// Parcourir un carré
robot
  .forward(0.3)
  .turnRight(90)
  .forward(0.3)
  .turnRight(90)
  .forward(0.3)
  .turnRight(90)
  .forward(0.3);

console.log(robot.getStatus());
```

### 2. Système complet ESP32
```javascript
const PAMI_Robot = require('./esp32-robot');
const robot = new PAMI_Robot();

robot.startup();
robot.diagnostics();
robot.shutdown();
```

---

## Exemples moteurs

### Contrôle basique des moteurs
```javascript
const { DualServoMotor } = require('./servo-motor');
const motors = new DualServoMotor();

motors.enableAll();
motors.motor1.moveSteps(100);      // Motor 1: 100 steps
motors.motor2.moveSteps(100);      // Motor 2: 100 steps

motors.motor1.setSpeed(1000);      // 1000 steps/seconde
motors.motor2.setTorque(75);       // 75% couple

motors.motor1.goToPosition(0);     // Position absolue
motors.motor1.home();              // Retour à HOME

motors.disableAll();
```

### Mouvement différentiel (avancer/reculer)
```javascript
// Avancer: deux moteurs en avant
motors.differential(100, 100);

// Reculer: deux moteurs en arrière
motors.differential(-100, -100);

// Tourner sur place (sens horaire)
motors.turnInPlace(50, 1);

// Tourner sur place (sens anti-horaire)
motors.turnInPlace(50, -1);
```

### Courbes et mouvements complexes
```javascript
// Courbe à droite: moteur gauche plus rapide
motors.differential(150, 100);

// Arc de cercle
motors.motor1.setSpeed(800);
motors.motor2.setSpeed(1200);
motors.differential(200, 200);
```

---

## Exemples pompe à air

### Activation simple
```javascript
const AirPump = require('./air-pump');
const pump = new AirPump();

// Activer en continu
pump.activate();
// ... faire quelque chose ...
pump.deactivate();
```

### Pulse (impulsion unique)
```javascript
// Impulsion de 150ms
pump.pulse(150);
```

### Rafales (burst)
```javascript
// 5 impulsions de 100ms espacées de 300ms
pump.burst(5, 300, 100);
```

### Activation temporisée
```javascript
// Activer pour exactement 2 secondes
pump.activateFor(2000);

// Automatiquement désactivé après 2s
```

### Patterns complexes
```javascript
// Pattern 1: pulse rapide × 3
for (let i = 0; i < 3; i++) {
  pump.pulse(100);
  setTimeout(() => {}, 200); // Attendre
}

// Pattern 2: alternance on/off
setInterval(() => {
  pump.toggle();
}, 500);  // Toggle chaque 500ms

// Pattern 3: rampe
let duration = 100;
setInterval(() => {
  pump.pulse(duration);
  duration += 50;
  if (duration > 500) duration = 100;
}, 1000);
```

---

## Exemples limit switches

### Vérification d'état
```javascript
const { LimitSwitches } = require('./limit-switches');
const switches = new LimitSwitches();

// Lire tous les switches
const states = switches.readAll();
states.forEach(s => {
  console.log(`Switch ${s.switchId}: ${s.isTriggered ? 'DÉCLENCHÉ' : 'OK'}`);
});
```

### Attendre un déclenchement
```javascript
// Attendre que le switch 1 soit déclenché
const sw1 = switches.get(1);
const triggered = await sw1.waitForTrigger(5000);  // Timeout 5s

if (triggered) {
  console.log('Switch 1 déclenché!');
} else {
  console.log('Timeout');
}
```

### Homing avec limit switches
```javascript
// Homing: recul jusqu'au switch
motors.motor1.moveSteps(-1000);

// Attendre que le limit switch s'active
await switches.get(1).waitForTrigger();

// Reset position
motors.motor1.home();
console.log('Homing complété!');
```

---

## Exemples LEDs

### Contrôle simple
```javascript
const { LEDStrips } = require('./led-control');
const leds = new LEDStrips();

// LED 1
leds.get(1).on();
leds.get(1).setBrightness(128);
leds.get(1).off();

// Tous les LEDs
leds.allOn();
leds.setBrightnessAll(255);
leds.allOff();
```

### Effets visuels
```javascript
// Respiration (effet doux)
leds.get(1).breathe(2000);  // Cycle 2s

// Clignotement
leds.get(2).blink(5, 200);  // 5x, 200ms chacun

// Pulse rapide
leds.get(3).pulse(500);     // 500ms cycle

// Arc-en-ciel synchronisé
leds.rainbow();
```

### Synchronisation globale
```javascript
// Clignoter tous les LEDs ensemble
leds.blinkAll(10, 100);

// Respiration synchronisée
leds.breatheAll(3000);

// Alternance de couleurs
leds.get(1).setBrightness(255);
leds.get(2).setBrightness(128);
leds.get(3).setBrightness(64);
```

---

## Exemples gestion alimentation

### Monitoring batterie
```javascript
const { PowerManagement } = require('./power-management');
const power = new PowerManagement();

// Lire les tensions
const voltages = power.readAllVoltages();
console.log(`Batterie totale: ${voltages.total.toFixed(2)}V`);

// Vérifier la santé du système
if (power.checkSystemHealth()) {
  console.log('✅ Système OK');
} else {
  console.log('⚠️ Problème d\'alimentation');
}
```

### Autonomie estimée
```javascript
// Estimer l'autonomie avec une consommation de 150Wh/h
const runtime = power.estimateRuntime(150);
console.log(`Durée: ~${runtime.estimatedRuntimeHours} heures`);

power.printRuntime(150);  // Affichage complet
```

### Monitoring batterie individuelles
```javascript
power.battery1.readVoltage();
power.battery1.readCurrent();
power.battery1.readPower();

const health = power.battery1.getHealth();
console.log(`Batterie 1: ${health.status} (${health.percent}%)`);

// Alerte basse tension
power.battery2.checkLowVoltage();
```

### Arrêt d'urgence
```javascript
// Activé par le bouton d'arrêt d'urgence
power.emergencyStop();

// Plus tard: reprendre
power.resumeOperation();
```

---

## Tâches composées (robot complet)

### Séquence pick and drop
```javascript
const PAMI_Robot = require('./esp32-robot');
const robot = new PAMI_Robot();

robot.startup();

// 1. Avancer vers l'objet
robot.moveForward(0.5);

// 2. Attendre 1s
setTimeout(() => {
  // 3. Saisir avec pompe
  robot.grabObject();
  
  // 4. Revenir au point de départ
  setTimeout(() => {
    robot.moveBackward(0.5);
    robot.shutdown();
  }, 3000);
}, 1000);
```

### Homing complet
```javascript
// Retour à l'origine avec capteurs
robot.homeRobot();

// Puis parler d'autres tâches...
setTimeout(() => {
  robot.moveForward(0.3);
}, 2000);
```

### Cycle complet (avancer → saisir → placer)
```javascript
const sequence = async () => {
  robot.startup();

  // Phase 1: Aller à la première zone
  console.log('Phase 1: Déplacement');
  robot.moveToZone('forward', 0.5, 90);

  // Phase 2: Saisir objet
  await new Promise(r => setTimeout(r, 2000));
  console.log('Phase 2: Saisir');
  robot.grabObject();

  // Phase 3: Aller à la zone de dépôt
  await new Promise(r => setTimeout(r, 3000));
  console.log('Phase 3: Aller au dépôt');
  robot.moveToZone('forward', 0.3, -90);

  // Phase 4: Placer
  await new Promise(r => setTimeout(r, 2000));
  robot.power.printSummary();

  robot.shutdown();
};

sequence();
```

### Diagnostic complet
```javascript
robot.startup();

// 1s après: diagnostics
setTimeout(() => {
  robot.diagnostics();
  
  // Ensuite: shutdown
  setTimeout(() => {
    robot.shutdown();
  }, 2000);
}, 1000);
```

---

## Patterns avancés

### State machine (robot autonome)
```javascript
class RobotController {
  constructor(robot) {
    this.robot = robot;
    this.state = 'IDLE';
  }

  async run() {
    this.robot.startup();

    while (true) {
      switch(this.state) {
        case 'IDLE':
          console.log('En attente...');
          this.state = 'MOVE';
          break;
        
        case 'MOVE':
          this.robot.moveForward(0.5);
          this.state = 'GRAB';
          await this.delay(2000);
          break;
        
        case 'GRAB':
          this.robot.grabObject();
          this.state = 'RETURN';
          await this.delay(3000);
          break;
        
        case 'RETURN':
          this.robot.moveBackward(0.5);
          this.state = 'IDLE';
          await this.delay(2000);
          break;
      }
    }
  }

  delay(ms) {
    return new Promise(r => setTimeout(r, ms));
  }
}

const controller = new RobotController(robot);
controller.run();
```

### Monitoring en temps réel
```javascript
setInterval(() => {
  const status = robot.power.getSummary();
  
  if (status.battery1.voltage < 11) {
    console.log('⚠️ Batterie 1 faible!');
  }
  
  if (robot.limitSwitches.isTriggered(1)) {
    console.log('🛑 Limit switch 1 déclenché');
  }
}, 1000);  // Vérifier chaque seconde
```

---

## 🐛 Debugging

### Activer tous les logs
```javascript
const robot = new PAMI_Robot();

// Vérifier chaque état
robot.motors.motor1.printStatus();
robot.airPump.printStatus();
robot.limitSwitches.printAllStatus();
robot.leds.printAllStatus();
robot.power.printSummary();
```

### Tester un composant seul
```javascript
// Tester juste la pompe
const AirPump = require('./air-pump');
const pump = new AirPump();

pump.activate();
pump.printStatus();
pump.pulse(200);
pump.printStatus();
```

### Simulation sans hardware
```javascript
// Tous les modules fonctionnent en simulation:
const robot = new PAMI_Robot();
robot.startup();

// Aucun matériel ESP32 n'est nécessaire pour tester la logique
robot.moveForward(1.0);
robot.airPump.burst(5, 300, 100);
robot.diagnostics();
```

---

**Plus d'exemples à venir!** 📖
