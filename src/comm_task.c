/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file comm_task.c
 * @brief FreeRTOS communication task implementation.
 *
 * Snapshots @c g_device_state every @c COMM_TASK_INTERVAL_MS milliseconds
 * and publishes MQTT messages for telemetry, system status, and alarm events.
 * Alarm publishes are edge-triggered (rising edge only).
 */

#include "comm_task.h"
#include "freertos/projdefs.h"
#include "task_config.h"
#include "device_state.h"
#include "mqtt_publish.h"
#include "mqtt_app.h"

#include "app_config.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

static const char *TAG = "[COMM]";

/** @copydoc comm_task */
void comm_task(void *pvParameters)
{
    (void)pvParameters;

    uint32_t counter = 0;
    uint32_t alive_counter = 0;

    /* Previous alarm / connection state for edge detection. */
    bool last_alarm_as312_active = false;
    bool last_alarm_mics5524_active = false;
    bool last_mqtt_connected = false;

    ESP_LOGI(TAG, "Communication task started");

    TickType_t xLastWakeTime = xTaskGetTickCount();
    TickType_t now = xLastWakeTime;

    /* Stagger first publishes to fire immediately. */
    TickType_t last_mqtt_environment = now - pdMS_TO_TICKS(MQTT_ENVIRONMENT_INTERVAL_MS);
    TickType_t last_mqtt_system = now - pdMS_TO_TICKS(MQTT_SYSTEM_INTERVAL_MS);

    for (;;)
    {
        logTaskAlive(TAG, &alive_counter, 5);

        now = xTaskGetTickCount();

        device_state_t state_copy = {0};

        if (xSemaphoreTake(g_device_state_mutex, portMAX_DELAY) == pdTRUE)
        {
            state_copy = g_device_state;
            xSemaphoreGive(g_device_state_mutex);
        }
        else
        {
            ESP_LOGW(TAG, "Failed to lock device state mutex");
            logTaskStackHeapUsage(&counter, LOG_CEILING_COMM, TAG, STACK_COMM_WORDS);
            vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(COMM_TASK_INTERVAL_MS));
            continue;
        }

        if (state_copy.mqtt_connected && g_mqtt_client != NULL)
        {
            if (!last_mqtt_connected)
            {
                ESP_LOGI(TAG, "MQTT became connected");

                if (mqtt_publish_system(g_mqtt_client, APP_DEVICE_ID, &state_copy) < 0)
                {
                    ESP_LOGW(TAG, "Failed to publish system status");
                }
            }

            if ((state_copy.as312_alarm && !last_alarm_as312_active) || (state_copy.mics5524_alarm && !last_alarm_mics5524_active))
            {
                ESP_LOGW(TAG, "Alarm activated as312, publishing event");

                if (mqtt_publish_alarm(g_mqtt_client, APP_DEVICE_ID, &state_copy) < 0)
                {
                    ESP_LOGW(TAG, "Failed to publish alarm event");
                }
            }

            if((now - last_mqtt_environment) >= pdMS_TO_TICKS(MQTT_ENVIRONMENT_INTERVAL_MS))
            {
                last_mqtt_environment = now;

                if (mqtt_publish_environment(g_mqtt_client, APP_DEVICE_ID, &state_copy) < 0)
                {
                    ESP_LOGW(TAG, "Failed to publish environment");
                }
            }

            if((now - last_mqtt_system) >= pdMS_TO_TICKS(MQTT_SYSTEM_INTERVAL_MS))
            {
                last_mqtt_system = now;

                if (mqtt_publish_system(g_mqtt_client, APP_DEVICE_ID, &state_copy) < 0)
                {
                    ESP_LOGW(TAG, "Failed to publish system status");
                }
            }
        }
        else
        {
            if (last_mqtt_connected)
            {
                ESP_LOGW(TAG, "MQTT connection lost");
            }
        }

        last_alarm_as312_active = state_copy.as312_alarm;
        last_alarm_mics5524_active = state_copy.mics5524_alarm;
        last_mqtt_connected = state_copy.mqtt_connected;

        logTaskStackHeapUsage(&counter, LOG_CEILING_COMM, TAG, STACK_COMM_WORDS);
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(COMM_TASK_INTERVAL_MS));
    }
}
