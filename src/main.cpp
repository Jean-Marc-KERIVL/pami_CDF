// Main application - ROS 2 Robot PAMI ESP32

#include "pami_robot.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <cstdio>

static const char* TAG = "PAMI_MAIN";

// Instance du robot (globale pour accès facile)
PAMIRobot* g_robot = nullptr;

// Tâche ROS 2 spin (sur core 1)
void ros2_spin_task(void* arg) {
    ESP_LOGI(TAG, "ROS2 Spin Task Started");

    PAMIRobot* robot = static_cast<PAMIRobot*>(arg);
    robot->spin();

    vTaskDelete(nullptr);
}

// Tâche de monitoring (sur core 0)
void monitoring_task(void* arg) {
    PAMIRobot* robot = static_cast<PAMIRobot*>(arg);
    int count = 0;

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(5000));  // Chaque 5 secondes

        count++;
        if (count % 3 == 0) {  // Tous les 15 secondes
            robot->print_diagnostics();
        }

        // Monitoring basique
        printf("[Monitor] Tick %d\n", count);
    }
}

extern "C" void app_main() {
    printf("\n\n");
    printf("========================================\n");
    printf("  PAMI Robot - ROS 2 Microcontroller   \n");
    printf("  ESP32 + micro-ROS                    \n");
    printf("========================================\n\n");

    // Créer instance du robot
    g_robot = new PAMIRobot();

    // Démarrer le robot
    if (!g_robot->startup()) {
        ESP_LOGE(TAG, "Failed to startup robot");
        delete g_robot;
        return;
    }

    // Afficher diagnostics initiaux
    g_robot->print_diagnostics();

    // Démarrer tâche ROS 2 spin sur core 1
    xTaskCreatePinnedToCore(
        ros2_spin_task,           // Fonction
        "ros2_spin",              // Nom
        8192,                     // Stack size
        g_robot,                  // Paramètre
        1,                        // Priorité
        nullptr,                  // Handle
        1                         // Core 1
    );

    // Démarrer tâche de monitoring sur core 0
    xTaskCreatePinnedToCore(
        monitoring_task,
        "monitoring",
        4096,
        g_robot,
        2,
        nullptr,
        0
    );

    // Main ne revient pas (géré par FreeRTOS)
    printf("PAMI Robot running. All tasks started.\n");
}

// ============= TESTS =============

// Uncomment pour tester sans ROS 2

/*
extern "C" void app_main() {
    printf("\n=== PAMI Robot Test Mode ===\n\n");

    // Créer composants séparément pour testing
    ServoMotor motor1(1);
    motor1.enable();
    motor1.set_speed(1000);
    motor1.move_steps(100);
    motor1.disable();

    AirPump pump;
    pump.activate();
    vTaskDelay(pdMS_TO_TICKS(500));
    pump.deactivate();
    pump.pulse(100);

    LimitSwitches switches;
    switches.read_all();

    LEDStrips leds;
    leds.all_on();
    vTaskDelay(pdMS_TO_TICKS(1000));
    leds.all_off();
    leds.led1.pulse(200);

    PowerManagement power;
    power.read_all_voltages();
    auto status = power.get_summary();
    printf("System voltage: %.2f V\n", status.system_voltage);

    printf("\n=== Tests Complete ===\n");
}
*/
