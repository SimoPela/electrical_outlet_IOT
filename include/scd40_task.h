#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

typedef struct {
    uint16_t co2_ppm;
    int32_t  temp_mC;      // milli °C
    int32_t  rh_mpermil;   // milli %RH
    uint32_t seq;
    int16_t  last_error;   // 0 = ok
    bool     valid;
} scd40_sample_t;

extern QueueHandle_t g_scd40_queue;

typedef struct {
    int sda_gpio;
    int scl_gpio;
} scd40_cfg_t;

void scd40_task(void *pv);
