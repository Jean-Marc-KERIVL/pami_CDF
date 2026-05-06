// Gestion de l'alimentation et mesure de tension (INA219)

const { PINS, POWER } = require('./esp32-pins');

class Battery {
  constructor(batteryId = 1, ina219Address = null) {
    this.batteryId = batteryId;
    this.ina219Address = ina219Address || [PINS.INA219_BATTERY_1, PINS.INA219_BATTERY_2][batteryId - 1];
    this.voltage = 0; // V
    this.current = 0; // mA
    this.power = 0; // W
    this.isConnected = false;
    this.calibrationFactor = 0.1; // Pour convertisseur 10x (exemple)
    this.maxVoltage = 12; // ou 24V selon config
    this.minVoltage = 9; // seuil alerte basse tension
    this.history = [];
  }

  // Lire la tension via I2C INA219
  readVoltage() {
    // Code réel ESP32:
    // const raw = ina219.readBusVoltage();
    // this.voltage = raw / 1000; // en V

    // Simulation pour tests:
    this.voltage = this.maxVoltage - (Math.random() * 0.5);
    this.isConnected = true;

    const timestamp = new Date().toISOString();
    this.history.push({
      timestamp,
      voltage: this.voltage,
      current: this.current,
      power: this.power,
    });

    return this.voltage;
  }

  // Lire le courant via I2C INA219
  readCurrent() {
    // Code réel ESP32:
    // this.current = ina219.readCurrent_mA();

    // Simulation:
    this.current = Math.random() * 500; // 0-500mA

    return this.current;
  }

  // Lire la puissance
  readPower() {
    this.readVoltage();
    this.readCurrent();
    this.power = (this.voltage * this.current) / 1000; // en W
    return this.power;
  }

  // Vérifier la santé de la batterie
  getHealth() {
    const percent = (this.voltage / this.maxVoltage) * 100;
    const status = percent > 80 ? '🟢 Excellente' :
                   percent > 60 ? '🟡 Bonne' :
                   percent > 40 ? '🟠 Faible' :
                   '🔴 Critique';

    return {
      percent: percent.toFixed(1),
      status,
      voltage: this.voltage.toFixed(2),
    };
  }

  // Avertissement si tension basse
  checkLowVoltage() {
    if (this.voltage < this.minVoltage) {
      console.log(`⚠️ Batterie ${this.batteryId}: TENSION BASSE (${this.voltage.toFixed(2)}V)`);
      return true;
    }
    return false;
  }

  // Obtenir l'état
  getStatus() {
    return {
      batteryId: this.batteryId,
      ina219Address: `0x${this.ina219Address.toString(16)}`,
      isConnected: this.isConnected,
      voltage: this.voltage.toFixed(2),
      current: this.current.toFixed(1),
      power: this.power.toFixed(2),
      health: this.getHealth(),
      maxVoltage: this.maxVoltage,
      minVoltage: this.minVoltage,
    };
  }

  printStatus() {
    const s = this.getStatus();
    console.log(`\n🔋 État Batterie ${s.batteryId}:`);
    console.log(`   I2C: ${s.ina219Address} (${s.isConnected ? '🟢 Connectée' : '🔴 Déconnectée'})`);
    console.log(`   Tension: ${s.voltage}V (max: ${s.maxVoltage}V, min: ${s.minVoltage}V)`);
    console.log(`   Courant: ${s.current}mA`);
    console.log(`   Puissance: ${s.power}W`);
    console.log(`   Santé: ${s.health.status} (${s.health.percent}%)`);
    return this;
  }
}

// Classe pour gérer les deux batteries
class PowerManagement {
  constructor() {
    this.battery1 = new Battery(1, PINS.INA219_BATTERY_1);
    this.battery2 = new Battery(2, PINS.INA219_BATTERY_2);
    this.voltage12V = 12;
    this.voltage5V = 5;
    this.voltage3V3 = 3.3;
    this.emergencyStopActive = false;
  }

  // Lire toutes les tensions
  readAllVoltages() {
    this.battery1.readVoltage();
    this.battery2.readVoltage();

    return {
      battery1: this.battery1.voltage,
      battery2: this.battery2.voltage,
      total: this.battery1.voltage + this.battery2.voltage,
    };
  }

  // Lire tous les courants
  readAllCurrents() {
    this.battery1.readCurrent();
    this.battery2.readCurrent();

    return {
      battery1: this.battery1.current,
      battery2: this.battery2.current,
      total: this.battery1.current + this.battery2.current,
    };
  }

  // Vérifier la santé générale
  checkSystemHealth() {
    const bat1Low = this.battery1.checkLowVoltage();
    const bat2Low = this.battery2.checkLowVoltage();

    if (bat1Low || bat2Low) {
      console.log('⚠️ ⚠️ ALERTE: Tension batterie basse');
      return false;
    }

    console.log('✅ Système électrique sain');
    return true;
  }

  // Obtenir un résumé du système
  getSummary() {
    return {
      battery1: this.battery1.getStatus(),
      battery2: this.battery2.getStatus(),
      systemVoltage: (this.battery1.voltage + this.battery2.voltage).toFixed(2),
      systemCurrent: (this.battery1.current + this.battery2.current).toFixed(1),
      systemPower: (this.battery1.power + this.battery2.power).toFixed(2),
      emergencyStop: this.emergencyStopActive,
    };
  }

  // Afficher un résumé complet
  printSummary() {
    const s = this.getSummary();
    console.log('\n⚡ RÉSUMÉ SYSTÈME ÉLECTRIQUE:');
    console.log(`   Tension totale: ${s.systemVoltage}V`);
    console.log(`   Courant total: ${s.systemCurrent}mA`);
    console.log(`   Puissance totale: ${s.systemPower}W`);
    console.log(`   Arrêt d'urgence: ${s.emergencyStop ? '🔴 ACTIVÉ' : '🟢 Inactif'}`);
    this.battery1.printStatus();
    this.battery2.printStatus();
    return this;
  }

  // Activer arrêt d'urgence
  emergencyStop() {
    this.emergencyStopActive = true;
    console.log('🛑 🛑 ARRÊT D\'URGENCE ACTIVÉ 🛑 🛑');
    // Couper toutes les alimentations
    return this;
  }

  // Désactiver arrêt d'urgence
  resumeOperation() {
    this.emergencyStopActive = false;
    console.log('✅ Reprise d\'opération après arrêt d\'urgence');
    return this;
  }

  // Obtenir le rapport d'autonomie estimée
  estimateRuntime(consumptionPerHourWh = 100) {
    const total_Wh = (this.battery1.voltage + this.battery2.voltage) * 10; // Estimation simple
    const runtimeHours = total_Wh / consumptionPerHourWh;
    const runtimeMinutes = runtimeHours * 60;

    return {
      estimatedRuntimeHours: runtimeHours.toFixed(1),
      estimatedRuntimeMinutes: runtimeMinutes.toFixed(0),
      consumptionPerHour: consumptionPerHourWh,
    };
  }

  printRuntime(consumptionPerHourWh = 100) {
    const rt = this.estimateRuntime(consumptionPerHourWh);
    console.log(`\n⏱️ AUTONOMIE ESTIMÉE:`);
    console.log(`   Consommation: ${rt.consumptionPerHour}Wh/h`);
    console.log(`   Durée: ~${rt.estimatedRuntimeHours}h (${rt.estimatedRuntimeMinutes}min)`);
    return this;
  }
}

module.exports = { Battery, PowerManagement };
