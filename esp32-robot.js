#!/usr/bin/env node

// Système de contrôle complet pour robot PAMI sur ESP32

const Robot = require('./robot');
const { DualServoMotor } = require('./servo-motor');
const AirPump = require('./air-pump');
const { LimitSwitches } = require('./limit-switches');
const { LEDStrips } = require('./led-control');
const { PowerManagement } = require('./power-management');

class PAMI_Robot {
  constructor() {
    console.log('🤖 Initialisation du robot PAMI ESP32\n');

    // Composants physiques
    this.motors = new DualServoMotor();
    this.airPump = new AirPump();
    this.limitSwitches = new LimitSwitches();
    this.leds = new LEDStrips();
    this.power = new PowerManagement();

    // Logique de base (Robot classe simple)
    this.movement = new Robot();

    // État du robot
    this.isRunning = false;
    this.currentTask = null;
  }

  // === INITIALISATION ===

  startup() {
    console.log('🚀 Démarrage du robot PAMI\n');

    // Vérifier l'alimentation
    this.power.readAllVoltages();
    if (!this.power.checkSystemHealth()) {
      console.log('❌ Impossible de démarrer: problème d\'alimentation');
      return false;
    }

    // Allumer les LEDs d'indication
    this.leds.get(1).setBrightness(100); // LED de puissance

    // Activer les moteurs
    this.motors.enableAll();

    // Initialiser les limit switches
    this.limitSwitches.resetAll();

    this.isRunning = true;
    console.log('✅ Robot PAMI démarré\n');
    return true;
  }

  shutdown() {
    console.log('\n🛑 Arrêt du robot PAMI\n');

    this.motors.disableAll();
    this.airPump.deactivate();
    this.leds.allOff();

    this.isRunning = false;
    console.log('✅ Robot PAMI arrêté\n');
  }

  // === MOUVEMENTS ===

  moveForward(distance = 0.3) {
    if (!this.isRunning) return this;
    console.log(`\n➡️ Mouvement AVANT (${distance}m)`);
    this.motors.moveBothSteps(100, 100);
    this.movement.forward(distance);
    return this;
  }

  moveBackward(distance = 0.3) {
    if (!this.isRunning) return this;
    console.log(`\n⬅️ Mouvement ARRIÈRE (${distance}m)`);
    this.motors.moveBothSteps(-100, -100);
    this.movement.backward(distance);
    return this;
  }

  turnRight(angle = 90) {
    if (!this.isRunning) return this;
    console.log(`\n🔄 Rotation DROITE (${angle}°)`);
    this.motors.turnInPlace(50, 1);
    this.movement.turnRight(angle);
    return this;
  }

  turnLeft(angle = 90) {
    if (!this.isRunning) return this;
    console.log(`\n🔄 Rotation GAUCHE (${angle}°)`);
    this.motors.turnInPlace(50, -1);
    this.movement.turnLeft(angle);
    return this;
  }

  // === POMPE À AIR ===

  activatePump() {
    if (!this.isRunning) return this;
    this.airPump.activate();
    this.leds.get(2).on(); // LED de pompe
    return this;
  }

  deactivatePump() {
    if (!this.isRunning) return this;
    this.airPump.deactivate();
    this.leds.get(2).off();
    return this;
  }

  pumpPulse(duration = 100) {
    if (!this.isRunning) return this;
    this.airPump.pulse(duration);
    return this;
  }

  // === TÂCHES COMPOSÉES ===

  // Récupérer un objet (saisir avec pompe)
  grabObject() {
    console.log('\n🎯 Tâche: SAISIR OBJET');
    this.airPump.burst(3, 200, 100); // 3 rafales rapides
    return this;
  }

  // Déplacer vers une zone (avancer + tourner)
  moveToZone(direction = 'forward', distance = 0.3, angle = 0) {
    console.log(`\n🗺️ Tâche: ALLER VERS ${direction.toUpperCase()}`);

    if (direction === 'forward') {
      this.moveForward(distance);
    } else if (direction === 'backward') {
      this.moveBackward(distance);
    }

    if (angle !== 0) {
      if (angle > 0) {
        this.turnRight(angle);
      } else {
        this.turnLeft(-angle);
      }
    }

    return this;
  }

