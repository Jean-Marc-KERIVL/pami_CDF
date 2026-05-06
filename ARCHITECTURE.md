# 🏗️ Architecture PAMI Robot - Vue d'ensemble

Système complet de contrôle de robot avec deux implémentations:
1. **JavaScript/Node.js** - Simulation et prototypage rapide
2. **C++/ROS 2** - Production sur ESP32

## 📊 Diagramme Architecture

```
┌──────────────────────────────────────────────────────────────┐
│                    PAMI Robot Control System                 │
└──────────────────────────────────────────────────────────────┘

┌─────────────────────────┐      ┌──────────────────────────┐
│  JavaScript/Node.js     │      │  C++ / ROS 2 / ESP32     │
│  (Prototyping)          │      │  (Production)            │
├─────────────────────────┤      ├──────────────────────────┤
│ • robot.js              │      │ • pami_robot.hpp/cpp     │
│ • servo-motor.js        │      │ • servo_motor.hpp/cpp    │
│ • air-pump.js           │      │ • air_pump.hpp/cpp       │
│ • limit-switches.js     │      │ • limit_switches.hpp/cpp │
│ • led-control.js        │      │ • led_control.hpp/cpp    │
│ • power-management.js   │      │ • power_management.hpp   │
│ • esp32-robot.js        │      │                          │
│ • index.js              │      │ ROS 2 Topics:            │
│                         │      │ • /cmd_vel               │
│ Tests: npm start        │      │ • /odom                  │
│                         │      │ • /sensor_status         │
│                         │      │ • /power_status          │
└─────────────────────────┘      └──────────────────────────┘
        │                                   │
        │ Simulation                        │ Hardware
        │ (Tests logiques)                  │ (ESP32)
        │                                   │
        ▼                                   ▼
┌───────────────────────────────────────────────────────────────┐
│           Hardware Components (Identiques)                    │
├───────────────────────────────────────────────────────────────┤
│                                                               │
│  Moteurs:              SERVO42D NEMA17 (x2)                  │
│  Pompe à air:          GPIO Relay (pin 14)                   │
│  Capteurs:             Limit switches (34, 35, 36)           │
│  LEDs:                 LED strips PWM (12, 13, 27)           │
│  Batterie:             INA219 (I2C 21, 22)                   │
│  Communication:        UART (16, 17) + I2C                   │
│                                                               │
└───────────────────────────────────────────────────────────────┘
```

## 📦 Contenu du Repo

### JavaScript (Prototypage)
```
├── robot.js                  # Classe Robot de base
├── esp32-robot.js           # Système intégré ESP32
├── servo-motor.js           # Moteurs SERVO42D
├── air-pump.js              # Pompe à air
├── limit-switches.js        # Capteurs limites
├── led-control.js           # LEDs
├── power-management.js      # Batterie + INA219
├── esp32-pins.js            # Configuration pins
├── index.js                 # Tests simples
├── EXAMPLES.md              # 40+ exemples
└── README.md                # Documentation Node.js
```

### C++ / ROS 2 / ESP32
```
├── include/
│   ├── pami_robot.hpp           # Système intégré ROS 2
│   ├── servo_motor.hpp
│   ├── air_pump.hpp
│   ├── limit_switches.hpp
│   ├── led_control.hpp
│   └── power_management.hpp
├── src/
│   ├── main.cpp                 # Point d'entrée
│   ├── pami_robot.cpp
│   ├── servo_motor.cpp
│   ├── air_pump.cpp
│   ├── limit_switches.cpp
│   ├── led_control.cpp
│   └── power_management.cpp
├── CMakeLists.txt              # Build config
├── sdkconfig.defaults          # ESP32 config
├── idf_component.yml           # Component manifest
├── ROS2_GUIDE.md              # ROS 2 guide
└── README_CPP.md              # Documentation C++
```

### Documentation
```
├── README.md                    # Vue générale
├── README_CPP.md               # C++ guide
├── ROS2_GUIDE.md               # ROS 2 integration
├── ESP32_HARDWARE_SPEC.md      # Specs matériel
├── EXAMPLES.md                 # Exemples JavaScript
└── ARCHITECTURE.md             # Cette file
```

## 🎯 Cas d'usage

### 1. Développement rapide (JavaScript)
```bash
npm start                        # Tester la logique
node esp32-robot.js            # Tests moteurs/pompe
```

### 2. Prototypage sur simulated hardware
```javascript
const PAMI_Robot = require('./esp32-robot');
const robot = new PAMI_Robot();

robot.startup();
robot.moveForward(0.5);
robot.airPump.activate();
robot.diagnostics();
```

### 3. Déploiement ESP32 réel
```bash
idf.py set-target esp32
idf.py build
idf.py -p /dev/ttyUSB0 flash
idf.py -p /dev/ttyUSB0 monitor
```

### 4. Intégration ROS 2
```bash
# Terminal 1: Agent micro-ROS
micro-ros-agent serial --dev /dev/ttyUSB0

# Terminal 2: Commandes
ros2 topic pub /cmd_vel geometry_msgs/Twist \
  "linear: {x: 0.5}"
```

## 🔄 Flux de développement recommandé

