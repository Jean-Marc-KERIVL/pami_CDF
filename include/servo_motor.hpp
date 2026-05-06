// Contrôle moteurs SERVO42D NEMA17 via UART

#ifndef SERVO_MOTOR_HPP
#define SERVO_MOTOR_HPP

#include <cstdint>
#include <string>

typedef struct {
    uint8_t motor_id;
    int32_t position;
    bool enabled;
    uint32_t speed;        // steps/s
    uint16_t torque;       // %
    uint8_t microstepping;
} ServoMotorState;

class ServoMotor {
public:
    ServoMotor(uint8_t id, int uart_num = 1);
    ~ServoMotor();

    // Mouvements
    void move_steps(int32_t steps);
    void go_to_position(int32_t target_pos);
    void home();

    // Contrôle
    void enable();
    void disable();
    void set_speed(uint32_t steps_per_second);
    void set_torque(uint16_t percent);
    void set_microstepping(uint8_t division);

    // État
    ServoMotorState get_status() const;
    int32_t get_position() const { return state_.position; }
    bool is_enabled() const { return state_.enabled; }

private:
    uint8_t motor_id_;
    int uart_num_;
    ServoMotorState state_;

    void send_command(const std::string& cmd);
};

// Classe pour gérer deux moteurs
class DualServoMotor {
public:
    DualServoMotor(int uart_num = 1);

    ServoMotor motor1, motor2;

    void enable_all();
    void disable_all();
    void move_both(int32_t steps1, int32_t steps2);
    void differential(int32_t left, int32_t right);
    void turn_in_place(int32_t steps, int direction);
};

#endif // SERVO_MOTOR_HPP
