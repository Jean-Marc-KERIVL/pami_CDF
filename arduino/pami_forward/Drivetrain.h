// Drivetrain.h - Paire de moteurs (gauche + droite) pour entraînement différentiel
#pragma once

#include "Motor.h"

class Drivetrain {
public:
    Drivetrain(Motor& left, Motor& right);

    void begin();

    // Avance les deux moteurs à la même vitesse signée
    void drive(int speed);

    // Stop
    void stop();

    Motor& left()  { return _left;  }
    Motor& right() { return _right; }

private:
    Motor& _left;
    Motor& _right;
};
