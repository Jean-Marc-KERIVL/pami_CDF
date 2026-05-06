#!/usr/bin/env node

// Exemple d'utilisation du robot PAMI

const Robot = require('./robot');

console.log('🤖 Initialisation du robot PAMI\n');
const robot = new Robot();

// Scenario 1: Mouvements simples
console.log('--- Test 1: Mouvements simples ---');
robot
  .forward(0.3)      // Avance 0.3m
  .turnRight(90)     // Tourne 90° à droite
  .forward(0.3)      // Avance 0.3m
  .turnRight(90)     // Tourne 90°
  .forward(0.3);     // Avance 0.3m

console.log('\n--- Test 2: Contrôle du bras ---');
robot
  .armUp()           // Bras vers le haut
  .forward(0.3)
  .grabBlock()       // Saisir un bloc
  .turnAround()      // Demi-tour
  .forward(0.3)
  .placeBlock();     // Placer le bloc

console.log('\n--- Test 3: Mouvements latéraux ---');
robot.reset();
robot
  .moveLeft(0.3)
  .forward(0.3)
  .moveRight(0.6)
  .turnLeft(90)
  .forward(0.3);

console.log('\n--- Test 4: Commandes texte ---');
robot.reset();
robot
  .executeCommand('avance 0.3')
  .executeCommand('droite 90')
  .executeCommand('avance 0.3')
  .executeCommand('brasup')
  .executeCommand('saisir')
  .executeCommand('demitour')
  .executeCommand('avance 0.3')
  .executeCommand('placer');

console.log('\n📊 État final du robot:');
console.log(robot.getStatus());

// Mode interactif (optionnel)
console.log('\n💡 Pour utiliser le robot en interactif:');
console.log('const robot = new Robot();');
console.log('robot.forward(0.3).turnRight(90).armUp().grabBlock();');
