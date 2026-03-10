#ifndef SCD40_SENSOR_H
#define SCD40_SENSOR_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint16_t co2_ppm;
    float temperature_c;
    float humidity_percent;
    bool valid;
} scd40_sample_t;

bool scd40_sensor_init(int sda_pin, int scl_pin);
bool scd40_sensor_start_periodic_measurement(void);
bool scd40_sensor_data_ready(bool *ready);
bool scd40_sensor_read_sample(scd40_sample_t *sample);

#endif /* SCD40_SENSOR_H */