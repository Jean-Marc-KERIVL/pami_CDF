// Implémentation contrôle LED strips

#include "led_control.hpp"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstdio>
#include <cmath>

// Configuration PWM
#define PWM_FREQUENCY 5000  // Hz
#define PWM_RESOLUTION LEDC_TIMER_8_BIT  // 0-255

// Pins pour ESP32
#define LED_STRIP_1_PIN GPIO_NUM_12
#define LED_STRIP_2_PIN GPIO_NUM_13
#define LED_STRIP_3_PIN GPIO_NUM_27

LEDStrip::LEDStrip(uint8_t id, uint32_t pwm_pin)
    : strip_id_(id), pwm_pin_(pwm_pin), brightness_(0), is_on_(false),
      pwm_frequency_(PWM_FREQUENCY), pwm_resolution_(8) {

    // Configurer LED PWM
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = PWM_RESOLUTION,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = pwm_frequency_,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ledc_timer_config(&timer_conf);

    ledc_channel_config_t channel_conf = {
        .gpio_num = static_cast<int>(pwm_pin_),
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = static_cast<ledc_channel_t>(id),
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };

    ledc_channel_config(&channel_conf);

    printf("[LEDStrip%d] Configured on GPIO %lu (PWM %uHz)\n",
           id, pwm_pin_, pwm_frequency_);
}

void LEDStrip::on() {
    set_brightness(255);
    printf("[LEDStrip%d] Turned ON\n", strip_id_);
}

void LEDStrip::off() {
    set_brightness(0);
    printf("[LEDStrip%d] Turned OFF\n", strip_id_);
}

void LEDStrip::toggle() {
    if (is_on_) {
        off();
    } else {
        on();
    }
}

void LEDStrip::set_brightness(uint8_t value) {
    if (value > 255) value = 255;

    brightness_ = value;
    is_on_ = (value > 0);

    apply_brightness();

    uint8_t percent = (value * 100) / 255;
    printf("[LEDStrip%d] Brightness: %d/255 (%d%%)\n",
           strip_id_, value, percent);
}

void LEDStrip::brighten(uint8_t step) {
    uint16_t new_val = static_cast<uint16_t>(brightness_) + step;
    if (new_val > 255) new_val = 255;

    set_brightness(static_cast<uint8_t>(new_val));
}

void LEDStrip::dim(uint8_t step) {
    int16_t new_val = static_cast<int16_t>(brightness_) - step;
    if (new_val < 0) new_val = 0;

    set_brightness(static_cast<uint8_t>(new_val));
}

void LEDStrip::pulse(uint32_t duration_ms, uint8_t intensity) {
    set_brightness(intensity);
    vTaskDelay(pdMS_TO_TICKS(duration_ms / 2));
    set_brightness(0);
    vTaskDelay(pdMS_TO_TICKS(duration_ms / 2));

    printf("[LEDStrip%d] Pulse (%u ms)\n", strip_id_, duration_ms);
}

void LEDStrip::blink(uint8_t count, uint32_t duration_ms) {
    printf("[LEDStrip%d] Blinking %d times (%u ms)\n",
           strip_id_, count, duration_ms);

    uint32_t half = duration_ms / 2;
    for (uint8_t i = 0; i < count; i++) {
        set_brightness(255);
        vTaskDelay(pdMS_TO_TICKS(half));
        set_brightness(0);
        vTaskDelay(pdMS_TO_TICKS(half));
    }
}

void LEDStrip::breathe(uint32_t cycle_duration_ms) {
    printf("[LEDStrip%d] Breathing (%u ms cycle)\n",
           strip_id_, cycle_duration_ms);

    const uint32_t steps = 50;
    uint32_t step_duration = cycle_duration_ms / (2 * steps);

    // Augmenter progressivement
    for (uint32_t i = 0; i < steps; i++) {
        uint8_t val = static_cast<uint8_t>((i * 255) / steps);
        brightness_ = val;
        apply_brightness();
        vTaskDelay(pdMS_TO_TICKS(step_duration));
    }

    // Diminuer progressivement
    for (uint32_t i = steps; i > 0; i--) {
        uint8_t val = static_cast<uint8_t>((i * 255) / steps);
        brightness_ = val;
        apply_brightness();
        vTaskDelay(pdMS_TO_TICKS(step_duration));
    }
}

LEDStatus LEDStrip::get_status() const {
    uint8_t percent = (brightness_ * 100) / 255;
    return LEDStatus{
        .strip_id = strip_id_,
        .pin = pwm_pin_,
        .brightness = brightness_,
        .is_on = is_on_
    };
}

void LEDStrip::apply_brightness() {
    ledc_set_duty(LEDC_HIGH_SPEED_MODE,
                  static_cast<ledc_channel_t>(strip_id_),
                  brightness_);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE,
                     static_cast<ledc_channel_t>(strip_id_));
}

// ============= LEDStrips =============

LEDStrips::LEDStrips()
    : led1(1, LED_STRIP_1_PIN),
      led2(2, LED_STRIP_2_PIN),
      led3(3, LED_STRIP_3_PIN) {

    printf("[LEDStrips] Initialized 3 LED strips\n");
}

void LEDStrips::all_on() {
    led1.on();
    led2.on();
    led3.on();
}

void LEDStrips::all_off() {
    led1.off();
    led2.off();
    led3.off();
}

void LEDStrips::set_brightness_all(uint8_t value) {
    led1.set_brightness(value);
    led2.set_brightness(value);
    led3.set_brightness(value);
}

void LEDStrips::rainbow() {
    led1.set_brightness(255);
    led2.set_brightness(128);
    led3.set_brightness(64);

    printf("[LEDStrips] Rainbow mode\n");
}

void LEDStrips::blink_all(uint8_t count, uint32_t duration_ms) {
    printf("[LEDStrips] Blinking all %d times\n", count);

    uint32_t half = duration_ms / 2;
    for (uint8_t i = 0; i < count; i++) {
        all_on();
        vTaskDelay(pdMS_TO_TICKS(half));
        all_off();
        vTaskDelay(pdMS_TO_TICKS(half));
    }
}

void LEDStrips::breathe_all(uint32_t cycle_duration_ms) {
    printf("[LEDStrips] Breathing all LEDs\n");

    const uint32_t steps = 50;
    uint32_t step_duration = cycle_duration_ms / (2 * steps);

    for (uint32_t i = 0; i < steps; i++) {
        uint8_t val = static_cast<uint8_t>((i * 255) / steps);
        set_brightness_all(val);
        vTaskDelay(pdMS_TO_TICKS(step_duration));
    }

    for (uint32_t i = steps; i > 0; i--) {
        uint8_t val = static_cast<uint8_t>((i * 255) / steps);
        set_brightness_all(val);
        vTaskDelay(pdMS_TO_TICKS(step_duration));
    }
}
