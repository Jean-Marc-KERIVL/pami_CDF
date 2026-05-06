// Contrôle de la pompe à air

const { PINS } = require('./esp32-pins');

class AirPump {
  constructor(pinMode = null) {
    this.pin = PINS.AIR_PUMP;
    this.pinMode = pinMode; // Mock pour tests, remplacer par vrai GPIO
    this.isActive = false;
    this.startTime = null;
    this.totalRunTime = 0; // en millisecondes
  }

  // Activer la pompe
  activate() {
    if (this.isActive) {
      console.log('⚠️ Pompe à air déjà en marche');
      return this;
    }

    this.isActive = true;
    this.startTime = Date.now();

    // Code réel pour ESP32:
    // digitalWrite(this.pin, HIGH);

    console.log(`💨 Pompe à air ACTIVÉE (pin ${this.pin})`);
    return this;
  }

  // Désactiver la pompe
  deactivate() {
    if (!this.isActive) {
      console.log('⚠️ Pompe à air déjà arrêtée');
      return this;
    }

    const runTime = Date.now() - this.startTime;
    this.totalRunTime += runTime;
    this.isActive = false;

    // Code réel pour ESP32:
    // digitalWrite(this.pin, LOW);

    console.log(`💨 Pompe à air DÉSACTIVÉE (durée: ${(runTime / 1000).toFixed(2)}s)`);
    return this;
  }

  // Basculer la pompe
  toggle() {
    if (this.isActive) {
      return this.deactivate();
    } else {
      return this.activate();
    }
  }

  // Activer pour une durée définie
  activateFor(durationMs) {
    this.activate();

    // Simulation pour tests
    setTimeout(() => {
      this.deactivate();
    }, durationMs);

    console.log(`⏱️  Pompe à air activée pour ${durationMs}ms`);
    return this;
  }

  // Pulse: activer/désactiver rapidement
  pulse(duration = 100) {
    this.activate();
    setTimeout(() => {
      this.deactivate();
    }, duration);

    console.log(`⚡ Pulse pompe (${duration}ms)`);
    return this;
  }

  // Mode rafales
  burst(count = 3, interval = 200, duration = 100) {
    let currentBurst = 0;

    const doBurst = () => {
      if (currentBurst < count) {
        this.activate();
        setTimeout(() => {
          this.deactivate();
          currentBurst++;
          setTimeout(doBurst, interval);
        }, duration);
      }
    };

    console.log(`🔄 Rafales pompe: ${count}x (${duration}ms tous les ${interval}ms)`);
    doBurst();
    return this;
  }

  // Obtenir l'état
  getStatus() {
    const status = {
      pin: this.pin,
      isActive: this.isActive,
      totalRunTime: `${(this.totalRunTime / 1000).toFixed(2)}s`,
      currentRunTime: this.isActive ? `${((Date.now() - this.startTime) / 1000).toFixed(2)}s` : 'N/A',
    };
    return status;
  }

  // Afficher l'état
  printStatus() {
    const s = this.getStatus();
    console.log(`\n📊 État pompe à air:`);
    console.log(`   Pin: ${s.pin}`);
    console.log(`   Statut: ${s.isActive ? '🟢 ACTIVE' : '🔴 INACTIVE'}`);
    console.log(`   Temps total d'exécution: ${s.totalRunTime}`);
    console.log(`   Temps d'exécution actuel: ${s.currentRunTime}`);
    return this;
  }
}

module.exports = AirPump;
