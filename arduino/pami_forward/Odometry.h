// Odometry.h - Odométrie différentielle à partir de 2 encodeurs
//
//  Basé sur 01_Test_PAMI_firmware (ECN Nantrobot)
#pragma once

#include <Arduino.h>
#include "Encoder.h"

class Odometry {
public:
    Odometry(Encoder& enc_left, Encoder& enc_right,
             float wheel_diameter_m,
             float wheel_base_m,
             int   pulses_per_revolution);

    void init();                            // capture des comptes initiaux
    void update();                          // à appeler régulièrement
    void resetPose(float x = 0, float y = 0, float theta = 0);

    float getX() const     { return _x;     }
    float getY() const     { return _y;     }
    float getTheta() const { return _theta; }
    float getThetaDeg() const { return _theta * 180.0f / PI; }

    // Distance totale parcourue (centre du robot, signée)
    float getDistanceM() const { return _distance_total; }

private:
    Encoder& _left;
    Encoder& _right;
    float _wheel_diameter;
    float _wheel_base;
    float _pulses_per_meter;

    long _last_left_count;
    long _last_right_count;

    float _x;
    float _y;
    float _theta;
    float _distance_total;
};
