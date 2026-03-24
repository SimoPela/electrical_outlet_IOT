/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#include "acquisition_task.h"
#include "task_config.h"
#include "device_state.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"

// sensor headers
#include "mics5524.h"
#include "as312.h"
#include "scd40.h"
#include "sht41.h"
#include "sgp41.h"
#include "bmp.h"
#include "as7341_w.h"
#include "pms7003_w.h"

#include <math.h>
#include <inttypes.h>

static const char *TAG = "[ACQUISITION]";

/** @copydoc acquisition_task */
void acquisition_task(void *pvParameters)
{
    (void)pvParameters;

    // Counter used to log the stack usage periodically
    uint32_t counter = 0;

    // Counter used to print that the task is alive every 2000 ms
    uint32_t alive_counter = 0;

    ESP_LOGI(TAG, "Acquisition task started");

    // Get the reference wake time for vTaskDelayUntil()
    TickType_t xLastWakeTime = xTaskGetTickCount();
    TickType_t now = xLastWakeTime;

    // Local persistent state: keeps the last valid values
    // initialize all fields to 0
    acquisition_local_state_t local_state = {0};

    // Force first read
    TickType_t last_as312    = now - pdMS_TO_TICKS(AS312_INTERVAL_MS);
    TickType_t last_mics5524 = now - pdMS_TO_TICKS(MICS5524_INTERVAL_MS);
    TickType_t last_sgp41    = now - pdMS_TO_TICKS(SGP41_INTERVAL_MS);
    TickType_t last_sht41    = now - pdMS_TO_TICKS(SHT41_INTERVAL_MS);
    TickType_t last_bmp280   = now - pdMS_TO_TICKS(BMP280_INTERVAL_MS);
    TickType_t last_as7341   = now - pdMS_TO_TICKS(AS7341_INTERVAL_MS);
    TickType_t last_pms7003  = now - pdMS_TO_TICKS(PMS7003_INTERVAL_MS);
    TickType_t last_scd40    = now - pdMS_TO_TICKS(SCD40_INTERVAL_MS);

    for (;;)
    {
        logTaskAlive(TAG, &alive_counter, 20);

        // Update current time
        now = xTaskGetTickCount();

        // AS312 - motion
        if ((now - last_as312) >= pdMS_TO_TICKS(AS312_INTERVAL_MS))
        {
            last_as312 = now;

            local_state.motion_detected = as312_read_motion();
            local_state.as312_last_update = now;
            local_state.as312_valid = true;
            local_state.as312_fault = false;
        }

        // MiCS-5524 - gas
        // The averaging of samples and ADC calibration are handled
        // internally by mics5524_read_voltage().
        if ((now - last_mics5524) >= pdMS_TO_TICKS(MICS5524_INTERVAL_MS))
        {
            last_mics5524 = now;

            float voltage = mics5524_read_voltage();
            float ppm     = mics5524_read_ppm();

            if (voltage >= 0.0f && ppm >= 0.0f) {
                local_state.gas_level_raw    = voltage; /* [V] */
                local_state.gas_ppm          = ppm;     /* estimated CO ppm */
                local_state.mics5524_last_update  = now;
                local_state.mics5524_valid        = true;
                local_state.mics5524_fault        = false;

                // Debug log
                ESP_LOGD(TAG_DEBUG, "MiCS-5524: voltage=%.2f V, ppm=%.2f ppm", voltage, ppm);
            } else {
                local_state.mics5524_valid  = false;
                local_state.mics5524_fault  = true;
            }
        }

        // SHT41 - temperature and humidity
        if ((now - last_sht41) >= pdMS_TO_TICKS(SHT41_INTERVAL_MS))
        {
            last_sht41 = now;

            sht41_data_t sht = {0};
            if (sht41_read(&sht) == ESP_OK) {
                local_state.temperature_c      = sht.temperature_c;
                local_state.humidity_percent    = sht.humidity_percent;
                local_state.sht41_last_update   = now;
                local_state.sht41_valid         = true;
                local_state.sht41_fault         = false;
                // Debug log
                ESP_LOGD(TAG_DEBUG, "SHT41: temperature=%.1f°C, humidity=%.1f%%", local_state.temperature_c, local_state.humidity_percent);
            } else {
                local_state.sht41_valid = false;
                local_state.sht41_fault = true;
            }
        }

        // SGP41 - VOC / NOx (raw SRAW signals, pass SHT41 compensation)
        if ((now - last_sgp41) >= pdMS_TO_TICKS(SGP41_INTERVAL_MS))
        {
            last_sgp41 = now;

            float comp_t  = local_state.sht41_valid ? local_state.temperature_c   : NAN;
            float comp_rh = local_state.sht41_valid ? local_state.humidity_percent : NAN;

            sgp41_data_t sgp = {0};
            if (sgp41_read(&sgp, comp_t, comp_rh) == ESP_OK) {
                local_state.voc_index          = (float)sgp.voc_index;
                local_state.nox_index          = (float)sgp.nox_index;
                local_state.sgp41_last_update   = now;
                local_state.sgp41_valid         = true;
                local_state.sgp41_fault         = false;
                ESP_LOGD(TAG_DEBUG, "SGP41: VOC=%"PRId32"  NOx=%"PRId32"  (sraw %u / %u)",
                         sgp.voc_index, sgp.nox_index, sgp.sraw_voc, sgp.sraw_nox);
            } else {
                local_state.sgp41_valid = false;
                local_state.sgp41_fault = true;
            }
        }

        // BMP280 - pressure
        if ((now - last_bmp280) >= pdMS_TO_TICKS(BMP280_INTERVAL_MS))
        {
            last_bmp280 = now;

            bmp_data_t bmp = {0};
            if (bmp_read(&bmp) == ESP_OK) {
                local_state.bmp280_pressure_hpa = bmp.pressure_hpa;
                local_state.bmp280_temperature_c = bmp.temperature_c;
                local_state.bmp280_last_update = now;
                local_state.bmp280_valid       = true;
                local_state.bmp280_fault       = false;
                // Debug log
                ESP_LOGD(TAG_DEBUG, "BMP280: P=%.2f hPa  T=%.1f°C", bmp.pressure_hpa, bmp.temperature_c);
            } else {
                local_state.bmp280_valid = false;
                local_state.bmp280_fault = true;
            }
        }

        // AS7341 - light spectrum (F1-F8 basic counts)
        if ((now - last_as7341) >= pdMS_TO_TICKS(AS7341_INTERVAL_MS))
        {
            last_as7341 = now;

            as7341_data_t as = {0};
            if (as7341_w_read(&as) == ESP_OK) {
                local_state.light              = as;
                local_state.as7341_last_update = now;
                local_state.as7341_valid       = true;
                local_state.as7341_fault       = false;
                // Debug log
                ESP_LOGD(TAG_DEBUG, "AS7341: light=%.0f, %.0f, %.0f, %.0f, %.0f, %.0f, %.0f, %.0f",
                         as.channels[0], as.channels[1], as.channels[2], as.channels[3],
                         as.channels[4], as.channels[5], as.channels[6], as.channels[7]);
            } else {
                local_state.as7341_valid = false;
                local_state.as7341_fault = true;
            }
        }

        // SCD40 - CO2
        if ((now - last_scd40) >= pdMS_TO_TICKS(SCD40_INTERVAL_MS))
        {
            last_scd40 = now;

            scd40_data_t scd = {0};
            esp_err_t err = scd40_read(&scd);

            if (err == ESP_OK)
            {
                local_state.co2_ppm           = scd.co2_ppm;
                local_state.temperature_scd40 = scd.temperature_c;
                local_state.humidity_scd40    = scd.humidity_percent;
                local_state.scd40_last_update = now;
                local_state.scd40_valid       = true;
                local_state.scd40_fault       = false;

                // Debug log
                ESP_LOGD(TAG_DEBUG,
                        "SCD40: co2=%.0f ppm  T=%.1f°C  RH=%.1f%%",
                        scd.co2_ppm,
                        scd.temperature_c,
                        scd.humidity_percent);
            }
            else if (err == ESP_ERR_NOT_FINISHED)
            {
                ESP_LOGD(TAG_DEBUG, "SCD40: no new sample yet");
            }
            else
            {
                ESP_LOGE(TAG_DEBUG, "SCD40 read error: %s", esp_err_to_name(err));
                local_state.scd40_valid = false;
                local_state.scd40_fault = true;
            }
        }

        // PMS7003 - particulate matter
        if ((now - last_pms7003) >= pdMS_TO_TICKS(PMS7003_INTERVAL_MS))
        {
            last_pms7003 = now;

            pms7003_data_t pm = {0};
            esp_err_t err = pms7003_w_read(&pm);

            if (err == ESP_OK)
            {
                local_state.pm1_0_ug_m3        = pm.pm1_0_ug_m3;
                local_state.pm2_5_ug_m3        = pm.pm2_5_ug_m3;
                local_state.pm10_ug_m3         = pm.pm10_ug_m3;
                local_state.pms7003_last_update = now;
                local_state.pms7003_valid       = true;
                local_state.pms7003_fault       = false;

                ESP_LOGD(TAG_DEBUG, "PMS7003: PM1.0=%.0f  PM2.5=%.0f  PM10=%.0f µg/m³",
                         pm.pm1_0_ug_m3, pm.pm2_5_ug_m3, pm.pm10_ug_m3);
            }
            else if (err == ESP_ERR_NOT_FINISHED)
            {
                ESP_LOGD(TAG_DEBUG, "PMS7003: stabilizing or no frame yet");
            }
            else
            {
                ESP_LOGE(TAG_DEBUG, "PMS7003 read error: %s", esp_err_to_name(err));
                local_state.pms7003_valid = false;
                local_state.pms7003_fault = true;
            }
        }

        // Copy the complete local state into the shared device state
        if (xSemaphoreTake(g_device_state_mutex, portMAX_DELAY) == pdTRUE)
        {
            g_device_state.motion_detected = local_state.motion_detected;
            g_device_state.gas_level_raw = local_state.gas_level_raw;
            g_device_state.gas_ppm = local_state.gas_ppm;
            g_device_state.voc_index = local_state.voc_index;
            g_device_state.nox_index = local_state.nox_index;
            g_device_state.temperature_c = local_state.temperature_c;
            g_device_state.humidity_percent = local_state.humidity_percent;
            g_device_state.bmp280_pressure_hpa = local_state.bmp280_pressure_hpa;
            g_device_state.bmp280_temperature_c = local_state.bmp280_temperature_c;
            g_device_state.co2_ppm = local_state.co2_ppm;
            g_device_state.temperature_scd40 = local_state.temperature_scd40;
            g_device_state.humidity_scd40 = local_state.humidity_scd40;
            g_device_state.pm1_0_ug_m3 = local_state.pm1_0_ug_m3;
            g_device_state.pm2_5_ug_m3 = local_state.pm2_5_ug_m3;
            g_device_state.pm10_ug_m3 = local_state.pm10_ug_m3;
            g_device_state.light = local_state.light;

            g_device_state.as312_last_update = local_state.as312_last_update;
            g_device_state.mics5524_last_update = local_state.mics5524_last_update;
            g_device_state.sgp41_last_update = local_state.sgp41_last_update;
            g_device_state.sht41_last_update = local_state.sht41_last_update;
            g_device_state.bmp280_last_update = local_state.bmp280_last_update;
            g_device_state.scd40_last_update = local_state.scd40_last_update;
            g_device_state.pms7003_last_update = local_state.pms7003_last_update;
            g_device_state.as7341_last_update = local_state.as7341_last_update;

            g_device_state.as312_valid = local_state.as312_valid;
            g_device_state.mics5524_valid = local_state.mics5524_valid;
            g_device_state.sgp41_valid = local_state.sgp41_valid;
            g_device_state.sht41_valid = local_state.sht41_valid;
            g_device_state.bmp280_valid = local_state.bmp280_valid;
            g_device_state.scd40_valid = local_state.scd40_valid;
            g_device_state.pms7003_valid = local_state.pms7003_valid;
            g_device_state.as7341_valid = local_state.as7341_valid;

            g_device_state.as312_fault = local_state.as312_fault;
            g_device_state.mics5524_fault = local_state.mics5524_fault;
            g_device_state.sgp41_fault = local_state.sgp41_fault;
            g_device_state.sht41_fault = local_state.sht41_fault;
            g_device_state.bmp280_fault = local_state.bmp280_fault;
            g_device_state.scd40_fault = local_state.scd40_fault;
            g_device_state.pms7003_fault = local_state.pms7003_fault;
            g_device_state.as7341_fault = local_state.as7341_fault;

            xSemaphoreGive(g_device_state_mutex);
        }

        // Log the stack usage periodically
        logTaskStackUsage(&counter, 50, TAG, STACK_ACQUISITION_WORDS);

        // Wait for 100 ms
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(ACQUISITION_TASK_INTERVAL_MS));
    }
}
