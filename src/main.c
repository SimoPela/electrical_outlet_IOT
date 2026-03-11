// standard headers
#include <stdint.h>
#include <stdbool.h>

// freertos headers
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// esp-idf headers
#include "esp_log.h"

// task headers
#include "acquisition_task.h"
#include "audio_task.h"
#include "system_task.h"
#include "comm_task.h"

static const char *TAG = "MAIN";

/* Stack sizes are in WORDS, not bytes (1 word = 4 bytes on ESP32) */
#define STACK_LED_WORDS        2048
#define STACK_SENSOR_WORDS     4096

#define LED_TASK_PRIORITY      1
#define SENSOR_TASK_PRIORITY   3

static void led_task(void *pvParameters)
{
    (void)pvParameters;
    ESP_LOGI("LED", "LED task started");

    while (true) {
        ESP_LOGI("LED", "Heartbeat");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void sensor_task(void *pvParameters)
{
    (void)pvParameters;
    ESP_LOGI("SENSOR", "Sensor task started");

    while (true) {
        ESP_LOGI("SENSOR", "Polling sensors...");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "System booting...");

    xTaskCreate(
        led_task,
        "led_task",
        STACK_LED_WORDS,
        NULL,
        LED_TASK_PRIORITY,
        NULL
    );

    xTaskCreate(
        sensor_task,
        "sensor_task",
        STACK_SENSOR_WORDS,
        NULL,
        SENSOR_TASK_PRIORITY,
        NULL
    );

    ESP_LOGI(TAG, "Tasks started");
}