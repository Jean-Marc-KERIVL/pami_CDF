# 🤖 ROS 2 Integration Guide - PAMI Robot ESP32

## Architecture ROS 2

Le robot PAMI implémente une architecture **micro-ROS** pour l'ESP32, permettant l'intégration avec un système ROS 2 principal.

### Composants ROS 2

```
┌─────────────────────────────────────────┐
│     ROS 2 Desktop (Ubuntu/macOS)        │
│     - ros2 CLI                          │
│     - rqt monitoring                    │
│     - Navigation stack                  │
└────────────────┬────────────────────────┘
                 │ DDS Bridge / UART
                 │
┌────────────────┴────────────────────────┐
│    micro-ROS Agent (ESP32)              │
│  - /cmd_vel (input)                     │
│  - /odom (output)                       │
│  - /sensor_state (output)               │
│  - /power_status (output)               │
└─────────────────────────────────────────┘
```

## Topics ROS 2

### Subscriptions (Inputs)

#### `/cmd_vel` (geometry_msgs/Twist)
Commandes de mouvement du robot.

```cpp
typedef struct {
    float linear_x;   // m/s - vitesse linéaire avant/arrière
    float angular_z;  // rad/s - vitesse angulaire de rotation
} CmdVel;
```

**Exemple:**
```bash
ros2 topic pub /cmd_vel geometry_msgs/Twist \
  "linear: {x: 0.5, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.0}"
```

#### `/air_pump_cmd` (std_msgs/Int8)
Contrôle pompe à air: `-1`=pulse, `0`=off, `1`=on, `2`=burst

#### `/led_control` (std_msgs/Int32)
Contrôle LEDs: bits 0-7 pour luminosité LED1, bits 8-15 pour LED2, bits 16-23 pour LED3

### Publications (Outputs)

#### `/odom` (nav_msgs/Odometry)
Odométrie basée sur position moteurs.

```
position: x, y, theta
velocity: linear_x, angular_z
```

#### `/sensor_status` (std_msgs/UInt16)
État des limit switches et autres capteurs.

```
Bits:
- 0-2: Limit switches (1 = triggered)
- 3-5: Réservé
- 6-7: Réservé
```

#### `/power_status` (Custom Message)
État du système électrique.

```
battery1_voltage: float32
battery1_current: float32
battery2_voltage: float32
battery2_current: float32
emergency_stop: bool
```

## Services ROS 2

### `/robot/homing` (std_srvs/Trigger)
Retour à la position de départ avec les limit switches.

```bash
ros2 service call /robot/homing std_srvs/srv/Trigger
```

### `/robot/emergency_stop` (std_srvs/Trigger)
Arrêt d'urgence complet du robot.

### `/robot/motor_command` (Custom Service)
Commande moteur avancée.

```
Request:
  motor_id: uint8
  command: string (MOVE, SPEED, TORQUE, etc.)
  value: int32

Response:
  success: bool
  message: string
```

## Installation et Build

### Prérequis

1. **ESP-IDF 5.0+**
   ```bash
   git clone https://github.com/espressif/esp-idf.git
   cd esp-idf && git checkout v5.0
   ./install.sh
   source export.sh
   ```

2. **micro-ROS pour ESP32**
   ```bash
   mkdir -p ~/ros2_pami_ws/src
   cd ~/ros2_pami_ws/src

   git clone -b humble https://github.com/micro-ROS/micro_ros_setup.git
   cd micro_ros_setup && bash install_dependencies.sh
   cd ~/ros2_pami_ws
   ```

