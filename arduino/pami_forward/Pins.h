// Pins.h - Câblage matériel PAMI Robot
//
//   Hardware:
//     - ESP32 DevKit
//     - Module MJKDZ (driver L298N dual H-bridge)
//     - 2 moteurs DC
//     - Capteur ultrasons HC-SR04
//
//   ⚠️ IMPORTANT: LEFT_IN2 a été déplacé de TX (GPIO1) vers D23
//   pour libérer le Serial USB. Vérifie que ton fil est bien sur D23.

#pragma once

namespace Pins {
    // Moteur droit (via MJKDZ IN1/IN2)
    constexpr int RIGHT_IN1 = 19;   // D19
    constexpr int RIGHT_IN2 = 21;   // D21

    // Moteur gauche (via MJKDZ IN3/IN4)
    constexpr int LEFT_IN1  = 18;   // D18
    constexpr int LEFT_IN2  = 23;   // D23 (déplacé de TX/GPIO1)

    // Capteur ultrasons HC-SR04
    constexpr int US_TRIG   = 27;   // D27
    constexpr int US_ECHO   = 14;   // D14

    // LED interne ESP32
    constexpr int LED       = 2;
}

namespace Config {
    // PWM
    constexpr int  PWM_FREQ_HZ       = 1000;   // 1 kHz
    constexpr int  PWM_RESOLUTION    = 8;      // 0..255

    // Capteur ultrasons
    constexpr unsigned long ECHO_TIMEOUT_US = 25000UL;  // ~4 m max

    // Comportement
    constexpr int  DIST_STOP_CM      = 15;     // distance d'arrêt
    constexpr int  DIST_FULL_CM      = 60;     // distance pleine vitesse
    constexpr int  SPEED_MIN         = 90;     // PWM min pour démarrer le moteur
    constexpr int  SPEED_MAX         = 255;    // PWM max

    // Timings
    constexpr unsigned long STARTUP_DELAY_MS = 2000;
    constexpr unsigned long LOOP_PERIOD_MS   = 50;
    constexpr unsigned long PRINT_PERIOD_MS  = 200;
    constexpr unsigned long SERIAL_BAUD      = 115200;
}
