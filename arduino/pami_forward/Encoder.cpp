// Encoder.cpp
#include "Encoder.h"

Encoder::Encoder(int pin_a, int pin_b)
    : _pin_a(pin_a), _pin_b(pin_b), _count(0), _last_a(false) {}

void Encoder::begin(void (*isr_func)()) {
    pinMode(_pin_a, INPUT_PULLUP);
    pinMode(_pin_b, INPUT_PULLUP);
    _last_a = digitalRead(_pin_a);
    attachInterrupt(digitalPinToInterrupt(_pin_a), isr_func, CHANGE);
}

long Encoder::getCount() {
    noInterrupts();
    long c = _count;
    interrupts();
    return c;
}

void Encoder::reset() {
    noInterrupts();
    _count = 0;
    interrupts();
}

void IRAM_ATTR Encoder::handleInterrupt() {
    bool a = digitalRead(_pin_a);
    bool b = digitalRead(_pin_b);
    if (a != _last_a) {
        if (a == b) {
            _count++;
        } else {
            _count--;
        }
    }
    _last_a = a;
}
