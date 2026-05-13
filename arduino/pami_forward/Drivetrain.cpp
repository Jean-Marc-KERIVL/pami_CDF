// Drivetrain.cpp
#include "Drivetrain.h"

Drivetrain::Drivetrain(Motor& left, Motor& right)
    : _left(left), _right(right) {}

void Drivetrain::begin() {
    _left.begin();
    _right.begin();
}

void Drivetrain::drive(int speed) {
    _left.setSpeed(speed);
    _right.setSpeed(speed);
}

void Drivetrain::stop() {
    _left.stop();
    _right.stop();
}
