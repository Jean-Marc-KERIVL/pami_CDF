// Robot PAMI - Contrôle des mouvements et du bras

class Robot {
  constructor() {
    this.position = { x: 0, y: 0 };
    this.direction = 0; // 0=Nord, 90=Est, 180=Sud, 270=Ouest
    this.armPosition = 0; // 0=bas, 1=haut
    this.speed = 0.3; // mètres par commande
    this.history = [];
  }

  // Mouvements de base
  forward(distance = this.speed) {
    const rad = (this.direction * Math.PI) / 180;
    this.position.x += distance * Math.cos(rad);
    this.position.y += distance * Math.sin(rad);
    this._logAction(`Avance de ${distance}m`);
    return this;
  }

  backward(distance = this.speed) {
    const rad = (this.direction * Math.PI) / 180;
    this.position.x -= distance * Math.cos(rad);
    this.position.y -= distance * Math.sin(rad);
    this._logAction(`Recule de ${distance}m`);
    return this;
  }

  turnRight(angle = 90) {
    this.direction = (this.direction + angle) % 360;
    this._logAction(`Tourne à droite ${angle}°`);
    return this;
  }

  turnLeft(angle = 90) {
    this.direction = (this.direction - angle + 360) % 360;
    this._logAction(`Tourne à gauche ${angle}°`);
    return this;
  }

  // Demi-tour (180°)
  turnAround() {
    this.direction = (this.direction + 180) % 360;
    this._logAction(`Demi-tour (180°)`);
    return this;
  }

  // Mouvements latéraux (strafe)
  moveRight(distance = this.speed) {
    const rad = ((this.direction + 90) * Math.PI) / 180;
    this.position.x += distance * Math.cos(rad);
    this.position.y += distance * Math.sin(rad);
    this._logAction(`Aller à droite ${distance}m`);
    return this;
  }

  moveLeft(distance = this.speed) {
    const rad = ((this.direction - 90) * Math.PI) / 180;
    this.position.x += distance * Math.cos(rad);
    this.position.y += distance * Math.sin(rad);
    this._logAction(`Aller à gauche ${distance}m`);
    return this;
  }

  // Contrôle du bras
  armUp() {
    this.armPosition = 1;
    this._logAction(`Bras vers le HAUT`);
    return this;
  }

  armDown() {
    this.armPosition = 0;
    this._logAction(`Bras vers le BAS`);
    return this;
  }

  armToggle() {
    this.armPosition = this.armPosition === 0 ? 1 : 0;
    this._logAction(`Bras basculé (${this.armPosition === 0 ? 'BAS' : 'HAUT'})`);
    return this;
  }

  // Commandes combinées
  grabBlock() {
    if (this.armPosition === 0) {
      this._logAction(`⚠️ Bras doit être levé! Levant le bras...`);
      this.armUp();
    }
    this._logAction(`Saisit un bloc`);
    return this;
  }

  placeBlock() {
    if (this.armPosition === 1) {
      this._logAction(`Bras abaissé pour placer le bloc`);
      this.armDown();
    }
    this._logAction(`Place un bloc`);
    return this;
  }

  // Utilitaires
  getStatus() {
    const directions = ['Nord', 'Est', 'Sud', 'Ouest'];
    const dirIndex = this.direction / 90;
    return {
      position: this.position,
      direction: `${this.direction}° (${directions[dirIndex % 4]})`,
      armPosition: this.armPosition === 0 ? 'BAS' : 'HAUT',
      history: this.history
    };
  }

  _logAction(action) {
    const status = `[${this.history.length + 1}] ${action} | Pos: (${this.position.x.toFixed(2)}, ${this.position.y.toFixed(2)}) | Dir: ${this.direction}° | Bras: ${this.armPosition === 0 ? '↓' : '↑'}`;
    this.history.push(status);
    console.log(status);
  }

  reset() {
    this.position = { x: 0, y: 0 };
    this.direction = 0;
    this.armPosition = 0;
    this.history = [];
    console.log('🔄 Robot réinitialisé');
    return this;
  }

  // Parser de commandes texte
  executeCommand(command) {
    const cmd = command.toLowerCase().trim();
    const match = cmd.match(/(\w+)\s*(\d+)?/);

    if (!match) return this;

    const action = match[1];
    const value = match[2] ? parseInt(match[2]) : null;

    switch (action) {
      case 'avance':
      case 'forward':
        return this.forward(value || this.speed);
      case 'recule':
      case 'backward':
        return this.backward(value || this.speed);
      case 'droite':
      case 'right':
        return this.turnRight(value || 90);
      case 'gauche':
      case 'left':
        return this.turnLeft(value || 90);
      case 'demitour':
      case 'turnaround':
        return this.turnAround();
      case 'brasup':
      case 'armup':
        return this.armUp();
      case 'brasdown':
      case 'armdown':
        return this.armDown();
      case 'saisir':
      case 'grab':
        return this.grabBlock();
      case 'placer':
      case 'place':
        return this.placeBlock();
      default:
        console.log(`❌ Commande inconnue: ${action}`);
        return this;
    }
  }
}

module.exports = Robot;