3. **ROS 2 Desktop** (sur machine hôte)
   - Ubuntu: `sudo apt install ros-humble-desktop`
   - macOS: Voir [ROS 2 Installation Guide](https://docs.ros.org/en/humble/)

### Build pour ESP32

```bash
cd ~/ros2_pami_ws

# Build micro-ROS Agent
cd ~/ros2_pami_ws/src
git clone https://github.com/micro-ROS/micro_ros_espidf_component.git

# Build le firmware PAMI Robot
cd ~/ros2_pami_ws
colcon build --merge-install

# Flash sur ESP32
idf.py -p /dev/ttyUSB0 flash
idf.py -p /dev/ttyUSB0 monitor
```

## Utilisation

### 1. Lancer micro-ROS Agent (si pas de bridge automatique)

```bash
micro-ros-agent serial --dev /dev/ttyUSB0
```

### 2. Vérifier le nœud ROS 2

```bash
ros2 node list
# Devrait afficher: /pami_robot_esp32

ros2 topic list
# Devrait afficher les topics disponibles
```

### 3. Téléopération basique

```bash
# Terminal 1: Démarrer le robot
idf.py -p /dev/ttyUSB0 monitor

# Terminal 2: Envoyer commandes
ros2 topic pub --rate 10 /cmd_vel geometry_msgs/Twist \
  "linear: {x: 0.3}, angular: {z: 0.0}"
```

### 4. Monitoring en temps réel

```bash
# Voir l'odométrie
ros2 topic echo /odom

# Voir l'état des capteurs
ros2 topic echo /sensor_status

# Voir l'alimentation
ros2 topic echo /power_status
```

### 5. Tester services

```bash
# Homing
ros2 service call /robot/homing std_srvs/srv/Trigger

# Arrêt d'urgence
ros2 service call /robot/emergency_stop std_srvs/srv/Trigger
```

## Exemples ROS 2

### Python - Contrôle simple

```python
#!/usr/bin/env python3

import rclpy
from geometry_msgs.msg import Twist

def main():
    rclpy.init()
    node = rclpy.create_node('pami_teleop')
    
    pub = node.create_publisher(Twist, '/cmd_vel', 10)
    
    # Avancer 0.5 m/s
    cmd = Twist()
    cmd.linear.x = 0.5
    pub.publish(cmd)
    
    # Attendre
    node.create_timer(2.0, lambda: None)
    
    rclpy.spin(node)

if __name__ == '__main__':
    main()
```

### C++ - Homing avec feedback

```cpp
#include "rclcpp/rclcpp.hpp"
#include "std_srvs/srv/trigger.hpp"

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    
    auto node = rclcpp::create_node("pami_homing");
    
    auto client = node->create_client<std_srvs::srv::Trigger>(
        "/robot/homing");
    
    auto request = std::make_shared<std_srvs::srv::Trigger::Request>();
    
    // Appeler le service
    auto result = client->async_send_request(request);
    
    RCLCPP_INFO(node->get_logger(), "Homing started");
    
    rclcpp::spin(node);
    rclcpp::shutdown();
    
    return 0;
}
```

## Debugging

### Voir les logs ESP32

```bash
# Live logs
idf.py -p /dev/ttyUSB0 monitor

# Avec filtrage
idf.py -p /dev/ttyUSB0 monitor | grep "PAMIRobot"
```

### Enregistrer tous les topics

```bash
ros2 bag record -a -o pami_run_$(date +%Y%m%d_%H%M%S)
```

### Tester latence

```bash
# Publier avec timestamp
ros2 topic pub /cmd_vel geometry_msgs/Twist \
  "linear: {x: 0.1}" --rate 100
```

## Troubleshooting

### Problème: "DDS Agent not found"
- Vérifier que l'agent micro-ROS est lancé
- Vérifier le port série: `ls /dev/tty*`

### Problème: "Firmware crashes"
- Augmenter la taille du stack (voir CMakeLists.txt)
- Réduire la fréquence de spin (100ms par défaut)

### Problème: "Topics not visible"
- Vérifier les interfaces ROS 2: `ros2 interface list`
- Republier les topics manuellement

## Architecture Future

Pour une intégration nav2 complète:

```
ROS 2 Nav2 Stack
    ├── Path Planner
    ├── Controller
    └── Costmap

    ↓ /cmd_vel

PAMI Robot (ESP32)
    ├── Motor Driver
    ├── Sensors
    └── Power Management

    ↑ /odom, /sensor_state

    ↓ Closed Loop Control
```

## Ressources

- [ROS 2 Humble Documentation](https://docs.ros.org/en/humble/)
- [micro-ROS for ESP-IDF](https://micro-ros.github.io/docs/tutorials/core/esp32/esp32_basic_example/)
- [DDS for microcontrollers](https://micro-ros.github.io/docs/concepts/middleware/dds/)

---

**Status**: Implémentation de base complète ✅  
**TODO**: Service motor_command, messages personnalisées, nav2 integration

