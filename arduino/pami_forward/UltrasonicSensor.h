// UltrasonicSensor.h - Capteur de distance HC-SR04
#pragma once

#include <Arduino.h>

class UltrasonicSensor {
public:
    UltrasonicSensor(int trig_pin, int echo_pin,
                     unsigned long timeout_us = 25000UL);

    void begin();

    // Mesure unique. Retourne distance en cm, ou -1 si pas d'écho.
    long readCm();

    int trigPin() const { return _trig; }
    int echoPin() const { return _echo; }

private:
    int _trig;
    int _echo;
    unsigned long _timeout_us;
};
