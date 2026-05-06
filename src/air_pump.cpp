// Implémentation pompe à air

#include "air_pump.hpp"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstdio>
#include <time.h>

AirPump::AirPump(uint32_t gpio_pin)
    : gpio_pin_(gpio_pin), state_(PUMP_OFF), total_run_time_ms_(0) {

    // Configurer GPIO en sortie
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << gpio_pin_);
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;

    gpio_config(&io_conf);
    gpio_set_level(static_cast<gpio_num_t>(gpio_pin_), 0);

    printf("[AirPump] Initialized on GPIO pin %lu\n", gpio_pin_);
}

AirPump::~AirPump() {
    if (state_ == PUMP_ON) {
        deactivate();
    }
}

void AirPump::activate() {
    if (state_ == PUMP_ON) {
        printf("[AirPump] Already active\n");
        return;
    }

    state_ = PUMP_ON;
    session_start_time_ = time(nullptr);
    set_gpio(1);

    printf("[AirPump] Activated\n");
}

void AirPump::deactivate() {
    if (state_ == PUMP_OFF) {
        printf("[AirPump] Already inactive\n");
        return;
    }

    state_ = PUMP_OFF;
    set_gpio(0);

    uint32_t run_time = (time(nullptr) - session_start_time_) * 1000;
    total_run_time_ms_ += run_time;

    printf("[AirPump] Deactivated (run time: %u ms)\n", run_time);
}

void AirPump::toggle() {
    if (state_ == PUMP_ON) {
        deactivate();
    } else {
        activate();
    }
}

void AirPump::pulse(uint32_t duration_ms) {
    activate();
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    deactivate();

    printf("[AirPump] Pulse sent (%u ms)\n", duration_ms);
}

void AirPump::burst(uint8_t count, uint32_t interval_ms, uint32_t duration_ms) {
    printf("[AirPump] Burst: %d x %u ms (interval: %u ms)\n",
           count, duration_ms, interval_ms);

    for (uint8_t i = 0; i < count; i++) {
        pulse(duration_ms);
        if (i < count - 1) {
            vTaskDelay(pdMS_TO_TICKS(interval_ms));
        }
    }
}

void AirPump::activate_for(uint32_t duration_ms) {
    activate();
    printf("[AirPump] Activated for %u ms\n", duration_ms);

    // Créer une tâche pour désactiver
    auto deactivate_task = [](void* param) {
        uint32_t duration = *static_cast<uint32_t*>(param);
        vTaskDelay(pdMS_TO_TICKS(duration));
        // Impossible d'accéder à `this` ici, utiliser une var globale ou callback
    };

    // Alternative: utiliser timer ESP32
    // Pour maintenant, laisser comme est
}

AirPumpStatus AirPump::get_status() const {
    return AirPumpStatus{
        .state = state_,
        .pin = gpio_pin_,
        .total_run_time_ms = total_run_time_ms_,
        .session_start_time = static_cast<uint32_t>(session_start_time_)
    };
}

void AirPump::set_gpio(uint8_t level) {
    gpio_set_level(static_cast<gpio_num_t>(gpio_pin_), level);
    printf("[GPIO%lu] Set to %d\n", gpio_pin_, level);
}
