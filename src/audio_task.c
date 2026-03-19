/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#include <audio_task.h>
#include <task_config.h>
#include <device_state.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "AUDIO";

void audio_task(void *pvParameters)
{
    (void)pvParameters;
    // Counter used to log the stack usage periodically (counter = 10, tick = 1000 ms -> log every 10 seconds)
    uint32_t counter = 0;
    // Counter used to print that the task is alive every 2000 ms
    uint32_t alive_counter = 0;

    ESP_LOGI(TAG, "Audio task started");
    // get the time of the last wakeup
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        // Print that the task is alive every 2000 ms
        alive_counter++;
        if (alive_counter >= 8)
        {
            ESP_LOGI(TAG, "Audio task alive");
            alive_counter = 0;
        }

        // Log the stack usage periodically. Once the stack size is tuned, this can be removed.
        logTaskStackUsage(&counter, TAG, STACK_AUDIO_WORDS);
        
        // wait for 500ms
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(AUDIO_TASK_INTERVAL_MS));
    }
}