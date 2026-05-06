// Implémentation système ROS 2 pour robot PAMI

#include "pami_robot.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstdio>

PAMIRobot::PAMIRobot()
    : motors_(), air_pump_(), limit_switches_(), leds_(), power_(),
      current_cmd_vel_{0, 0}, is_running_(false) {

    printf("[PAMIRobot] Initialized\n");
}

PAMIRobot::~PAMIRobot() {
    if (is_running_) {
        shutdown();
    }
}

bool PAMIRobot::init() {
    printf("[PAMIRobot] Initializing ROS 2...\n");

    // Initialiser allocateur
    allocator_ = rcl_get_default_allocator();

    // Initialiser support ROS 2
    if (rclc_support_init(&support_, 0, nullptr, &allocator_) != RCL_RET_OK) {
        printf("[PAMIRobot] Failed to init ROS 2 support\n");
        return false;
    }

    // Créer nœud ROS 2
    if (rclc_node_init_default(&node_, "pami_robot_esp32", "", &support_) != RCL_RET_OK) {
        printf("[PAMIRobot] Failed to create ROS 2 node\n");
        return false;
    }

    printf("[PAMIRobot] ROS 2 initialized successfully\n");
    return true;
}

bool PAMIRobot::startup() {
    printf("[PAMIRobot] Starting up...\n");

    if (!init()) {
        return false;
    }

    // Vérifier l'alimentation
    power_.read_all_voltages();
    if (!power_.check_system_health()) {
        printf("[PAMIRobot] Power check failed\n");
        return false;
    }

    // Allumer LED de puissance
    leds_.led1.set_brightness(100);

    // Activer les moteurs
    motors_.enable_all();

    // Réinitialiser les limit switches
    limit_switches_.reset_all();

    is_running_ = true;

    printf("[PAMIRobot] Started successfully\n");
    printf("[PAMIRobot] Ready for ROS 2 commands\n");

    return true;
}

void PAMIRobot::shutdown() {
    printf("[PAMIRobot] Shutting down...\n");

    motors_.disable_all();
    air_pump_.deactivate();
    leds_.all_off();

    is_running_ = false;

    printf("[PAMIRobot] Shutdown complete\n");
}

void PAMIRobot::set_cmd_vel(const CmdVel& cmd) {
    if (!is_running_) {
        printf("[PAMIRobot] Not running, ignoring cmd_vel\n");
        return;
    }

    current_cmd_vel_ = cmd;

    // Convertir cmd_vel en steps moteurs
    // linear_x en m/s -> steps
    // angular_z en rad/s -> differential drive

    const float steps_per_meter = 1000;  // À calibrer
    int32_t left_steps = static_cast<int32_t>(cmd.linear_x * steps_per_meter);
    int32_t right_steps = static_cast<int32_t>(cmd.linear_x * steps_per_meter);

    // Ajouter rotation différentielle
    int32_t turn_steps = static_cast<int32_t>(cmd.angular_z * 100);
    left_steps -= turn_steps;
    right_steps += turn_steps;

    motors_.move_both(left_steps, right_steps);

    printf("[PAMIRobot] cmd_vel: linear=%.2f, angular=%.2f\n",
           cmd.linear_x, cmd.angular_z);
}

Odometry PAMIRobot::get_odometry() const {
    // Estimer odométrie basée sur position moteurs
    odometry_.x = motors_.motor1.get_position() / 1000.0f;
    odometry_.y = motors_.motor2.get_position() / 1000.0f;
    odometry_.theta = 0;  // À calculer selon orientation
    odometry_.linear_vel_x = current_cmd_vel_.linear_x;
    odometry_.angular_vel_z = current_cmd_vel_.angular_z;

    return odometry_;
}

void PAMIRobot::spin() {
    printf("[PAMIRobot] Spinning...\n");

    // Créer executor
    if (rclc_executor_init(&executor_, &support_.context, 1, &allocator_) != RCL_RET_OK) {
        printf("[PAMIRobot] Failed to init executor\n");
        return;
    }

    // Spin indéfiniment
    while (is_running_) {
        rclc_executor_spin_some(&executor_, RCL_MS_TO_NS(100));
        vTaskDelay(pdMS_TO_TICKS(100));

        // Publier l'état périodiquement
        Odometry odom = get_odometry();
        printf("[Odometry] x=%.2f, y=%.2f, theta=%.2f\n",
               odom.x, odom.y, odom.theta);

        // Vérifier capteurs
        limit_switches_.read_all();

        // Lire alimentation
        power_.read_all_voltages();
    }

    rcl_ret_t rc = rcl_node_fini(&node_);
    if (rc != RCL_RET_OK) {
        printf("[PAMIRobot] Failed to finalize node\n");
    }
}

void PAMIRobot::print_diagnostics() {
    printf("\n=== PAMI ROBOT DIAGNOSTICS ===\n\n");

    // Moteurs
    printf("--- Motors ---\n");
    auto motor1_status = motors_.motor1.get_status();
    auto motor2_status = motors_.motor2.get_status();

    printf("Motor 1: pos=%ld, enabled=%d, speed=%u\n",
           motor1_status.position, motor1_status.enabled, motor1_status.speed);
    printf("Motor 2: pos=%ld, enabled=%d, speed=%u\n",
           motor2_status.position, motor2_status.enabled, motor2_status.speed);

    // Pompe à air
    printf("\n--- Air Pump ---\n");
    auto pump_status = air_pump_.get_status();
    printf("State: %s, Total run time: %u ms\n",
           pump_status.state == 0 ? "OFF" : "ON",
           pump_status.total_run_time_ms);

    // Limit switches
    printf("\n--- Limit Switches ---\n");
    limit_switches_.read_all();
    for (uint8_t i = 1; i <= 3; i++) {
        auto sw = limit_switches_.get(i);
        if (sw) {
            printf("Switch %d: %s\n", i,
                   sw->is_pressed() ? "TRIGGERED" : "OK");
        }
    }

    // LEDs
    printf("\n--- LED Strips ---\n");
    auto led1_status = leds_.led1.get_status();
    auto led2_status = leds_.led2.get_status();
    auto led3_status = leds_.led3.get_status();

    printf("LED 1: brightness=%d, state=%s\n",
           led1_status.brightness, led1_status.is_on ? "ON" : "OFF");
    printf("LED 2: brightness=%d, state=%s\n",
           led2_status.brightness, led2_status.is_on ? "ON" : "OFF");
    printf("LED 3: brightness=%d, state=%s\n",
           led3_status.brightness, led3_status.is_on ? "ON" : "OFF");

    // Alimentation
    printf("\n--- Power System ---\n");
    auto power_status = power_.get_summary();

    printf("Battery 1: %.2fV, %.1fmA, %.2fW\n",
           power_status.battery1.voltage,
           power_status.battery1.current,
           power_status.battery1.power);
    printf("Battery 2: %.2fV, %.1fmA, %.2fW\n",
           power_status.battery2.voltage,
           power_status.battery2.current,
           power_status.battery2.power);
    printf("System: %.2fV, %.1fmA, %.2fW\n",
           power_status.system_voltage,
           power_status.system_current,
           power_status.system_power);
    printf("Emergency Stop: %s\n",
           power_status.emergency_stop_active ? "ACTIVE" : "INACTIVE");

    printf("\n================================\n\n");
}
