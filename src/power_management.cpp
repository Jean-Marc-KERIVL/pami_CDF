// Implémentation gestion alimentation

#include "power_management.hpp"
#include "driver/i2c.h"
#include <cstdio>
#include <cmath>

// I2C pins pour ESP32
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_FREQ_HZ 100000

Battery::Battery(uint8_t id, uint8_t i2c_addr)
    : battery_id_(id), i2c_addr_(i2c_addr), voltage_(0), current_(0),
      power_(0), is_connected_(false), max_voltage_(12), min_voltage_(9) {

    printf("[Battery%d] Initialized at I2C address 0x%02X\n", id, i2c_addr);
}

void Battery::read_voltage() {
    // Simulation pour tests (remplacer par vrai lecteur INA219)
    voltage_ = max_voltage_ - (random() % 30) / 10.0f;
    is_connected_ = true;

    printf("[Battery%d] Voltage: %.2fV\n", battery_id_, voltage_);
}

void Battery::read_current() {
    // Simulation pour tests
    current_ = (random() % 50) * 10.0f;  // 0-500mA

    printf("[Battery%d] Current: %.1fmA\n", battery_id_, current_);
}

void Battery::read_power() {
    read_voltage();
    read_current();

    power_ = (voltage_ * current_) / 1000.0f;

    printf("[Battery%d] Power: %.2fW\n", battery_id_, power_);
}

void Battery::check_low_voltage() {
    if (voltage_ < min_voltage_) {
        printf("[Battery%d] WARNING: Low voltage (%.2fV < %.2fV)\n",
               battery_id_, voltage_, min_voltage_);
    }
}

BatteryStatus Battery::get_status() const {
    return BatteryStatus{
        .battery_id = battery_id_,
        .voltage = voltage_,
        .current = current_,
        .power = power_,
        .is_connected = is_connected_
    };
}

// ============= PowerManagement =============

PowerManagement::PowerManagement()
    : battery1(1, 0x40), battery2(2, 0x41),
      voltage_12v_(12), voltage_5v_(5), voltage_3v3_(3.3),
      emergency_stop_active_(false) {

    // Initialiser I2C
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .master = {.clk_speed = I2C_MASTER_FREQ_HZ},
    };

    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, I2C_MODE_MASTER, 0, 0, 0);

    printf("[PowerManagement] I2C initialized\n");
}

void PowerManagement::read_all_voltages() {
    battery1.read_voltage();
    battery2.read_voltage();

    printf("[PowerManagement] Total voltage: %.2fV\n",
           battery1.voltage_ + battery2.voltage_);
}

void PowerManagement::read_all_currents() {
    battery1.read_current();
    battery2.read_current();

    printf("[PowerManagement] Total current: %.1fmA\n",
           battery1.current_ + battery2.current_);
}

bool PowerManagement::check_system_health() {
    read_all_voltages();

    bool bat1_ok = battery1.voltage_ >= battery1.min_voltage_;
    bool bat2_ok = battery2.voltage_ >= battery2.min_voltage_;

    if (!bat1_ok || !bat2_ok) {
        printf("[PowerManagement] WARNING: Low voltage detected!\n");
        return false;
    }

    printf("[PowerManagement] System healthy\n");
    return true;
}

SystemPowerStatus PowerManagement::get_summary() {
    read_all_voltages();
    read_all_currents();
    battery1.read_power();
    battery2.read_power();

    return SystemPowerStatus{
        .battery1 = battery1.get_status(),
        .battery2 = battery2.get_status(),
        .system_voltage = battery1.voltage_ + battery2.voltage_,
        .system_current = battery1.current_ + battery2.current_,
        .system_power = battery1.power_ + battery2.power_,
        .emergency_stop_active = emergency_stop_active_
    };
}

void PowerManagement::emergency_stop() {
    emergency_stop_active_ = true;
    printf("[PowerManagement] EMERGENCY STOP ACTIVATED\n");
}

void PowerManagement::resume_operation() {
    emergency_stop_active_ = false;
    printf("[PowerManagement] Resumed operation\n");
}

float PowerManagement::estimate_runtime(float consumption_per_hour_wh) {
    float total_voltage = battery1.voltage_ + battery2.voltage_;
    float total_capacity_wh = total_voltage * 10;  // Estimation simple

    float runtime_hours = total_capacity_wh / consumption_per_hour_wh;

    printf("[PowerManagement] Estimated runtime: %.1f hours\n", runtime_hours);
    return runtime_hours;
}
