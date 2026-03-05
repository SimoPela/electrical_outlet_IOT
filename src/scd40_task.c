#include "scd40_task.h"

#include <string.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "../components/scd4x_i2c.h"
#include "../components/sensirion_i2c_hal.h"

#include "esp_task_wdt.h"

static const char *TAG = "SCD40";

QueueHandle_t g_scd40_queue = NULL;

/* ---- Recovery policy (commercial tuning) ---- */
#define SCD40_FIRST_MEAS_WAIT_MS       5000
#define SCD40_POLL_MS                  200
#define SCD40_DATA_TIMEOUT_MS          12000
#define SCD40_MAX_CONSEC_ERRORS        5
#define SCD40_MAX_INVALID_SAMPLES      5
#define SCD40_RECOVERY_BACKOFF_MS      1000

/* After a successful read, sleep a bit so we don't hammer I2C/logs */
#define SCD40_OK_SAMPLE_DELAY_MS       400

static void msleep(uint32_t ms)
{
    const uint32_t step = 200;
    while (ms > 0) {
        uint32_t chunk = (ms > step) ? step : ms;
        vTaskDelay(pdMS_TO_TICKS(chunk));
        esp_task_wdt_reset();
        ms -= chunk;
    }
}

static int16_t scd40_start_measurement_clean(void)
{
    int16_t err = 0;

    (void)scd4x_wake_up();
    (void)scd4x_stop_periodic_measurement();
    (void)scd4x_reinit();

    err = scd4x_start_periodic_measurement();
    if (err) return err;

    msleep(SCD40_FIRST_MEAS_WAIT_MS);
    return 0;
}

static void scd40_recover(const scd40_cfg_t *cfg, int attempt, int16_t last_err)
{
    ESP_LOGW(TAG, "Recovery start (attempt=%d, last_err=%d)", attempt, (int)last_err);

    (void)scd4x_stop_periodic_measurement();

    (void)sensirion_i2c_hal_free();
    msleep(50);

    int16_t err = sensirion_i2c_hal_init(cfg->sda_gpio, cfg->scl_gpio);
    if (err) {
        ESP_LOGE(TAG, "sensirion_i2c_hal_init failed: %d", (int)err);
    }

    uint32_t backoff = SCD40_RECOVERY_BACKOFF_MS * (uint32_t)(attempt < 5 ? attempt : 5);
    msleep(backoff);

    err = scd40_start_measurement_clean();
    if (err) {
        ESP_LOGE(TAG, "start_periodic_measurement failed after recovery: %d", (int)err);
    } else {
        ESP_LOGI(TAG, "Recovery done");
    }
}

void scd40_task(void *pv)
{
    esp_task_wdt_reset();

    const scd40_cfg_t cfg = *(const scd40_cfg_t *)pv;
    uint32_t seq = 0;

    int16_t err = sensirion_i2c_hal_init(cfg.sda_gpio, cfg.scl_gpio);
    if (err) {
        ESP_LOGE(TAG, "sensirion_i2c_hal_init failed: %d", (int)err);
        // continue; recovery will handle
    }

    err = scd40_start_measurement_clean();
    if (err) {
        ESP_LOGE(TAG, "Initial start_measurement failed: %d", (int)err);
    }

    int consec_errors = 0;
    int invalid_samples = 0;
    int recovery_attempt = 0;
    int64_t last_ok_us = esp_timer_get_time();

    while (1) {
        esp_task_wdt_reset();

        // Timeout guard: if no OK sample for too long => recovery
        int64_t now_us = esp_timer_get_time();
        int64_t since_ok_ms = (now_us - last_ok_us) / 1000;
        if (since_ok_ms > SCD40_DATA_TIMEOUT_MS) {
            recovery_attempt++;
            scd40_recover(&cfg, recovery_attempt, err);
            consec_errors = 0;
            invalid_samples = 0;
            last_ok_us = esp_timer_get_time();
            continue;
        }

        // Poll data-ready
        bool ready = false;
        err = scd4x_get_data_ready_flag(&ready);
        if (err) {
            consec_errors++;
            ESP_LOGW(TAG, "get_data_ready_flag err=%d (consec=%d)", (int)err, consec_errors);
            msleep(200);

            if (consec_errors >= SCD40_MAX_CONSEC_ERRORS) {
                recovery_attempt++;
                scd40_recover(&cfg, recovery_attempt, err);
                consec_errors = 0;
                invalid_samples = 0;
            }
            continue;
        }

        if (!ready) {
            msleep(SCD40_POLL_MS);
            continue;
        }

        // Read measurement
        uint16_t co2 = 0;
        int32_t t_mC = 0;
        int32_t rh_m = 0;

        err = scd4x_read_measurement(&co2, &t_mC, &rh_m);
        if (err) {
            consec_errors++;
            ESP_LOGW(TAG, "read_measurement err=%d (consec=%d)", (int)err, consec_errors);

            if (consec_errors >= SCD40_MAX_CONSEC_ERRORS) {
                recovery_attempt++;
                scd40_recover(&cfg, recovery_attempt, err);
                consec_errors = 0;
                invalid_samples = 0;
            }
            msleep(200);
            continue;
        }

        if (co2 == 0) {
            invalid_samples++;
            ESP_LOGW(TAG, "Invalid sample (co2=0) invalid=%d", invalid_samples);

            if (invalid_samples >= SCD40_MAX_INVALID_SAMPLES) {
                recovery_attempt++;
                scd40_recover(&cfg, recovery_attempt, 0);
                consec_errors = 0;
                invalid_samples = 0;
            }
            msleep(200);
            continue;
        }

        // OK sample
        consec_errors = 0;
        invalid_samples = 0;
        recovery_attempt = 0;
        last_ok_us = esp_timer_get_time();

        scd40_sample_t s = {
            .co2_ppm = co2,
            .temp_mC = t_mC,
            .rh_mpermil = rh_m,
            .seq = ++seq,
            .last_error = 0,
            .valid = true,
        };

        // Publish "always latest"
        if (g_scd40_queue) {
            (void)xQueueOverwrite(g_scd40_queue, &s);
        }

        ESP_LOGI(TAG, "CO2=%u ppm | T=%.2f C | RH=%.2f %% (seq=%lu)",
                 (unsigned)co2, t_mC/1000.0, rh_m/1000.0, (unsigned long)seq);

        msleep(SCD40_OK_SAMPLE_DELAY_MS);
    }
}
