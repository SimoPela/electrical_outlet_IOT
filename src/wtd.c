#include "esp_log.h"
#include "esp_task_wdt.h"
#include "esp_system.h"

#include "wtd.h"

static const char *TAG = "WTD";

void watchdog_init(void)
{
    esp_task_wdt_config_t cfg = {
        .timeout_ms = WDT_TIMEOUT_S * 1000,
        .idle_core_mask = (1 << 0) | (1 << 1), // controlla anche idle tasks su entrambi i core
        .trigger_panic = true,                 // panic+reset se scade
    };

    // Utilise reconfigure si déjà initialisé, sinon init
    esp_err_t err = esp_task_wdt_reconfigure(&cfg);
    if (err == ESP_ERR_INVALID_STATE) {
        // WDT pas encore initialisé, on l'initialise
        err = esp_task_wdt_init(&cfg);
    }
    ESP_ERROR_CHECK(err);

    ESP_LOGI(TAG, "Task WDT configured (%ds). Reset reason=%d",
             WDT_TIMEOUT_S, (int)esp_reset_reason());
}