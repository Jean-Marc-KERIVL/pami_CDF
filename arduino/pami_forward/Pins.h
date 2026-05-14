// Pins.h - Câblage matériel PAMI Robot
//
//   Hardware:
//     - ESP32 DevKit
//     - Module MJKDZ (driver L298N dual H-bridge)
//     - 2 moteurs DC
//     - Capteur ultrasons HC-SR04
//     - Servo (D26)
//     - Starter (D32, vers GND par défaut, libéré au lancement)

#pragma once

namespace Pins {
    // Moteur droit
    constexpr int RIGHT_IN1 = 19;
    constexpr int RIGHT_IN2 = 21;

    // Moteur gauche
    constexpr int LEFT_IN1  = 18;
    constexpr int LEFT_IN2  = 17;     // TX2 (GPIO17) — UART2, Serial USB OK

    // Ultrasons HC-SR04
    constexpr int US_TRIG   = 27;
    constexpr int US_ECHO   = 14;

    // Servo (bras / pince)
    constexpr int SERVO     = 26;

    // Starter (fil reliant à GND, retiré pour démarrer)
    constexpr int STARTER   = 32;

    // LED interne
    constexpr int LED       = 2;
}

namespace Config {
    // PWM moteurs
    constexpr int  PWM_FREQ_HZ       = 1000;
    constexpr int  PWM_RESOLUTION    = 8;       // 0..255

    // Vitesse de croisière (0..255)
    constexpr int  SPEED_CRUISE      = 220;
    constexpr int  SPEED_TURN        = 180;     // pour le demi-tour sur place

    // Capteur ultrasons
    constexpr unsigned long ECHO_TIMEOUT_US = 25000UL;  // ~4 m
    constexpr int  DIST_STOP_CM      = 10;      // <10 cm -> arrêt
    constexpr int  DIST_HYST_CM      =  3;      // +3 cm pour repartir

    // Séquence
    constexpr unsigned long STARTER_DEBOUNCE_MS  = 50;
    constexpr unsigned long WAIT_AFTER_STARTER_MS = 90000UL;  // 90 s
    constexpr unsigned long FORWARD_DURATION_MS  = 10000UL;   // 10 s
    constexpr unsigned long UTURN_DURATION_MS    = 1500UL;    // ~1.5 s, à calibrer

    // Servo
    constexpr unsigned long SERVO_TOGGLE_PERIOD_MS = 500UL;   // haut/bas toutes les 500 ms
    constexpr int  SERVO_ANGLE_LOW  = 0;
    constexpr int  SERVO_ANGLE_HIGH = 180;

    // Boucle principale
    constexpr unsigned long LOOP_PERIOD_MS       = 30;
    constexpr unsigned long SERIAL_BAUD          = 115200;
    constexpr unsigned long PRINT_PERIOD_MS      = 500;
}
