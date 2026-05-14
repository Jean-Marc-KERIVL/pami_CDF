// Servo.cpp - Implémentation servo via LEDC (ESP32 Arduino core v3)
#include "Servo.h"

// Période 20 ms (50 Hz) avec 16 bits de résolution -> 65535 ticks pour 20 ms
static constexpr int SERVO_FREQ_HZ   = 50;
static constexpr int SERVO_RES_BITS  = 16;
static constexpr unsigned long PERIOD_US = 20000UL;
static constexpr unsigned long MAX_DUTY  = (1UL << SERVO_RES_BITS) - 1;

Servo::Servo(int pin, int pulse_min_us, int pulse_max_us)
    : _pin(pin),
      _pulse_min_us(pulse_min_us),
      _pulse_max_us(pulse_max_us),
      _last_angle(-1),
      _up(false),
      _t_last_toggle(0) {}

void Servo::begin() {
    ledcAttach(_pin, SERVO_FREQ_HZ, SERVO_RES_BITS);
    // Position initiale : milieu (90°)
    setAngle(90);
}

void Servo::setAngle(int angle_deg) {
    if (angle_deg < 0)   angle_deg = 0;
    if (angle_deg > 180) angle_deg = 180;
    if (angle_deg == _last_angle) return;   // évite les écritures inutiles
    _last_angle = angle_deg;

    // Conversion angle -> pulse en µs
    unsigned long pulse_us = map(angle_deg, 0, 180, _pulse_min_us, _pulse_max_us);
    // Conversion pulse -> duty (16 bits, période 20 ms)
    unsigned long duty = (pulse_us * MAX_DUTY) / PERIOD_US;

    ledcWrite(_pin, duty);
}

void Servo::toggleUpDown(unsigned long period_ms,
                        int angle_low_deg,
                        int angle_high_deg) {
    unsigned long now = millis();
    if (now - _t_last_toggle >= period_ms) {
        _t_last_toggle = now;
        _up = !_up;
        setAngle(_up ? angle_high_deg : angle_low_deg);
    }
}
