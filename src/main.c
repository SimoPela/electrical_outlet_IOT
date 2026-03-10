#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_task_wdt.h"
#include "esp_system.h"

#include "driver/adc.h"

#include "led_RGB.h"
#include "scd40_task.h"
#include "wtd.h"

static const char *TAG = "MAIN";

/* Stack sizes are in WORDS, not bytes (1 word = 4 bytes on ESP32) */
#define STACK_LED_WORDS        2048   // ~8 KB
#define STACK_SENSOR_WORDS     4096   // ~16 KB

#define LED_TASK_PRIORITY      1
#define SENSOR_TASK_PRIORITY   3

void app_main(void)
{
    watchdog_init();
    ESP_LOGI(TAG, "System booting...");

    /* Create queues (length MUST be 1 for xQueueOverwrite pattern) */
    g_scd40_queue = xQueueCreate(1, sizeof(scd40_sample_t));
    configASSERT(g_scd40_queue);

    /* SCD40 configuration */
    static const scd40_cfg_t scd_cfg = {
        .sda_gpio = 21,
        .scl_gpio = 22,
    };

    /* LED configuration */
    static const led_rgb_cfg_t led_cfg = {
        .red_gpio = 15,
        .green_gpio = 2,
        .blue_gpio = 4,
        .active_high = 1,  // 1 = common cathode, 0 = common anode
    };

    TaskHandle_t scd40_handle = NULL;
    TaskHandle_t led_handle   = NULL;

    BaseType_t ret;

    /* ---- SCD40 task ---- */
#if CONFIG_FREERTOS_UNICORE
    ret = xTaskCreate(scd40_task, "scd40", STACK_SENSOR_WORDS,
                      (void *)&scd_cfg, SENSOR_TASK_PRIORITY, &scd40_handle);
#else
    ret = xTaskCreatePinnedToCore(scd40_task, "scd40", STACK_SENSOR_WORDS,
                                  (void *)&scd_cfg, SENSOR_TASK_PRIORITY,
                                  &scd40_handle, 1);
#endif
    configASSERT(ret == pdPASS);

    /* ---- LED task ---- */
#if CONFIG_FREERTOS_UNICORE
    ret = xTaskCreate(led_rgb_task, "led_rgb", STACK_LED_WORDS,
                      (void *)&led_cfg, LED_TASK_PRIORITY, &led_handle);
#else
    ret = xTaskCreatePinnedToCore(led_rgb_task, "led_rgb", STACK_LED_WORDS,
                                  (void *)&led_cfg, LED_TASK_PRIORITY,
                                  &led_handle, 1);
#endif
    configASSERT(ret == pdPASS);

    /* Register tasks to watchdog */
    ESP_ERROR_CHECK(esp_task_wdt_add(scd40_handle));
    ESP_ERROR_CHECK(esp_task_wdt_add(led_handle));

    ESP_LOGI(TAG, "Tasks started and registered to WDT");
}
