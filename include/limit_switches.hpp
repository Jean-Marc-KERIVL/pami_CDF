// Gestion des limit switches

#ifndef LIMIT_SWITCHES_HPP
#define LIMIT_SWITCHES_HPP

#include <cstdint>
#include <array>

typedef struct {
    uint8_t switch_id;
    uint32_t pin;
    bool is_triggered;
    uint32_t total_triggers;
} LimitSwitchStatus;

class LimitSwitch {
public:
    LimitSwitch(uint8_t id, uint32_t gpio_pin);

    void read();
    bool is_pressed() const { return is_triggered_; }
    void reset();

    LimitSwitchStatus get_status() const;

private:
    uint8_t switch_id_;
    uint32_t gpio_pin_;
    bool is_triggered_;
    uint32_t total_triggers_;
};

// Gérer 3 limit switches
class LimitSwitches {
public:
    LimitSwitches();

    void read_all();
    bool is_triggered(uint8_t switch_id);
    LimitSwitch* get(uint8_t switch_id);
    void reset_all();

    LimitSwitch switch1, switch2, switch3;

private:
    std::array<LimitSwitch*, 3> switches_;
};

#endif // LIMIT_SWITCHES_HPP
