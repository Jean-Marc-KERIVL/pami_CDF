// Servo.h - Servo standard 50 Hz via LEDC PWM ESP32
#pragma once

#include <Arduino.h>

class Servo {
public:
    // angle_min..angle_max sont en degrés (0..180).
    // pulse_min_us..pulse_max_us sont les pulses correspondants (500..2500 µs typique).
    Servo(int pin,
          int pulse_min_us = 500,
          int pulse_max_us = 2500);

    void begin();

    // Bouge le servo à l'angle indiqué (0..180 °)
    void setAngle(int angle_deg);

    // Bouge haut <-> bas selon un timer interne (non-bloquant).
    // À appeler dans la loop.
    void toggleUpDown(unsigned long period_ms,
                      int angle_low_deg = 0,
                      int angle_high_deg = 180);

private:
    int  _pin;
    int  _pulse_min_us;
    int  _pulse_max_us;
    int  _last_angle;
    bool _up;
    unsigned long _t_last_toggle;
};
