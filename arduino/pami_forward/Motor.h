// Motor.h - Contrôle d'un moteur DC via L298N (2 pins PWM)
#pragma once

#include <Arduino.h>

class Motor {
public:
    Motor(int pin_in1, int pin_in2, bool inverted = false);

    // Doit être appelé dans setup() après begin()
    void begin();

    // Vitesse signée: positive = avant, négative = arrière, 0 = stop
    void setSpeed(int speed);

    // Arrêt immédiat (roue libre)
    void stop();

    int  getSpeed() const { return _last_speed; }
    bool isInverted() const { return _inverted; }

private:
    int  _pin_in1;
    int  _pin_in2;
    bool _inverted;
    int  _last_speed;
};
