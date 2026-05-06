# 🤖 PAMI Robot - Système de Contrôle

Système de contrôle pour robot PAMI avec fonctions de mouvement et contrôle du bras.

## 📦 Installation

```bash
npm install
```

## 🚀 Utilisation

### Mode test
```bash
npm start
```

### Intégration dans votre code
```javascript
const Robot = require('./robot');

const robot = new Robot();

// Mouvements
robot
  .forward(0.3)      // Avance 0.3m
  .turnRight(90)     // Tourne 90° à droite
  .moveLeft(0.3)     // Va à gauche 0.3m
  .armUp()           // Lève le bras
  .grabBlock()       // Saisit un bloc
  .turnAround()      // Demi-tour (180°)
  .forward(0.3)
  .placeBlock();     // Place le bloc
```

## 🎮 Fonctions disponibles

### Mouvements avant/arrière
- `forward(distance)` - Avance (défaut: 0.3m)
- `backward(distance)` - Recule (défaut: 0.3m)

### Rotations
- `turnRight(angle)` - Tourne à droite (défaut: 90°)
- `turnLeft(angle)` - Tourne à gauche (défaut: 90°)
- `turnAround()` - Demi-tour (180°)

### Mouvements latéraux
- `moveRight(distance)` - Va à droite (défaut: 0.3m)
- `moveLeft(distance)` - Va à gauche (défaut: 0.3m)

### Contrôle du bras
- `armUp()` - Lève le bras
- `armDown()` - Abaisse le bras
- `armToggle()` - Bascule le bras haut/bas

### Commandes combinées
- `grabBlock()` - Saisit un bloc
- `placeBlock()` - Place un bloc

### Utilitaires
- `getStatus()` - Récupère position, direction, état du bras et historique
- `reset()` - Réinitialise le robot
- `executeCommand(text)` - Exécute une commande textuelle

## 📝 Format des commandes texte

```javascript
robot.executeCommand('avance 0.3');   // Avance 0.3m
robot.executeCommand('droite 90');    // Tourne 90° à droite
robot.executeCommand('brasup');       // Bras vers le haut
robot.executeCommand('saisir');       // Saisir bloc
robot.executeCommand('demitour');     // Demi-tour
```

## 📊 Exemple complet

```javascript
const Robot = require('./robot');
const robot = new Robot();

// Créer un carré
robot
  .forward(0.3)
  .turnRight(90)
  .forward(0.3)
  .turnRight(90)
  .forward(0.3)
  .turnRight(90)
  .forward(0.3);

// Afficher l'état final
console.log(robot.getStatus());
```

## 🔄 Chaînage de commandes

Toutes les fonctions retournent `this`, ce qui permet le chaînage :

```javascript
robot
  .forward(0.3)
  .turnRight(90)
  .armUp()
  .grabBlock()
  .turnAround()
  .forward(0.5)
  .armDown();
```

## 📍 Système de coordonnées

- **Position X/Y** : Coordonnées cartésiennes
- **Direction** : 0°=Nord, 90°=Est, 180°=Sud, 270°=Ouest
- **Bras** : 0=bas, 1=haut

## 🔗 Intégration future

Pour intégrer avec le matériel réel:
1. Créer une sous-classe `RealRobot` qui hérite de `Robot`
2. Surcharger les fonctions avec du code de communication matériel
3. Garder la même interface pour compatibilité

Exemple:
```javascript
class RealRobot extends Robot {
  forward(distance) {
    super.forward(distance);
    // Envoyer commande au robot réel
    sendToHardware('FORWARD', distance);
  }
}
```

## 📄 License

MIT