  // Sequence complète: avancer + saisir + revenir
  pickAndDrop() {
    console.log('\n🚀 Tâche: PICK AND DROP');

    // Phase 1: Avancer vers l'objet
    this.leds.get(3).on();
    this.moveForward(0.5);

    // Phase 2: Saisir
    setTimeout(() => {
      this.grabObject();
    }, 1000);

    // Phase 3: Revenir
    setTimeout(() => {
      this.moveBackward(0.5);
      this.leds.get(3).off();
    }, 3000);

    return this;
  }

  // === HOMING / INITIALISATION ===

  homeRobot() {
    console.log('\n🏠 Homing du robot\n');

    this.leds.get(1).pulse(200); // LED clignotante
    this.motors.motor1.moveSteps(-500); // Aller à gauche
    this.motors.motor2.moveSteps(-500); // Aller à droite

    // Attendre les limit switches
    setTimeout(() => {
      this.limitSwitches.readAll().forEach(s => {
        if (s.isTriggered) {
          console.log(`✅ Limit switch ${s.switchId} atteint`);
        }
      });

      this.motors.motor1.home();
      this.motors.motor2.home();
      console.log('✅ Homing complété\n');
    }, 2000);

    return this;
  }

  // === DIAGNOSTICS ===

  diagnostics() {
    console.log('\n🔍 DIAGNOSTICS COMPLETS DU ROBOT\n');

    console.log('--- Alimentation ---');
    this.power.printSummary();

    console.log('\n--- Moteurs ---');
    this.motors.printStatus();

    console.log('\n--- Limit Switches ---');
    this.limitSwitches.printAllStatus();

    console.log('\n--- LEDs ---');
    this.leds.printAllStatus();

    console.log('\n--- Pompe à air ---');
    this.airPump.printStatus();

    console.log('\n--- Position du robot ---');
    console.log(`   Position: (${this.movement.position.x.toFixed(2)}, ${this.movement.position.y.toFixed(2)})`);
    console.log(`   Direction: ${this.movement.direction}°\n`);

    return this;
  }

  // === ARRÊT D'URGENCE ===

  emergency() {
    console.log('\n\n🛑 🛑 ARRÊT D\'URGENCE! 🛑 🛑\n');

    this.motors.disableAll();
    this.airPump.deactivate();
    this.leds.allOff();
    this.power.emergencyStop();

    this.isRunning = false;
    return this;
  }
}

// === EXEMPLE D'UTILISATION ===

console.log('='.repeat(60));
console.log('       SYSTÈME DE CONTRÔLE ROBOT PAMI ESP32');
console.log('='.repeat(60));

const robot = new PAMI_Robot();

// Démarrage
robot.startup();

// Test 1: Mouvements simples
console.log('\n--- TEST 1: Mouvements simples ---');
robot
  .moveForward(0.3)
  .turnRight(90)
  .moveForward(0.3)
  .turnRight(90)
  .moveForward(0.3);

// Test 2: Pompe à air
console.log('\n--- TEST 2: Pompe à air ---');
setTimeout(() => {
  robot
    .activatePump()
    .pumpPulse(200);

  setTimeout(() => {
    robot.deactivatePump();
  }, 500);
}, 2000);

// Test 3: LEDs
console.log('\n--- TEST 3: Patterns LEDs ---');
setTimeout(() => {
  robot.leds.rainbow();
  setTimeout(() => {
    robot.leds.blinkAll(3, 300);
  }, 1000);
}, 4000);

// Diagnostics
setTimeout(() => {
  robot.diagnostics();
}, 6000);

// Arrêt
setTimeout(() => {
  robot.shutdown();
}, 8000);

module.exports = PAMI_Robot;
