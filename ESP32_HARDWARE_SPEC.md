# 🔧 Spécifications Matériel ESP32 PAMI Robot

## 📋 Résumé du projet
Carte de contrôle pour robot PAMI basée sur **ESP32** avec:
- 2x moteurs pas à pas SERVO42D NEMA17 (Closed Loop)
- 1x pompe à air
- 3x limit switches
- 3x LED strips
- Mesure de tension 2x batteries (INA219)
- Connecteurs JST coudés sur les côtés
- Taille minimisée avec entraxe de trous standard

---

## 🔌 Alimentation

### Entrées
- **JST 60A**: 2x connecteurs (options jumper 12V / 24V)
- **Diode Schottky** sur VIN
- Pas de switch (bouton d'arrêt d'urgence existant)

### Sorties regulées
- **12V**: Pour servos et Raspberry Pi convertisseur
- **5V**: Pour logique et relays
- **3V3**: Pour ESP32, capteurs
- **3V3_2**: Header optionnel (femelle + mâle)

### Réduction puissance LED
- ❌ LED power trop puissante actuellement
- ✅ À changer: résistance plus grosse (R1 = 470Ω → 2.2kΩ)

---

## 🎯 Configuration des pins ESP32

### Communication
| Signal | Pin | Type | Fonction |
|--------|-----|------|----------|
| RX | 16 | INPUT | UART moteurs SERVO42D |
| TX | 17 | OUTPUT | UART moteurs SERVO42D |
| SDA | 21 | I2C | I2C bus INA219 |
| SCL | 22 | I2C | I2C bus INA219 |

### Capteurs (INPUT ONLY)
| Signal | Pin | Type | Fonction |
|--------|-----|------|----------|
| LIMIT_1 | 34 | INPUT | Limit switch 1 |
| LIMIT_2 | 35 | INPUT | Limit switch 2 |
| LIMIT_3 | 36 | INPUT | Limit switch 3 (VP) |

### Sorties (GPIO + PWM)
| Signal | Pin | Type | Fonction |
|--------|-----|------|----------|
| LED_1 | 12 | PWM | LED strip 1 (3V3 via convertisseur) |
| LED_2 | 13 | PWM | LED strip 2 (3V3 via convertisseur) |
| LED_3 | 27 | PWM | LED strip 3 (3V3 via convertisseur) |
| AIR_PUMP | 14 | GPIO | Relay pompe à air |
| EMG_STOP | 32 | INPUT | Bouton d'arrêt d'urgence |

### Pin spéciales
| Signal | Type | Fonction |
|--------|------|----------|
| EN | RESET | Reset (petit condo: EN-GND) |

---

## 🔗 Connecteurs JST

### Moteurs (JST PH 3.96)
```
Connecteur Moteur 1 & 2 vers MKS SERVO42D:
Pin 1: STEP
Pin 2: DIR
Pin 3: ENABLE
```

### Pompe à air (JST XH 2.54)
```
Pin 1: 5V
Pin 2: GND
Pin 3: Relay control (pin 14)
```

### Limit Switches (JST PH 2.0)
```
Pin 1: GND
Pin 2: Signal (pins 34/35/36)
```

### LEDs (JST PH 2.0 ou 3.96)
```
Pin 1: 3V3
Pin 2: Signal (pins 12/13/27)
Pin 3: GND
```

### I2C (connecteur standard)
```
Pin 1: GND
Pin 2: VCC (3V3)
Pin 3: SDA (pin 21)
Pin 4: SCL (pin 22)

⚠️ À définir: combien de connecteurs I2C?
- Option A: 1x connecteur + carte de dérive
- Option B: 2-3x connecteurs pour scalabilité
```

### Batterie (JST 60A)
```
Connecteur 1: Batterie 1 (12V ou 24V)
Connecteur 2: Batterie 2 (12V ou 24V)
Jumper: Sélectionner 12V ou 24V
```

