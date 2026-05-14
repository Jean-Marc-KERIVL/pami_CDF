// Encoder.h - Encodeur quadrature avec interruption
#pragma once

#include <Arduino.h>

class Encoder {
public:
    Encoder(int pin_a, int pin_b);
    void begin(void (*isr_func)());     // attache l'ISR fourni par l'utilisateur
    long getCount();                    // lecture atomique
    void reset();
    void handleInterrupt();             // appelé depuis l'ISR

private:
    int  _pin_a;
    int  _pin_b;
    volatile long _count;
    volatile bool _last_a;
};
