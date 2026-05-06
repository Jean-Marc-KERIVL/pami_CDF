// Gestion de l'alimentation et mesure batterie (INA219)

#ifndef POWER_MANAGEMENT_HPP
#define POWER_MANAGEMENT_HPP

#include <cstdint>

typedef struct {
    uint8_t battery_id;
    float voltage;      // V
    float current;      // mA
    float power;        // W
    bool is_connected;
} BatteryStatus;

typedef struct {
    BatteryStatus battery1;
    BatteryStatus battery2;
    float system_voltage;
    float system_current;
    float system_power;
    bool emergency_stop_active;
} SystemPowerStatus;

class Battery {
public:
    Battery(uint8_t id, uint8_t i2c_addr);

    void read_voltage();
    void read_current();
    void read_power();

    void check_low_voltage();
    BatteryStatus get_status() const;

    float get_voltage() const { return voltage_; }
    float get_current() const { return current_; }
    float get_power() const { return power_; }

private:
    uint8_t battery_id_;
    uint8_t i2c_addr_;
    float voltage_;
    float current_;
    float power_;
    bool is_connected_;
    float max_voltage_;
    float min_voltage_;
};

class PowerManagement {
public:
    PowerManagement();

    void read_all_voltages();
    void read_all_currents();
    bool check_system_health();

    SystemPowerStatus get_summary();
    void emergency_stop();
    void resume_operation();

    float estimate_runtime(float consumption_per_hour_wh);

    Battery battery1, battery2;

private:
    float voltage_12v_;
    float voltage_5v_;
    float voltage_3v3_;
    bool emergency_stop_active_;
};

#endif // POWER_MANAGEMENT_HPP
