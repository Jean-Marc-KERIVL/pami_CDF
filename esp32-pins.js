// Configuration des pins ESP32

const PINS = {
  // SPI / Communication
  RX: 16,  // UART RX pour SERVO42D
  TX: 17,  // UART TX pour SERVO42D

  // I2C (standard: GND, VCC, SDA, SCL)
  SDA: 21,
  SCL: 22,

  // Limit Switches (INPUT ONLY sur ESP32)
  LIMIT_SWITCH_1: 34,  // Limit switch 1 (input only)
  LIMIT_SWITCH_2: 35,  // Limit switch 2 (input only)
  LIMIT_SWITCH_3: 36,  // Limit switch 3 (input only) - VP (aussi utilisé pour ADC)

  // LED Strips (PWM)
  LED_STRIP_1: 12,     // LED strip 1
  LED_STRIP_2: 13,     // LED strip 2
  LED_STRIP_3: 27,     // LED strip 3

  // Pompe à air
  AIR_PUMP: 14,        // Relay pour pompe à air

  // Bouton d'arrêt d'urgence
  EMERGENCY_STOP: 32,  // Bouton d'arrêt d'urgence

  // Reset (EN pin)
  RESET: 'EN',         // Pin EN avec condensateur

  // Alimentation / Mesure (via I2C - INA219)
  // Utilise les pins I2C (SDA/SCL) 21/22
  INA219_BATTERY_1: 0x40,  // Adresse I2C pour batterie 1
  INA219_BATTERY_2: 0x41,  // Adresse I2C pour batterie 2
};

const POWER = {
  VIN_12V: 'VIN_12V',
  VIN_24V: 'VIN_24V (option jumper)',
  VBUS_5V: 'VBUS_5V',
  VBUS_3V3: 'VBUS_3V3',
  VBUS_3V3_2: 'VBUS_3V3_2 (option headers)',
};

const MOTORS = {
  MOTOR_1: {
    step: 'VIA_SERVO42D',  // Via connecteur JST au driver SERVO42D
    dir: 'VIA_SERVO42D',
    enable: 'VIA_SERVO42D',
  },
  MOTOR_2: {
    step: 'VIA_SERVO42D',
    dir: 'VIA_SERVO42D',
    enable: 'VIA_SERVO42D',
  },
};

module.exports = { PINS, POWER, MOTORS };
