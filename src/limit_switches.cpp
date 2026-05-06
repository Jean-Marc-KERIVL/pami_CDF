// Implémentation limit switches

#include "limit_switches.hpp"
#include "driver/gpio.h"
#include <cstdio>

// Pins pour ESP32 (input only sur 34, 35, 36)
#define LIMIT_SWITCH_1_PIN 34
#define LIMIT_SWITCH_2_PIN 35
#define LIMIT_SWITCH_3_PIN 36

LimitSwitch::LimitSwitch(uint8_t id, uint32_t gpio_pin)
    : switch_id_(id), gpio_pin_(gpio_pin), is_triggered_(false), total_triggers_(0) {

    // Configurer GPIO en entrée
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << gpio_pin_);
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;

    gpio_config(&io_conf);

    printf("[LimitSwitch%d] Initialized on GPIO pin %lu\n", id, gpio_pin_);
}

void LimitSwitch::read() {
    uint32_t level = gpio_get_level(static_cast<gpio_num_t>(gpio_pin_));
    is_triggered_ = (level == 0);  // Active LOW

    if (is_triggered_) {
        total_triggers_++;
        printf("[LimitSwitch%d] Triggered (count: %u)\n", switch_id_, total_triggers_);
    }
}

void LimitSwitch::reset() {
    is_triggered_ = false;
    printf("[LimitSwitch%d] Reset\n", switch_id_);
}

LimitSwitchStatus LimitSwitch::get_status() const {
    return LimitSwitchStatus{
        .switch_id = switch_id_,
        .pin = gpio_pin_,
        .is_triggered = is_triggered_,
        .total_triggers = total_triggers_
    };
}

// ============= LimitSwitches =============

LimitSwitches::LimitSwitches()
    : switch1(1, LIMIT_SWITCH_1_PIN),
      switch2(2, LIMIT_SWITCH_2_PIN),
      switch3(3, LIMIT_SWITCH_3_PIN) {

    switches_[0] = &switch1;
    switches_[1] = &switch2;
    switches_[2] = &switch3;

    printf("[LimitSwitches] Initialized 3 switches\n");
}

void LimitSwitches::read_all() {
    for (auto sw : switches_) {
        if (sw) {
            sw->read();
        }
    }
}

bool LimitSwitches::is_triggered(uint8_t switch_id) {
    if (switch_id < 1 || switch_id > 3) return false;

    return switches_[switch_id - 1]->is_pressed();
}

LimitSwitch* LimitSwitches::get(uint8_t switch_id) {
    if (switch_id < 1 || switch_id > 3) return nullptr;

    return switches_[switch_id - 1];
}

void LimitSwitches::reset_all() {
    for (auto sw : switches_) {
        if (sw) {
            sw->reset();
        }
    }

    printf("[LimitSwitches] All switches reset\n");
}
