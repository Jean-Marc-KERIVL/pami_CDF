// Système intégré ROS 2 pour robot PAMI

#ifndef PAMI_ROBOT_HPP
#define PAMI_ROBOT_HPP

#include "servo_motor.hpp"
#include "air_pump.hpp"
#include "limit_switches.hpp"
#include "led_control.hpp"
#include "power_management.hpp"

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <memory>

// ROS 2 Topics et Services
typedef struct {
    float linear_x;   // m/s
    float angular_z;  // rad/s
} CmdVel;

typedef struct {
    float x, y;           // Position
    float theta;          // Orientation
    float linear_vel_x;
    float angular_vel_z;
} Odometry;

class PAMIRobot {
public:
    PAMIRobot();
    ~PAMIRobot();

    // Lifecycle
    bool init();
    bool startup();
    void shutdown();

    // ROS 2 spin
    void spin();
    rcl_node_t* get_node() { return &node_; }

    // Composants
    DualServoMotor motors_;
    AirPump air_pump_;
    LimitSwitches limit_switches_;
    LEDStrips leds_;
    PowerManagement power_;

    // Contrôle
    void set_cmd_vel(const CmdVel& cmd);
    Odometry get_odometry() const;

    // Diagnostics
    void print_diagnostics();

private:
    // ROS 2 nodes
    rcl_allocator_t allocator_;
    rclc_support_t support_;
    rcl_node_t node_;
    rclc_executor_t executor_;

    // ROS 2 subscriptions (commandes)
    rcl_subscription_t cmd_vel_sub_;
    rcl_subscription_t air_pump_sub_;
    rcl_subscription_t led_control_sub_;

    // ROS 2 publishers (états)
    rcl_publisher_t odom_pub_;
    rcl_publisher_t imu_pub_;
    rcl_publisher_t power_status_pub_;
    rcl_publisher_t sensor_status_pub_;

    // ROS 2 services
    rcl_service_t motor_cmd_srv_;
    rcl_service_t homing_srv_;
    rcl_service_t emergency_stop_srv_;

    // État interne
    Odometry odometry_;
    CmdVel current_cmd_vel_;
    bool is_running_;

    // Callbacks
    static void cmd_vel_callback(const void* msgin);
    static void air_pump_callback(const void* msgin);
    static void led_control_callback(const void* msgin);
};

#endif // PAMI_ROBOT_HPP
