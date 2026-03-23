/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

 // test that lib work
#include "mqtt_app.h"
#include "network_app.h"

// sensor init headers
#include "sensor_init.h"
#include "esp_log.h"
#include "esp_check.h"

#include "gpio_init.h"
#include "adc_init.h"
#include "uart_init.h"
#include "i2s_init.h"

// standard headers
#include <stdint.h>
#include <stdbool.h>

// freertos headers
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// esp-idf headers
#include "esp_log.h"

// task config headers
#include "task_config.h"

// task headers
#include "acquisition_task.h"
#include "audio_task.h"
#include "system_task.h"
#include "comm_task.h"

// device state headers
#include "device_state.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "System booting...");

    // Initialize the device state (structure to store the current state of the device measurements and system flags)
    device_state_init();

    // Initialize the sensors
    ESP_LOGI(TAG, "Initializing sensors");
    if (sensor_init_all() != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize sensors");
        abort();
    }
    else
    {
        ESP_LOGI(TAG, "Sensors initialized successfully");
    }
    
    // Check if the device state mutex was created successfully
    ESP_LOGI(TAG, "Initializing device state mutex");
    if (g_device_state_mutex == NULL)
    {
        ESP_LOGE(TAG, "Failed to create device_state mutex");
        abort();
    }
    else
    {
        ESP_LOGI(TAG, "Device state mutex created successfully");
    }
    
    ESP_LOGI(TAG, "Initializing network");
    if (!network_app_init())
    {
        ESP_LOGE(TAG, "Network initialization failed");
        abort();
    }
    else
    {
        ESP_LOGI(TAG, "Network initialized successfully");
    }

    ESP_LOGI(TAG, "Waiting for Wi-Fi connection");
    if (!network_app_wait_until_connected(15000))
    {
        ESP_LOGE(TAG, "Wi-Fi not connected, MQTT will not start");
        abort();
    }
    else
    {
        ESP_LOGI(TAG, "Wi-Fi connected successfully");
    }

    ESP_LOGI(TAG, "Starting MQTT");
    mqtt_app_start();

    ESP_LOGI(TAG, "Creating tasks");

    // Create tasks and check the return value of xTaskCreate.
    // If there is not enough RAM, the function will fail and return a value different from pdPASS.
    
    // Acquisition task
    if (xTaskCreate(acquisition_task,
        "acquisition_task",
        STACK_ACQUISITION_WORDS,
        NULL,
        ACQUISITION_TASK_PRIORITY,
        NULL) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create acquisition_task");
        abort(); // abort the program if the task creation fails
    }
    else
    {
        ESP_LOGI(TAG, "Acquisition task created");
    }

    // Audio task
    if(xTaskCreate(audio_task, 
       "audio_task",
       STACK_AUDIO_WORDS,
       NULL,
       AUDIO_TASK_PRIORITY,
       NULL) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create audio_task");
        abort();
    }
    else
    {
        ESP_LOGI(TAG, "Audio task created");
    }

    // Comm task
    if(xTaskCreate(comm_task, 
       "comm_task",
       STACK_COMM_WORDS,
       NULL,
       COMM_TASK_PRIORITY,
       NULL) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create comm_task");
        abort();
    }
    else
    {
        ESP_LOGI(TAG, "Comm task created");
    }

    // System task
    if(xTaskCreate(system_task, 
       "system_task",
       STACK_SYSTEM_WORDS,
       NULL,
       SYSTEM_TASK_PRIORITY,
       NULL) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create system_task");
        abort();
    }
    else
    {
        ESP_LOGI(TAG, "System task created");
    }

    ESP_LOGI(TAG, "All tasks started");
}