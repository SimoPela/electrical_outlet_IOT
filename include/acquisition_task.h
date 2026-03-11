#ifndef ACQUISITION_TASK_H
#define ACQUISITION_TASK_H

#include <stdbool.h>
#include "device_state.h"   // as7341_data_t

// define local state structure
typedef struct
{
    bool motion_detected;

    float gas_level_raw;

    float voc_index;
    float nox_index;

    float temperature_c;
    float humidity_percent;

    float pressure_hpa;

    float co2_ppm;
    float temperature_scd40;
    float humidity_scd40;

    float pm1_0_ug_m3;
    float pm2_5_ug_m3;
    float pm10_ug_m3;

    as7341_data_t light;
} acquisition_local_state_t;

void acquisition_task(void *pvParameters);

#endif