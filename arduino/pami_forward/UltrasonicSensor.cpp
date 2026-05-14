// UltrasonicSensor.cpp
#include "UltrasonicSensor.h"

UltrasonicSensor::UltrasonicSensor(int trig_pin, int echo_pin,
                                   unsigned long timeout_us)
    : _trig(trig_pin), _echo(echo_pin), _timeout_us(timeout_us) {}

void UltrasonicSensor::begin() {
    pinMode(_trig, OUTPUT);
    pinMode(_echo, INPUT);
    digitalWrite(_trig, LOW);
}

long UltrasonicSensor::readCm() {
    // Impulsion 10 µs sur TRIG
    digitalWrite(_trig, LOW);
    delayMicroseconds(3);
    digitalWrite(_trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(_trig, LOW);

    unsigned long duration = pulseIn(_echo, HIGH, _timeout_us);
    if (duration == 0) return -1;

    // Vitesse du son ≈ 343 m/s -> 0.0343 cm/µs, aller-retour donc /2
    return static_cast<long>(duration * 0.0343f / 2.0f);
    
}
