// Contrôle des LED strips via PWM

const { PINS } = require('./esp32-pins');

class LEDStrip {
  constructor(stripId = 1, pin = null) {
    this.stripId = stripId;
    this.pin = pin || [PINS.LED_STRIP_1, PINS.LED_STRIP_2, PINS.LED_STRIP_3][stripId - 1];
    this.brightness = 0; // 0-255
    this.isOn = false;
    this.pwmFrequency = 5000; // Hz (standard ESP32)
    this.pwmResolution = 8; // bits (0-255)
  }

  // Allumer à pleine puissance
  on() {
    this.setBrightness(255);
    this.isOn = true;
    console.log(`💡 LED strip ${this.stripId} ALLUMÉE`);
    return this;
  }

  // Éteindre
  off() {
    this.setBrightness(0);
    this.isOn = false;
    console.log(`💡 LED strip ${this.stripId} ÉTEINTE`);
    return this;
  }

  // Basculer on/off
  toggle() {
    return this.isOn ? this.off() : this.on();
  }

  // Définir la luminosité (0-255)
  setBrightness(value) {
    if (value < 0 || value > 255) {
      console.log(`⚠️ Luminosité doit être entre 0 et 255`);
      return this;
    }

    this.brightness = value;
    this.isOn = value > 0;

    // Code réel ESP32:
    // analogWrite(this.pin, value);
    // ou ledcWrite(channel, value); pour PWM

    const percent = Math.round((value / 255) * 100);
    console.log(`⚡ LED strip ${this.stripId} -> ${value}/255 (${percent}%)`);
    return this;
  }

  // Augmenter la luminosité
  brighten(step = 10) {
    const newValue = Math.min(this.brightness + step, 255);
    return this.setBrightness(newValue);
  }

  // Diminuer la luminosité
  dim(step = 10) {
    const newValue = Math.max(this.brightness - step, 0);
    return this.setBrightness(newValue);
  }

  // Pulse: clignoter
  pulse(durationMs = 500, intensity = 255) {
    const halfDuration = durationMs / 2;

    this.setBrightness(intensity);
    setTimeout(() => {
      this.setBrightness(0);
      setTimeout(() => {
        this.pulse(durationMs, intensity);
      }, halfDuration);
    }, halfDuration);

    console.log(`⚡ LED strip ${this.stripId} pulse (${durationMs}ms)`);
    return this;
  }

  // Respiratoire: augmente et diminue lentement
  breathe(cycleDurationMs = 2000) {
    const steps = 50;
    const stepDuration = cycleDurationMs / (2 * steps);
    let currentStep = 0;
    let increasing = true;

    const doBreathe = () => {
      if (increasing) {
        this.brightness = (currentStep / steps) * 255;
        currentStep++;
        if (currentStep >= steps) {
          increasing = false;
          currentStep = steps - 1;
        }
      } else {
        this.brightness = (currentStep / steps) * 255;
        currentStep--;
        if (currentStep < 0) {
          increasing = true;
          currentStep = 0;
        }
      }

      // Appliquer sans log à chaque étape
      this._applyBrightness();

      setTimeout(doBreathe, stepDuration);
    };

    console.log(`⚡ LED strip ${this.stripId} respiration (${cycleDurationMs}ms/cycle)`);
    doBreathe();
    return this;
  }

  // Clignotement rapide
  blink(count = 5, durationMs = 200) {
    let remaining = count * 2;

    const doBlink = () => {
      this.toggle();
      remaining--;
      if (remaining > 0) {
        setTimeout(doBlink, durationMs / 2);
      }
    };

    console.log(`⚡ LED strip ${this.stripId} clignotement (${count} x)`);
    doBlink();
    return this;
  }

  // Appliquer la luminosité (sans log)
  _applyBrightness() {
    // Code réel: analogWrite(this.pin, Math.round(this.brightness));
    this.isOn = this.brightness > 0;
  }

  // Obtenir l'état
  getStatus() {
    return {
      stripId: this.stripId,
      pin: this.pin,
      brightness: this.brightness,
      percent: Math.round((this.brightness / 255) * 100),
      isOn: this.isOn,
      pwmFrequency: this.pwmFrequency,
    };
  }

  printStatus() {
    const s = this.getStatus();
    console.log(`\n📊 État LED strip ${s.stripId}:`);
    console.log(`   Pin: ${s.pin}`);
    console.log(`   Luminosité: ${s.brightness}/255 (${s.percent}%)`);
    console.log(`   Statut: ${s.isOn ? '🟢 ALLUMÉE' : '🔴 ÉTEINTE'}`);
    console.log(`   Fréquence PWM: ${s.pwmFrequency}Hz`);
    return this;
  }
}

// Classe pour gérer 3 LED strips
class LEDStrips {
  constructor() {
    this.strips = [
      new LEDStrip(1, PINS.LED_STRIP_1),
      new LEDStrip(2, PINS.LED_STRIP_2),
      new LEDStrip(3, PINS.LED_STRIP_3),
    ];
  }

  // Obtenir un strip spécifique
  get(stripId) {
    return this.strips[stripId - 1];
  }

  // Allumer tous les LEDs
  allOn() {
    this.strips.forEach(s => s.on());
    return this;
  }

  // Éteindre tous les LEDs
  allOff() {
    this.strips.forEach(s => s.off());
    return this;
  }

  // Définir la luminosité pour tous
  setBrightnessAll(value) {
    this.strips.forEach(s => s.setBrightness(value));
    return this;
  }

  // Mode arc-en-ciel: chaque LED à une intensité différente
  rainbow() {
    const values = [255, 128, 64];
    this.strips.forEach((s, i) => s.setBrightness(values[i]));
    console.log('🌈 Mode arc-en-ciel activé');
    return this;
  }

  // Mode clignotement synchronisé
  blinkAll(count = 5, durationMs = 200) {
    let remaining = count * 2;

    const doBlink = () => {
      this.strips.forEach(s => s.toggle());
      remaining--;
      if (remaining > 0) {
        setTimeout(doBlink, durationMs / 2);
      }
    };

    console.log(`⚡ Clignotement tous LEDs (${count} x)`);
    doBlink();
    return this;
  }

  // Mode respiration synchronisée
  breatheAll(cycleDurationMs = 2000) {
    this.strips.forEach(s => s.breathe(cycleDurationMs));
    return this;
  }

  // Obtenir l'état de tous
  getAllStatus() {
    return {
      strips: this.strips.map(s => s.getStatus()),
    };
  }

  printAllStatus() {
    console.log('\n📊 État de tous les LED strips:');
    this.strips.forEach(s => {
      console.log(`   Strip ${s.stripId}: ${s.brightness}/255 (${s.isOn ? '🟢 ON' : '🔴 OFF'})`);
    });
    return this;
  }
}

module.exports = { LEDStrip, LEDStrips };
