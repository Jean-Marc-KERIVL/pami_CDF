// Contrôle pompe à air

#ifndef AIR_PUMP_HPP
#define AIR_PUMP_HPP

#include <cstdint>
#include <ctime>

typedef enum {
    PUMP_OFF = 0,
    PUMP_ON = 1
} PumpState;

typedef struct {
    PumpState state;
    uint32_t pin;
    uint32_t total_run_time_ms;
    uint32_t session_start_time;
} AirPumpStatus;

class AirPump {
public:
    explicit AirPump(uint32_t gpio_pin = 14);
    ~AirPump();

    // Contrôle basique
    void activate();
    void deactivate();
    void toggle();

    // Patterns
    void pulse(uint32_t duration_ms);
    void burst(uint8_t count, uint32_t interval_ms, uint32_t duration_ms);
    void activate_for(uint32_t duration_ms);

    // État
    AirPumpStatus get_status() const;
    PumpState get_state() const { return state_; }
    bool is_active() const { return state_ == PUMP_ON; }

private:
    uint32_t gpio_pin_;
    PumpState state_;
    uint32_t total_run_time_ms_;
    time_t session_start_time_;

    void set_gpio(uint8_t level);
};

#endif // AIR_PUMP_HPP
