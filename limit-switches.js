// Gestion des limit switches (capteurs de fin de course)

const { PINS } = require('./esp32-pins');

class LimitSwitch {
  constructor(switchId = 1, pin = null) {
    this.switchId = switchId;
    this.pin = pin || [PINS.LIMIT_SWITCH_1, PINS.LIMIT_SWITCH_2, PINS.LIMIT_SWITCH_3][switchId - 1];
    this.isTriggered = false;
    this.triggerMode = 'LOW'; // LOW ou HIGH (selon pullup/pulldown)
    this.lastTriggeredTime = null;
    this.totalTriggers = 0;
  }

  // Lire l'état du switch
  read() {
    // Code réel ESP32: const state = digitalRead(this.pin);
    // Pour tests: simuler aléatoirement
    return Math.random() > 0.8; // 20% chance d'être déclenché
  }

  // Vérifier si le switch est déclenché
  isPressed() {
    this.isTriggered = this.read();
    if (this.isTriggered) {
      this.lastTriggeredTime = Date.now();
      this.totalTriggers++;
      console.log(`🛑 Limit switch ${this.switchId} DÉCLENCHÉ (pin ${this.pin})`);
    }
    return this.isTriggered;
  }

  // Attendre que le switch soit déclenché (blocking)
  waitForTrigger(timeoutMs = 10000) {
    return new Promise((resolve) => {
      const startTime = Date.now();
      const interval = setInterval(() => {
        if (this.isPressed()) {
          clearInterval(interval);
          resolve(true);
        }
        if (Date.now() - startTime > timeoutMs) {
          clearInterval(interval);
          console.log(`⚠️ Timeout en attente du limit switch ${this.switchId}`);
          resolve(false);
        }
      }, 50);
    });
  }

  // Réinitialiser les stats
  reset() {
    this.isTriggered = false;
    this.lastTriggeredTime = null;
    console.log(`🔄 Limit switch ${this.switchId} réinitialisé`);
    return this;
  }

  // Obtenir l'état
  getStatus() {
    return {
      switchId: this.switchId,
      pin: this.pin,
      isTriggered: this.isTriggered,
      triggerMode: this.triggerMode,
      lastTriggeredTime: this.lastTriggeredTime ? new Date(this.lastTriggeredTime).toISOString() : 'N/A',
      totalTriggers: this.totalTriggers,
    };
  }

  printStatus() {
    const s = this.getStatus();
    console.log(`\n📊 État limit switch ${s.switchId}:`);
    console.log(`   Pin: ${s.pin}`);
    console.log(`   Déclenché: ${s.isTriggered ? '🔴 OUI' : '🟢 NON'}`);
    console.log(`   Mode: ${s.triggerMode}`);
    console.log(`   Dernier déclenchement: ${s.lastTriggeredTime}`);
    console.log(`   Total déclenchements: ${s.totalTriggers}`);
    return this;
  }
}

// Classe pour gérer 3 limit switches
class LimitSwitches {
  constructor() {
    this.switches = [
      new LimitSwitch(1, PINS.LIMIT_SWITCH_1),
      new LimitSwitch(2, PINS.LIMIT_SWITCH_2),
      new LimitSwitch(3, PINS.LIMIT_SWITCH_3),
    ];
  }

  // Vérifier tous les switches
  readAll() {
    const states = this.switches.map(s => ({
      switchId: s.switchId,
      isTriggered: s.isPressed(),
    }));
    return states;
  }

  // Obtenir un switch spécifique
  get(switchId) {
    return this.switches[switchId - 1];
  }

  // Vérifier si un switch est déclenché
  isTriggered(switchId) {
    const sw = this.get(switchId);
    return sw ? sw.isPressed() : false;
  }

  // Attendre que n'importe quel switch soit déclenché
  waitForAny(timeoutMs = 10000) {
    return Promise.race(
      this.switches.map(s => s.waitForTrigger(timeoutMs))
    );
  }

  // Attendre que tous les switches soient déclenchés
  waitForAll(timeoutMs = 10000) {
    return Promise.all(
      this.switches.map(s => s.waitForTrigger(timeoutMs))
    );
  }

  // Réinitialiser tous les switches
  resetAll() {
    this.switches.forEach(s => s.reset());
    console.log('🔄 Tous les limit switches réinitialisés');
    return this;
  }

  // Obtenir l'état de tous
  getAllStatus() {
    return {
      switches: this.switches.map(s => s.getStatus()),
    };
  }

  printAllStatus() {
    console.log('\n📊 État de tous les limit switches:');
    this.switches.forEach(s => {
      const triggered = s.isPressed();
      console.log(`   Switch ${s.switchId}: ${triggered ? '🔴 DÉCLENCHÉ' : '🟢 OK'} (pin ${s.pin})`);
    });
    return this;
  }
}

module.exports = { LimitSwitch, LimitSwitches };
