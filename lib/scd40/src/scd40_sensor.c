#include "scd40_sensor.h"

#include "scd4x_i2c.h"
#include "sensirion_i2c_hal.h"

bool scd40_sensor_init(int sda_pin, int scl_pin)
{
    if (sensirion_i2c_hal_init(sda_pin, scl_pin) != 0) {
        return false;
    }

    /* Optional: stop any previous measurement already running */
    (void)scd4x_stop_periodic_measurement();

    return true;
}

bool scd40_sensor_start_periodic_measurement(void)
{
    return (scd4x_start_periodic_measurement() == 0);
}

bool scd40_sensor_data_ready(bool *ready)
{
    int16_t ret;

    if (ready == NULL) {
        return false;
    }

    ret = scd4x_get_data_ready_flag(ready);
    if (ret != 0) {
        *ready = false;
        return false;
    }

    return true;
}

bool scd40_sensor_read_sample(scd40_sample_t *sample)
{
    uint16_t co2 = 0;
    int32_t temperature_m_deg_c = 0;
    int32_t humidity_m_percent_rh = 0;
    int16_t ret;

    if (sample == NULL) {
        return false;
    }

    ret = scd4x_read_measurement(&co2,
                                 &temperature_m_deg_c,
                                 &humidity_m_percent_rh);
    if (ret != 0) {
        sample->valid = false;
        return false;
    }

    sample->co2_ppm = co2;
    sample->temperature_c = (float)temperature_m_deg_c / 1000.0f;
    sample->humidity_percent = (float)humidity_m_percent_rh / 1000.0f;
    sample->valid = true;

    return true;
}