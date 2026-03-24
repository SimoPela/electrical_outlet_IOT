/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#include <audio_task.h>
#include <task_config.h>
#include <device_state.h>

#include "inmp441_w.h"

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

static const char *TAG = "[AUDIO]";

/** @copydoc audio_task */
void audio_task(void *pvParameters)
{
    (void)pvParameters;
    // Counter used to log the stack usage periodically (counter = 10, tick = 1000 ms -> log every 10 seconds)
    uint32_t counter = 0;
    // Counter used to print that the task is alive every 2000 ms
    uint32_t alive_counter = 0;

    // initialize all fields to 0
    audio_local_state_t local_state = {0};

    ESP_LOGI(TAG, "Audio task started");
    // get the time of the last wakeup
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        logTaskAlive(TAG, &alive_counter, 8);

        if (g_sensor_driver_mutex != NULL) {
            xSemaphoreTake(g_sensor_driver_mutex, portMAX_DELAY);
        }

        inmp441_data_t mic = {0};
        esp_err_t mic_err = inmp441_w_read(&mic);

        if (g_sensor_driver_mutex != NULL) {
            xSemaphoreGive(g_sensor_driver_mutex);
        }

        if (mic_err == ESP_OK) {
            local_state.noise_db = mic.noise_db;
            ESP_LOGD(TAG, "INMP441: noise_db=%.1f dB SPL (DS + offset)", local_state.noise_db);
        } else {
            ESP_LOGW(TAG, "INMP441 read failed: %s", esp_err_to_name(mic_err));
        }

        if (xSemaphoreTake(g_device_state_mutex, portMAX_DELAY) == pdTRUE)
        {
            g_device_state.noise_db = local_state.noise_db;
            g_device_state.inmp441_last_update = xTaskGetTickCount();
            g_device_state.inmp441_valid = (mic_err == ESP_OK);
            g_device_state.inmp441_fault = (mic_err != ESP_OK);

            xSemaphoreGive(g_device_state_mutex);
        }

        // Log the stack usage periodically. Once the stack size is tuned, this can be removed.
        logTaskStackUsage(&counter, 10, TAG, STACK_AUDIO_WORDS);
        
        // wait for 500ms
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(AUDIO_TASK_INTERVAL_MS));
    }
}