### Servo (RX/TX UART)
```
Pin 1: VCC (5V ou 3V3)
Pin 2: GND
Pin 3: RX (pin 16)
Pin 4: TX (pin 17)
```

### Bouton d'arrêt d'urgence
```
Connecteur JST 2.0:
Pin 1: GND
Pin 2: Signal (pin 32)
```

### Reset (optionnel)
```
Connecteur: Mini JST
Pin 1: EN
Pin 2: GND
Avec condensateur 10µF entre EN et GND
```

---

## 📏 Dimensions et trous

### Taille de la carte
- Dimensions: À définir (valeurs rondes en mm)
- Exemple proposé: **80mm x 60mm** (standard format robot)

### Trous de montage
- Entraxe standard: **50mm x 40mm** (ou 60mm x 45mm)
- Diamètre trous: **M3** (vis de montage)
- Position: coins arrondis pour sécurité

---

## 🔋 Mesure batterie (INA219)

### Configuration
- **INA219 #1**: Adresse I2C 0x40 (batterie 1)
- **INA219 #2**: Adresse I2C 0x41 (batterie 2)

### Alternativ: Mesure par résistances
```
Si pas d'INA219, utiliser ADC + diviseur résistif:
- Batterie 1 → résistance 10k:10k → ADC pin
- Batterie 2 → résistance 10k:10k → ADC pin
⚠️ Adapter facteur de conversion (1.1V / 4096 pour ESP32)
```

---

## ⚡ Convertisseurs DC/DC

### Nécessaires
- **12V/24V → 5V**: Pour logique + relays (≥500mA)
- **12V/24V → 3V3**: Pour ESP32 + capteurs (≥1A)

### Optionnel
- **5V → 3V3**: Pour LED strips (conversion logique niveau)
  - Actuel: LEDs sur 12V → à changer (12V trop pour logique)
  - Solution: Convertisseur ou MOSFET logique niveau

---

## 🎛️ Mode à implémenter

### Version mini (minimum)
```
- Mouvements: forward, backward, turn
- Pompe: on/off
- LEDs: on/off simples
```

### Version avancée
```
- Mouvements: differential drive, courbes
- Pompe: pulse, burst, patterns
- LEDs: PWM, breathing, patterns
- Capteurs: limit switch feedback
- Puissance: monitoring batterie
- UART: commandes moteurs complexes
```

---

## 📝 Checklist implémentation

- [ ] Schématique avec tous les connecteurs JST
- [ ] Placement composants (ESP32 au centre, connecteurs sur côtés)
- [ ] Routing PCB (couches simples possible?)
- [ ] Sélectionner exactement quel connecteur I2C (1 vs 3)
- [ ] Taille finale + entraxe trous (valeurs rondes)
- [ ] Résistances pull-up/down limit switches
- [ ] Diode protection VIN
- [ ] Résistance LED power (2.2kΩ)
- [ ] Petit condensateur EN/GND (10µF)
- [ ] Tester avec SERVO42D réel
- [ ] Fab. et tests proto

---

## 🔗 Références GitHub

- **Repo principal**: [2025-2026_Top_secret_ROS_Control](https://github.com/py-Alexis/2025-2026_Top_secret_ROS_Control)
- **Autre équipe**: [2024-2025-Robotics-cup](https://github.com/ECN-Nantrobot/2024-2025-Robotics-cup)

---

## 📞 Questions ouvertes

1. **Modèle moteur exact**: SERVO42D oui, mais variante précise?
2. **Connecteurs I2C**: 1 connecteur (+ carte dérive) ou plusieurs?
3. **Pull-up/down limit switches**: Quel mode?
4. **Taille PCB finale**: Dimensions précises?
5. **Convertisseur 5V→3V3**: Utilisé ou non?
6. **Ordre pins I2C**: GND, VCC, SDA, SCL? (ou autre?)

---

**Date**: Mai 2026  
**Version**: 1.0  
**Status**: Design en cours
