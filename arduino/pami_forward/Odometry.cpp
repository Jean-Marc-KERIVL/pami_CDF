// Odometry.cpp
#include "Odometry.h"
#include <math.h>

Odometry::Odometry(Encoder& enc_left, Encoder& enc_right,
                   float wheel_diameter_m,
                   float wheel_base_m,
                   int   pulses_per_revolution)
    : _left(enc_left), _right(enc_right),
      _wheel_diameter(wheel_diameter_m),
      _wheel_base(wheel_base_m),
      _last_left_count(0), _last_right_count(0),
      _x(0), _y(0), _theta(0), _distance_total(0) {
    float circumference = PI * _wheel_diameter;
    _pulses_per_meter = pulses_per_revolution / circumference;
}

void Odometry::init() {
    _last_left_count  = _left.getCount();
    _last_right_count = _right.getCount();
}

void Odometry::update() {
    long c_left  = _left.getCount();
    long c_right = _right.getCount();

    long d_left  = c_left  - _last_left_count;
    long d_right = c_right - _last_right_count;

    float dist_left  = d_left  / _pulses_per_meter;
    float dist_right = d_right / _pulses_per_meter;

    float dist_center = (dist_left + dist_right) * 0.5f;
    float d_theta     = (dist_right - dist_left) / _wheel_base;

    if (fabsf(d_theta) < 0.001f) {
        _x += dist_center * cosf(_theta);
        _y += dist_center * sinf(_theta);
    } else {
        float radius = dist_center / d_theta;
        _x += radius * (sinf(_theta + d_theta) - sinf(_theta));
        _y += radius * (cosf(_theta) - cosf(_theta + d_theta));
    }

    _theta += d_theta;
    while (_theta >  PI) _theta -= 2.0f * PI;
    while (_theta < -PI) _theta += 2.0f * PI;

    _distance_total += dist_center;

    _last_left_count  = c_left;
    _last_right_count = c_right;
}

void Odometry::resetPose(float x, float y, float theta) {
    _x = x;
    _y = y;
    _theta = theta;
    _distance_total = 0;
    _last_left_count  = _left.getCount();
    _last_right_count = _right.getCount();
}
