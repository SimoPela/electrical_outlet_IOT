/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

 #include "comm_task.h"
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
 
     bool last_alarm_active = false;
     bool last_mqtt_connected = false;
 
     ESP_LOGI(TAG, "Communication task started");
 
     TickType_t xLastWakeTime = xTaskGetTickCount();
 
     for (;;)
     {
         logTaskAlive(TAG, &alive_counter, 5);
 
         device_state_t state_copy = {0};
 
         if (xSemaphoreTake(g_device_state_mutex, portMAX_DELAY) == pdTRUE)
         {
             state_copy = g_device_state;
             xSemaphoreGive(g_device_state_mutex);
         }
         else
         {
             ESP_LOGW(TAG, "Failed to lock device state mutex");
             logTaskStackUsage(&counter, 10, TAG, STACK_COMM_WORDS);
             vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(COMM_TASK_INTERVAL_MS));
             continue;
         }
 
         if (state_copy.mqtt_connected && g_mqtt_client != NULL)
         {
             if (!last_mqtt_connected)
             {
                 ESP_LOGI(TAG, "MQTT became connected");
 
                 if (mqtt_publish_availability(g_mqtt_client, APP_DEVICE_ID, true) < 0)
                 {
                     ESP_LOGW(TAG, "Failed to publish availability");
                 }
             }
 
             if (mqtt_publish_all_periodic(g_mqtt_client, APP_DEVICE_ID, &state_copy) < 0)
             {
                 ESP_LOGW(TAG, "One or more periodic publishes failed");
             }
 
             if (state_copy.alarm_active && !last_alarm_active)
             {
                 ESP_LOGW(TAG, "Alarm activated, publishing event");
 
                 if (mqtt_publish_alarm(g_mqtt_client, APP_DEVICE_ID, &state_copy) < 0)
                 {
                     ESP_LOGW(TAG, "Failed to publish alarm event");
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
 
         last_alarm_active = state_copy.alarm_active;
         last_mqtt_connected = state_copy.mqtt_connected;
 
         logTaskStackUsage(&counter, 10, TAG, STACK_COMM_WORDS);
         vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(COMM_TASK_INTERVAL_MS));
     }
 }