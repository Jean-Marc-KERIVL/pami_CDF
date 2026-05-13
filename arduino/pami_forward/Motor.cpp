// Motor.cpp
#include "Motor.h"
#include "Pins.h"

Motor::Motor(int pin_in1, int pin_in2, bool inverted)
    : _pin_in1(pin_in1), _pin_in2(pin_in2), _inverted(inverted), _last_speed(0) {}

void Motor::begin() {
    ledcAttach(_pin_in1, Config::PWM_FREQ_HZ, Config::PWM_RESOLUTION);
    ledcAttach(_pin_in2, Config::PWM_FREQ_HZ, Config::PWM_RESOLUTION);
    stop();
}

void Motor::setSpeed(int speed) {
    _last_speed = speed;

    // Clamp -255..255
    if (speed >  255) speed =  255;
    if (speed < -255) speed = -255;

    // Si moteur monté en miroir, on inverse le sens logiquement
    if (_inverted) speed = -speed;

    if (speed > 0) {
        // Avance: IN1 = PWM, IN2 = 0
        ledcWrite(_pin_in1, speed);
        ledcWrite(_pin_in2, 0);
    } else if (speed < 0) {
        // Recule: IN1 = 0, IN2 = PWM
        ledcWrite(_pin_in1, 0);
        ledcWrite(_pin_in2, -speed);
    } else {
        // Stop
        ledcWrite(_pin_in1, 0);
        ledcWrite(_pin_in2, 0);
    }
}

void Motor::stop() {
    setSpeed(0);
}
