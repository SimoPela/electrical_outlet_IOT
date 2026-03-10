#ifndef SCD40_SENSOR_H
#define SCD40_SENSOR_H

#pragma once

#include <cstdint>

struct Scd40Sample {
    uint16_t co2_ppm;
    float temperature_c;
    float humidity_percent;
    bool valid;
};

bool scd40_sensor_init(int sda_pin, int scl_pin);
bool scd40_sensor_start_periodic();
bool scd40_sensor_read(Scd40Sample* out_sample);
bool scd40_sensor_data_ready(bool* ready);

#endif // SCD40_SENSOR_H