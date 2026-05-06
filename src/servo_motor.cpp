// Implémentation contrôle moteurs SERVO42D

#include "servo_motor.hpp"
#include "driver/uart.h"
#include <cstdio>
#include <cstring>

// UART pins pour ESP32
#define UART_TX_PIN 17
#define UART_RX_PIN 16
#define UART_NUM UART_NUM_1
#define UART_BAUD_RATE 115200
#define UART_BUF_SIZE 256

ServoMotor::ServoMotor(uint8_t id, int uart_num)
    : motor_id_(id), uart_num_(uart_num) {
    state_.motor_id = id;
    state_.position = 0;
    state_.enabled = false;
    state_.speed = 500;
    state_.torque = 100;
    state_.microstepping = 1;

    // Initialiser UART si premier moteur
    if (id == 1) {
        uart_config_t uart_config = {
            .baud_rate = UART_BAUD_RATE,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        };

        uart_param_config(UART_NUM, &uart_config);
        uart_set_pin(UART_NUM, UART_TX_PIN, UART_RX_PIN,
                     UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
        uart_driver_install(UART_NUM, UART_BUF_SIZE * 2, UART_BUF_SIZE * 2, 0, nullptr, 0);

        printf("[ServoMotor] UART initialized on pins TX:%d RX:%d\n", UART_TX_PIN, UART_RX_PIN);
    }
}

ServoMotor::~ServoMotor() {
    if (state_.enabled) {
        disable();
    }
}

void ServoMotor::move_steps(int32_t steps) {
    if (!state_.enabled) {
        printf("[ServoMotor%d] Motor must be enabled first\n", motor_id_);
        return;
    }

    state_.position += steps;

    char cmd[64];
    snprintf(cmd, sizeof(cmd), "M%d:MOVE:%ld:%u\n",
             motor_id_, labs(steps), state_.speed);

    send_command(cmd);
    printf("[ServoMotor%d] Moved %ld steps (pos: %ld)\n", motor_id_, steps, state_.position);
}

void ServoMotor::go_to_position(int32_t target_pos) {
    if (target_pos < 0) {
        printf("[ServoMotor%d] Invalid position\n", motor_id_);
        return;
    }

    int32_t steps = target_pos - state_.position;
    move_steps(steps);
}

void ServoMotor::home() {
    move_steps(-state_.position);
    state_.position = 0;
    printf("[ServoMotor%d] Homed to position 0\n", motor_id_);
}

void ServoMotor::enable() {
    state_.enabled = true;
    send_command("ENABLE");
    printf("[ServoMotor%d] Enabled\n", motor_id_);
}

void ServoMotor::disable() {
    state_.enabled = false;
    send_command("DISABLE");
    printf("[ServoMotor%d] Disabled\n", motor_id_);
}

void ServoMotor::set_speed(uint32_t steps_per_second) {
    state_.speed = steps_per_second;

    char cmd[64];
    snprintf(cmd, sizeof(cmd), "M%d:SPEED:%u\n", motor_id_, steps_per_second);
    send_command(cmd);

    printf("[ServoMotor%d] Speed set to %u steps/s\n", motor_id_, steps_per_second);
}

void ServoMotor::set_torque(uint16_t percent) {
    if (percent > 100) percent = 100;

    state_.torque = percent;

    char cmd[64];
    snprintf(cmd, sizeof(cmd), "M%d:TORQUE:%u\n", motor_id_, percent);
    send_command(cmd);

    printf("[ServoMotor%d] Torque set to %u%%\n", motor_id_, percent);
}

void ServoMotor::set_microstepping(uint8_t division) {
    if (division != 1 && division != 2 && division != 4 &&
        division != 8 && division != 16) {
        printf("[ServoMotor%d] Invalid microstepping value\n", motor_id_);
        return;
    }

    state_.microstepping = division;

    char cmd[64];
    snprintf(cmd, sizeof(cmd), "M%d:MICROSTEPPING:%u\n", motor_id_, division);
    send_command(cmd);

    printf("[ServoMotor%d] Microstepping set to 1/%u\n", motor_id_, division);
}

ServoMotorState ServoMotor::get_status() const {
    return state_;
}

void ServoMotor::send_command(const std::string& cmd) {
    uart_write_bytes(UART_NUM, cmd.c_str(), cmd.length());
    printf("[UART TX] M%d: %s", motor_id_, cmd.c_str());
}

// ============= DualServoMotor =============

DualServoMotor::DualServoMotor(int uart_num)
    : motor1(1, uart_num), motor2(2, uart_num) {
}

void DualServoMotor::enable_all() {
    motor1.enable();
    motor2.enable();
}

void DualServoMotor::disable_all() {
    motor1.disable();
    motor2.disable();
}

void DualServoMotor::move_both(int32_t steps1, int32_t steps2) {
    motor1.move_steps(steps1);
    motor2.move_steps(steps2);
}

void DualServoMotor::differential(int32_t left, int32_t right) {
    printf("[DualServoMotor] Differential: L:%ld R:%ld\n", left, right);
    move_both(left, right);
}

void DualServoMotor::turn_in_place(int32_t steps, int direction) {
    int32_t left = direction > 0 ? steps : -steps;
    int32_t right = direction > 0 ? -steps : steps;

    printf("[DualServoMotor] Turning in place: %ld steps, direction: %d\n", steps, direction);
    differential(left, right);
}
