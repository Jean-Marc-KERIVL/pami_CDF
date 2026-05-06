// Contrôle LED strips via PWM

#ifndef LED_CONTROL_HPP
#define LED_CONTROL_HPP

#include <cstdint>

typedef struct {
    uint8_t strip_id;
    uint32_t pin;
    uint8_t brightness;  // 0-255
    bool is_on;
} LEDStatus;

class LEDStrip {
public:
    LEDStrip(uint8_t id, uint32_t pwm_pin);

    void on();
    void off();
    void toggle();
    void set_brightness(uint8_t value);
    void brighten(uint8_t step = 10);
    void dim(uint8_t step = 10);

    void pulse(uint32_t duration_ms, uint8_t intensity = 255);
    void blink(uint8_t count, uint32_t duration_ms);
    void breathe(uint32_t cycle_duration_ms);

    LEDStatus get_status() const;

private:
    uint8_t strip_id_;
    uint32_t pwm_pin_;
    uint8_t brightness_;
    bool is_on_;
    uint32_t pwm_frequency_;
    uint8_t pwm_resolution_;

    void apply_brightness();
};

// Gérer 3 LED strips
class LEDStrips {
public:
    LEDStrips();

    void all_on();
    void all_off();
    void set_brightness_all(uint8_t value);
    void rainbow();
    void blink_all(uint8_t count, uint32_t duration_ms);
    void breathe_all(uint32_t cycle_duration_ms);

    LEDStrip led1, led2, led3;

private:
    // helpers
};

#endif // LED_CONTROL_HPP