```
1. Conception & Tests
   └─> JavaScript (rapid iteration)
   
2. Prototypage Hardware
   └─> Tester sur ESP32 en C++
   
3. Intégration ROS 2
   └─> ROS 2 topics/services
   
4. Validation Système
   └─> Navigation + autonomie
```

## 🌳 Hiérarchie des Classes

### JavaScript
```
Robot (base)
├── forward()
├── backward()
├── turnRight()
├── moveRight()
├── armUp()
└── armDown()

PAMIRobot (intégration)
├── motors_ (DualServoMotor)
├── air_pump_ (AirPump)
├── limit_switches_ (LimitSwitches)
├── leds_ (LEDStrips)
├── power_ (PowerManagement)
└── movement (Robot)
```

### C++ / ROS 2
```
ServoMotor
├── move_steps()
├── enable()/disable()
├── set_speed()
└── set_torque()

DualServoMotor
├── motor1, motor2
├── differential()
└── turn_in_place()

AirPump
├── activate()/deactivate()
├── pulse()
└── burst()

LimitSwitches
├── switch1, switch2, switch3
├── read_all()
└── wait_for_trigger()

LEDStrips
├── led1, led2, led3
├── set_brightness()
├── breathe()
└── blink()

PowerManagement
├── battery1, battery2
├── check_system_health()
└── estimate_runtime()

PAMIRobot (ROS 2)
├── startup()/shutdown()
├── set_cmd_vel()
├── get_odometry()
└── spin() [ROS 2 loop]
```

## 📡 Communication

### JavaScript (Node.js)
- **No networking** - tout en mémoire
- **Simulation pure** - pas d'I/O matériel

### C++ (ESP32)

#### UART
- **Moteurs** (115200 baud)
  - Format: `M{id}:{cmd}\n`
  - Ex: `M1:MOVE:100:1000\n`

#### I2C
- **Batteries** (INA219 - 0x40, 0x41)
- **Capteurs** (futurs)

#### ROS 2 / micro-ROS
- **DDS over Serial** ou **UDP** (WiFi)
- Topics, Services, Parameters

## ⚙️ Configuration

### Pins ESP32
```
GPIO 12, 13, 27     LEDs (PWM)
GPIO 14             Pompe (relay)
GPIO 16, 17         UART moteurs
GPIO 21, 22         I2C batterie
GPIO 32             Bouton urgence
GPIO 34, 35, 36     Limit switches
```

### Frequences
```
PWM LEDs:          5kHz
I2C:               100kHz
UART moteurs:      115200 baud
ROS 2 spin:        10Hz
```

## 📈 Roadmap

### ✅ Complété
- [x] Architecture modulaire
- [x] Classes moteurs/pompe/LEDs
- [x] JavaScript prototype
- [x] C++ implementation
- [x] ROS 2 integration
- [x] FreeRTOS tasks
- [x] Power management

### 🔄 En cours
- [ ] Tests hardware réel
- [ ] Calibration moteurs
- [ ] Navigation (nav2)
- [ ] SLAM optionnel

### 📋 Futur
- [ ] Machine Learning inference
- [ ] Computer vision
- [ ] Swarming (multi-robot)
- [ ] Cloud connectivity

## 🔗 Intégration avec l'écosystème ROS 2

```
PAMI Robot (micro-ROS)
    │
    ├─→ ROS 2 Navigation Stack (nav2)
    │   ├─ Path Planning
    │   ├─ Local Planner
    │   └─ Cost Maps
    │
    ├─→ ROS 2 Perception
    │   ├─ Camera drivers
    │   └─ Object detection
    │
    └─→ ROS 2 Manipulation
        ├─ MoveIt
        └─ Gripper control
```

## 📚 Ressources

| Type | Lien |
|------|------|
| ESP-IDF | https://docs.espressif.com/projects/esp-idf/ |
| micro-ROS | https://micro-ros.github.io/ |
| ROS 2 Humble | https://docs.ros.org/en/humble/ |
| FreeRTOS | https://www.freertos.org/ |
| SERVO42D | [Datasheet à vérifier] |

## 🤝 Contributing

Pour ajouter une feature:

1. **Implémenter en JavaScript** (tests rapides)
   ```javascript
   class NewComponent {
       constructor() {}
       do_something() { }
   }
   ```

2. **Traduire en C++**
   ```cpp
   class NewComponent {
   public:
       NewComponent() {}
       void do_something() { }
   };
   ```

3. **Intégrer dans PAMIRobot**
   ```cpp
   class PAMIRobot {
   public:
       NewComponent component_;
   };
   ```

4. **Ajouter ROS 2 topics/services** si nécessaire

## 🐛 Debugging

### JavaScript
```bash
node index.js
node esp32-robot.js
DEBUG=* node esp32-robot.js
```

### C++ / ESP32
```bash
idf.py build
idf.py -p /dev/ttyUSB0 flash
idf.py -p /dev/ttyUSB0 monitor | grep "PAMIRobot"
```

### ROS 2
```bash
ros2 node list
ros2 topic list
ros2 topic echo /cmd_vel
ros2 service call /robot/homing std_srvs/srv/Trigger
```

---

**Version**: 1.0  
**Status**: Architecture complete ✅  
**Last Update**: Mai 2026  
**Author**: Jean-Marc KERVIL
