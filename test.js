#!/usr/bin/env node

// Tests complets pour PAMI Robot

const Robot = require('./robot');
const PAMI_Robot = require('./esp32-robot');
const AirPump = require('./air-pump');
const { DualServoMotor } = require('./servo-motor');
const { LimitSwitches } = require('./limit-switches');
const { LEDStrips } = require('./led-control');
const { PowerManagement } = require('./power-management');

// ============= COLORS FOR TERMINAL =============
const colors = {
  reset: '\x1b[0m',
  green: '\x1b[32m',
  red: '\x1b[31m',
  yellow: '\x1b[33m',
  blue: '\x1b[36m',
  bold: '\x1b[1m',
};

const log = {
  success: (msg) => console.log(`${colors.green}✅ ${msg}${colors.reset}`),
  error: (msg) => console.log(`${colors.red}❌ ${msg}${colors.reset}`),
  test: (msg) => console.log(`${colors.blue}🧪 ${msg}${colors.reset}`),
  section: (msg) => console.log(`\n${colors.bold}${colors.yellow}=== ${msg} ===${colors.reset}\n`),
};

// ============= TEST SUITE =============

let passed = 0;
let failed = 0;

function assert(condition, message) {
  if (condition) {
    log.success(message);
    passed++;
  } else {
    log.error(message);
    failed++;
  }
}

// ============= TESTS =============

log.section('TEST 1: Robot de base');

const robot = new Robot();
assert(robot.position.x === 0 && robot.position.y === 0, 'Position initiale = (0, 0)');
assert(robot.direction === 0, 'Direction initiale = Nord (0°)');
assert(robot.armPosition === 0, 'Bras initial = Bas');

robot.forward(0.3);
assert(robot.position.y > 0, 'Avancer augmente Y');

robot.turnRight(90);
assert(robot.direction === 90, 'Tourne à 90° (Est)');

robot.forward(0.3);
assert(robot.position.x > 0, 'Avancer à l\'Est augmente X');

robot.armUp();
assert(robot.armPosition === 1, 'Bras levé');

log.section('TEST 2: Moteurs SERVO42D');

const motors = new DualServoMotor();
motors.enableAll();
assert(motors.motor1.is_enabled === true, 'Motor 1 activé');
assert(motors.motor2.is_enabled === true, 'Motor 2 activé');

motors.motor1.setSpeed(1000);
assert(motors.motor1.speed === 1000, 'Vitesse Motor 1 = 1000 steps/s');

motors.moveSteps(100, 100);
assert(motors.motor1.position === 100, 'Motor 1 a bougé de 100 steps');

motors.turnInPlace(50, 1);
assert(motors.motor1.position === 50, 'Rotation sur place OK');

log.section('TEST 3: Pompe à air');

const pump = new AirPump();
assert(pump.isActive === false, 'Pompe initialement inactive');

pump.activate();
assert(pump.isActive === true, 'Pompe activée');

pump.deactivate();
assert(pump.isActive === false, 'Pompe désactivée');

const status = pump.getStatus();
assert(status.isActive === false, 'Status reflect l\'état');
assert(status.totalRunTime > 0, 'Temps d\'exécution enregistré');

log.section('TEST 4: Limit Switches');

const switches = new LimitSwitches();
const states = switches.readAll();
assert(states.length === 3, '3 limit switches lus');

const sw1 = switches.get(1);
assert(sw1 !== undefined, 'Switch 1 accessible');

sw1.reset();
assert(sw1.totalTriggers === 0, 'Triggers réinitialisés');

log.section('TEST 5: LED Strips');

const leds = new LEDStrips();
leds.get(1).on();
assert(leds.get(1).isOn === true, 'LED 1 allumée');

leds.get(1).setBrightness(128);
assert(leds.get(1).brightness === 128, 'Luminosité = 128/255');

