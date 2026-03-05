#include "led_RGB.h"

#include <stdbool.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "scd40_task.h"
#include "esp_task_wdt.h"

static const char *TAG = "LED_RGB";

/* ---- Tuning ---- */
#define LED_LOOP_PERIOD_MS            200
#define LED_BOOT_POLL_MS              800
#define LED_OFFLINE_TIMEOUT_MS        15000
#define LED_OFFLINE_BLINK_PERIOD_MS   800

#define CO2_GREEN_MAX_PPM             800
#define CO2_YELLOW_MAX_PPM            1200

/* ---------------------------------------------------------- */

static inline int lvl(const led_rgb_cfg_t *c, int on)
{
    return c->active_high ? (on ? 1 : 0) : (on ? 0 : 1);
}

static void led_apply(const led_rgb_cfg_t *c, int r, int g, int b)
{
    gpio_set_level(c->red_gpio,   lvl(c, r));
    gpio_set_level(c->green_gpio, lvl(c, g));
    gpio_set_level(c->blue_gpio,  lvl(c, b));
}

static void led_apply_if_changed(const led_rgb_cfg_t *c,
                                 int r, int g, int b,
                                 int *cr, int *cg, int *cb)
{
    if (r == *cr && g == *cg && b == *cb)
        return;

    *cr = r;
    *cg = g;
    *cb = b;
    led_apply(c, r, g, b);
}

static void co2_to_rgb(uint16_t co2, int *r, int *g, int *b)
{
    if (co2 < CO2_GREEN_MAX_PPM) {
        *r = 0; *g = 1; *b = 0;   // green
    } else if (co2 < CO2_YELLOW_MAX_PPM) {
        *r = 1; *g = 1; *b = 0;   // yellow
    } else {
        *r = 1; *g = 0; *b = 0;   // red
    }
}

static int64_t now_ms(void)
{
    return esp_timer_get_time() / 1000;
}

/* ========================================================== */
/* ======================= TASK ============================= */
/* ========================================================== */

void led_rgb_task(void *pv)
{
    const led_rgb_cfg_t cfg = *(const led_rgb_cfg_t *)pv;

    /* GPIO configuration */
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << cfg.red_gpio) |
                        (1ULL << cfg.green_gpio) |
                        (1ULL << cfg.blue_gpio),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&io));

    int cur_r = 0, cur_g = 0, cur_b = 0;
    int target_r = 0, target_g = 0, target_b = 1; // boot = blue

    led_apply_if_changed(&cfg, 0, 0, 1, &cur_r, &cur_g, &cur_b);

    ESP_LOGI(TAG, "Boot: waiting first valid SCD40 sample...");

    scd40_sample_t s = {0};
    bool has_sample = false;

    uint32_t last_seq = 0;
    int64_t last_sample_ms = 0;
    int64_t last_blink_toggle_ms = 0;
    bool blink_on = false;

    /* ---------------- BOOT WAIT ---------------- */
    while (!has_sample) {
        esp_task_wdt_reset();

        if (g_scd40_queue &&
            xQueuePeek(g_scd40_queue, &s, pdMS_TO_TICKS(LED_BOOT_POLL_MS)) == pdTRUE &&
            s.valid && s.co2_ppm != 0) {

            has_sample = true;
            last_seq = s.seq;
            last_sample_ms = now_ms();

            co2_to_rgb(s.co2_ppm, &target_r, &target_g, &target_b);
            led_apply_if_changed(&cfg, target_r, target_g, target_b,
                                 &cur_r, &cur_g, &cur_b);

            ESP_LOGI(TAG, "First sample: CO2=%u ppm", (unsigned)s.co2_ppm);
            break;
        }

        /* keep blue while waiting */
        led_apply_if_changed(&cfg, 0, 0, 1, &cur_r, &cur_g, &cur_b);
    }

    const TickType_t loop_wait = pdMS_TO_TICKS(LED_LOOP_PERIOD_MS);

    /* ---------------- NORMAL LOOP ---------------- */
    while (1) {
        esp_task_wdt_reset();

        int64_t t = now_ms();

        /* Check if a NEW sample arrived */
        if (g_scd40_queue &&
            xQueuePeek(g_scd40_queue, &s, 0) == pdTRUE &&
            s.valid && s.co2_ppm != 0) {

            if (s.seq != last_seq) {
                last_seq = s.seq;
                last_sample_ms = t;
                co2_to_rgb(s.co2_ppm, &target_r, &target_g, &target_b);
            }
        }

        bool offline = (t - last_sample_ms) > LED_OFFLINE_TIMEOUT_MS;

        if (offline) {
            /* Blink blue */
            if ((t - last_blink_toggle_ms) >= LED_OFFLINE_BLINK_PERIOD_MS) {
                last_blink_toggle_ms = t;
                blink_on = !blink_on;
                led_apply_if_changed(&cfg,
                                     0, 0, blink_on ? 1 : 0,
                                     &cur_r, &cur_g, &cur_b);
            }
        } else {
            /* Normal operation */
            led_apply_if_changed(&cfg,
                                 target_r, target_g, target_b,
                                 &cur_r, &cur_g, &cur_b);
        }

        vTaskDelay(loop_wait);
    }
}
