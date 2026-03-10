#include "scd40_sensor.h"

extern "C" {
#include "scd4x_i2c.h"
#include "sensirion_i2c_hal.h"
}

bool scd40_sensor_init(int sda_pin, int scl_pin) {
    if (sensirion_i2c_hal_init(sda_pin, scl_pin) != 0) {
        return false;
    }

    scd4x_stop_periodic_measurement();

    return true;
}

bool scd40_sensor_start_periodic_measurement(void) {
    return scd4x_start_periodic_measurement() == 0;
}

bool scd40_sensor_data_ready(bool* ready) {
    if (ready == nullptr) {
        return false;
    }

    uint16_t data_ready = 0;
    int16_t ret = scd4x_get_data_ready_flag(&data_ready);
    if (ret != 0) {
        *ready = false;
        return false;
    }

    *ready = (data_ready != 0);
    return true;
}

bool scd40_sensor_read_sample(Scd40Sample* sample) {
    if (sample == nullptr) {
        return false;
    }

    uint16_t co2 = 0;
    float temperature = 0.0f;
    float humidity = 0.0f;

    int16_t ret = scd4x_read_measurement(&co2, &temperature, &humidity);
    if (ret != 0) {
        sample->valid = false;
        return false;
    }

    sample->co2_ppm = co2;
    sample->temperature_c = temperature;
    sample->humidity_percent = humidity;
    sample->valid = true;

    return true;
}