leds.get(2).setBrightness(255);
assert(leds.get(2).brightness === 255, 'LED 2 à pleine puissance');

leds.allOff();
assert(leds.get(1).brightness === 0, 'Tous les LEDs éteints');

log.section('TEST 6: Gestion Alimentation');

const power = new PowerManagement();
power.battery1.readVoltage();
assert(power.battery1.voltage > 0, 'Tension batterie 1 lue');

power.battery2.readVoltage();
assert(power.battery2.voltage > 0, 'Tension batterie 2 lue');

const summary = power.getSummary();
assert(summary.systemVoltage > 0, 'Tension système calculée');

const health = power.checkSystemHealth();
assert(typeof health === 'boolean', 'Health check retourne booléen');

log.section('TEST 7: Système intégré ESP32');

const pami = new PAMI_Robot();
assert(pami !== undefined, 'Robot PAMI créé');
assert(pami.motors_ !== undefined, 'Moteurs présents');
assert(pami.airPump_ !== undefined, 'Pompe présente');
assert(pami.limitSwitches_ !== undefined, 'Limit switches présents');
assert(pami.leds_ !== undefined, 'LEDs présentes');
assert(pami.power_ !== undefined, 'Gestion puissance présente');

pami.startup();
assert(pami.isRunning === true, 'Robot démarré');

pami.moveForward(0.5);
assert(pami.movement.position.y > 0, 'Robot a bougé');

pami.activatePump();
assert(pami.airPump_.isActive === true, 'Pompe activée via robot');

pami.shutdown();
assert(pami.isRunning === false, 'Robot arrêté');

log.section('TEST 8: Chaînage de commandes');

const robot2 = new Robot();
robot2
  .forward(0.3)
  .turnRight(90)
  .forward(0.3)
  .turnRight(90)
  .forward(0.3);

assert(robot2.history.length === 5, 'Chaînage fonctionne (5 commandes)');
assert(robot2.position.x < 0.01, 'Carré fermé (X ≈ 0)');
assert(robot2.position.y < 0.01, 'Carré fermé (Y ≈ 0)');
assert(robot2.direction === 180, 'Direction finale = Sud');

log.section('TEST 9: Parsing de commandes texte');

const robot3 = new Robot();
robot3
  .executeCommand('avance 0.3')
  .executeCommand('droite 90')
  .executeCommand('avance 0.3');

assert(robot3.position.x > 0, 'Commande texte "avance" fonctionne');
assert(robot3.direction === 90, 'Commande texte "droite" fonctionne');

log.section('TEST 10: Performance');

const startTime = Date.now();

const perf_robot = new Robot();
for (let i = 0; i < 1000; i++) {
  perf_robot
    .forward(0.1)
    .turnRight(1);
}

const endTime = Date.now();
const duration = endTime - startTime;

assert(duration < 100, `1000 commandes en ${duration}ms (< 100ms)`);
log.success(`Performance: ${(1000 / duration).toFixed(0)} ops/ms`);

// ============= RÉSUMÉ =============

log.section('RÉSUMÉ DES TESTS');

const total = passed + failed;
const percent = ((passed / total) * 100).toFixed(1);

console.log(`${colors.bold}Résultats:${colors.reset}`);
console.log(`  ${colors.green}✅ Réussi: ${passed}${colors.reset}`);
console.log(`  ${colors.red}❌ Échoué: ${failed}${colors.reset}`);
console.log(`  ${colors.blue}📊 Total: ${total}${colors.reset}`);
console.log(`  ${colors.bold}${percent}% de réussite${colors.reset}`);

if (failed === 0) {
  console.log(`\n${colors.bold}${colors.green}🎉 TOUS LES TESTS PASSENT! 🎉${colors.reset}\n`);
  process.exit(0);
} else {
  console.log(`\n${colors.bold}${colors.red}⚠️  ${failed} test(s) échoué(s)${colors.reset}\n`);
  process.exit(1);
}
