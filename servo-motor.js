// Contrôle des moteurs SERVO42D NEMA17

const { PINS } = require('./esp32-pins');

class ServoMotor {
  constructor(motorId = 1, uart = null) {
    this.motorId = motorId;
    this.uart = uart; // Connexion UART sur RX/TX pins 16/17
    this.position = 0; // position en steps
    this.isEnabled = false;
    this.direction = 1; // 1 = forward, -1 = backward
    this.speed = 500; // steps/s
    this.maxSteps = 100000; // limite max
  }

  // Activer le moteur
  enable() {
    if (this.isEnabled) return this;

    this.isEnabled = true;
    // UART command: {motorId}:ENABLE
    this._sendCommand('ENABLE');
    console.log(`✅ Moteur ${this.motorId} ACTIVÉ`);
    return this;
  }

  // Désactiver le moteur
  disable() {
    if (!this.isEnabled) return this;

    this.isEnabled = false;
    this._sendCommand('DISABLE');
    console.log(`❌ Moteur ${this.motorId} DÉSACTIVÉ`);
    return this;
  }

  // Bouger d'un nombre de steps
  moveSteps(steps) {
    if (!this.isEnabled) {
      console.log(`⚠️ Moteur ${this.motorId} doit être activé d'abord`);
      return this;
    }

    const newPos = this.position + steps;
    if (newPos < 0 || newPos > this.maxSteps) {
      console.log(`⚠️ Position hors limites: ${newPos} (max: ${this.maxSteps})`);
      return this;
    }

    this.position = newPos;
    const direction = steps > 0 ? 1 : -1;
    this.direction = direction;

    // UART: {motorId}:MOVE:{steps}:{speed}
    this._sendCommand(`MOVE:${Math.abs(steps)}:${this.speed}`);
    console.log(`⬆️ Moteur ${this.motorId} -> ${Math.abs(steps)} steps (pos: ${this.position})`);
    return this;
  }

  // Aller à une position absolue
  goToPosition(targetPos) {
    if (targetPos < 0 || targetPos > this.maxSteps) {
      console.log(`⚠️ Position invalide: ${targetPos}`);
      return this;
    }

    const steps = targetPos - this.position;
    return this.moveSteps(steps);
  }

  // Réinitialiser à la position 0
  home() {
    const steps = -this.position;
    this.position = 0;
    this._sendCommand(`MOVE:${Math.abs(steps)}:${this.speed}`);
    console.log(`🏠 Moteur ${this.motorId} retour à HOME (pos: 0)`);
    return this;
  }

  // Définir la vitesse (steps/s)
  setSpeed(stepsPerSecond) {
    if (stepsPerSecond < 0) {
      console.log('⚠️ Vitesse doit être positive');
      return this;
    }

    this.speed = stepsPerSecond;
    this._sendCommand(`SPEED:${stepsPerSecond}`);
    console.log(`⚙️ Moteur ${this.motorId} vitesse -> ${stepsPerSecond} steps/s`);
    return this;
  }

  // Définir le couple/force
  setTorque(percent) {
    if (percent < 0 || percent > 100) {
      console.log('⚠️ Couple doit être entre 0 et 100%');
      return this;
    }

    this._sendCommand(`TORQUE:${percent}`);
    console.log(`💪 Moteur ${this.motorId} couple -> ${percent}%`);
    return this;
  }

  // Mode micro-stepping (1, 2, 4, 8, 16)
  setMicrostepping(division) {
    const valid = [1, 2, 4, 8, 16];
    if (!valid.includes(division)) {
      console.log(`⚠️ Micro-stepping invalide: ${division}`);
      return this;
    }

    this._sendCommand(`MICROSTEPPING:${division}`);
    console.log(`🔧 Moteur ${this.motorId} micro-stepping -> 1/${division}`);
    return this;
  }

  // Obtenir l'état
  getStatus() {
    return {
      motorId: this.motorId,
      isEnabled: this.isEnabled,
      position: this.position,
      direction: this.direction === 1 ? 'forward' : 'backward',
      speed: this.speed,
      maxSteps: this.maxSteps,
    };
  }

  // Afficher l'état
  printStatus() {
    const s = this.getStatus();
    console.log(`\n📊 État moteur ${s.motorId}:`);
    console.log(`   Statut: ${s.isEnabled ? '🟢 ACTIVÉ' : '🔴 DÉSACTIVÉ'}`);
    console.log(`   Position: ${s.position} steps`);
    console.log(`   Direction: ${s.direction}`);
    console.log(`   Vitesse: ${s.speed} steps/s`);
    return this;
  }

  // Envoyer une commande UART
  _sendCommand(cmd) {
    const message = `M${this.motorId}:${cmd}\n`;
    if (this.uart) {
      // Code réel ESP32: Serial1.print(message);
      // Pour tests:
      console.log(`📡 UART TX: ${message.trim()}`);
    } else {
      console.log(`📡 [SIM] UART TX: ${message.trim()}`);
    }
  }
}

// Classe pour gérer deux moteurs ensemble
class DualServoMotor {
  constructor(uart = null) {
    this.motor1 = new ServoMotor(1, uart);
    this.motor2 = new ServoMotor(2, uart);
  }

  // Activer les deux
  enableAll() {
    this.motor1.enable();
    this.motor2.enable();
    return this;
  }

  // Désactiver les deux
  disableAll() {
    this.motor1.disable();
    this.motor2.disable();
    return this;
  }

  // Bouger les deux en synchrone
  moveBothSteps(steps1, steps2) {
    this.motor1.moveSteps(steps1);
    this.motor2.moveSteps(steps2);
    return this;
  }

  // Mouvement différentiel (avant/arrière)
  differential(leftSteps, rightSteps) {
    this.motor1.moveSteps(leftSteps);
    this.motor2.moveSteps(rightSteps);
    console.log(`🔄 Mouvement différentiel: L:${leftSteps}, R:${rightSteps}`);
    return this;
  }

  // Tourner sur place
  turnInPlace(steps, direction = 1) {
    const leftSteps = direction > 0 ? steps : -steps;
    const rightSteps = direction > 0 ? -steps : steps;
    return this.differential(leftSteps, rightSteps);
  }

  getStatus() {
    return {
      motor1: this.motor1.getStatus(),
      motor2: this.motor2.getStatus(),
    };
  }

  printStatus() {
    this.motor1.printStatus();
    this.motor2.printStatus();
    return this;
  }
}

module.exports = { ServoMotor, DualServoMotor };
