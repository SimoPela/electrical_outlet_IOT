/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

 #include <pms7003.h>

 #include <string.h>
 #include <driver/gpio.h>
 #include <driver/uart.h>
 #include <freertos/FreeRTOS.h>
 #include <freertos/timers.h>
 #include <freertos/semphr.h>
 #include <esp_log.h>
 #include <esp_err.h>
 #include <esp_rom_sys.h>
 #include <esp_check.h>
 
 // Commands length
 #define PMS_CMD_LEN             0x07
 #define PMS_RESPONSE_LEN        0x08
 
 // Commands
 static const uint8_t pms_cmd_mode_passive[] = {0x42, 0x4d, 0xe1, 0x00, 0x00, 0x01, 0x70};
 static const uint8_t pms_cmd_mode_active[]  = {0x42, 0x4d, 0xe1, 0x00, 0x01, 0x01, 0x71};
 static const uint8_t pms_cmd_state_sleep[]  = {0x42, 0x4d, 0xe4, 0x00, 0x00, 0x01, 0x73};
 static const uint8_t pms_cmd_state_active[] = {0x42, 0x4d, 0xe4, 0x00, 0x01, 0x01, 0x74};
 //static const uint8_t pms_cmd_read_passive[] = {0x42, 0x4d, 0xe2, 0x00, 0x00, 0x01, 0x71};
 
 // Data frame length
 #define PMS_3003_FRAME_LEN      24
 #define PMS_X003_FRAME_LEN      32
 
 // Start bytes
 #define PMS_START_BYTE_HIGH     0x42
 #define PMS_START_BYTE_LOW      0x4d
 
 // Offsets
 #define PMS_PM1_CF1_HIGH_BYTE       0x04
 #define PMS_PM1_CF1_LOW_BYTE        0x05
 #define PMS_PM2_5_CF1_HIGH_BYTE     0x06
 #define PMS_PM2_5_CF1_LOW_BYTE      0x07
 #define PMS_PM10_CF1_HIGH_BYTE      0x08
 #define PMS_PM10_CF1_LOW_BYTE       0x09
 #define PMS_PM1_ATM_HIGH_BYTE       0x0A
 #define PMS_PM1_ATM_LOW_BYTE        0x0B
 #define PMS_PM2_5_ATM_HIGH_BYTE     0x0C
 #define PMS_PM2_5_ATM_LOW_BYTE      0x0D
 #define PMS_PM10_ATM_HIGH_BYTE      0x0E
 #define PMS_PM10_ATM_LOW_BYTE       0x0F
 #define PMS_0_3UM_HIGH_BYTE         0x10
 #define PMS_0_3UM_LOW_BYTE          0x11
 #define PMS_0_5UM_HIGH_BYTE         0x12
 #define PMS_0_5UM_LOW_BYTE          0x13
 #define PMS_1UM_HIGH_BYTE           0x14
 #define PMS_1UM_LOW_BYTE            0x15
 #define PMS_2_5UM_HIGH_BYTE         0x16
 #define PMS_2_5UM_LOW_BYTE          0x17
 #define PMS_5_0UM_HIGH_BYTE         0x18
 #define PMS_5_0UM_LOW_BYTE          0x19
 #define PMS_10UM_HIGH_BYTE          0x1A
 #define PMS_10UM_LOW_BYTE           0x1B
 
 #define PMS_TEMP_HIGH_BYTE          0x18
 #define PMS_TEMP_LOW_BYTE           0x19
 #define PMS_HUMID_HIGH_BYTE         0x1A
 #define PMS_HUMID_LOW_BYTE          0x1B
 
 // Timing
 // 2 ms reset pulse is sufficient per datasheet; kept as-is.
 #define PMS_RESET_TIME_MS      2000
 // PMS7003 datasheet requires >= 30 s warm-up after power-on.
 #define PMS_STABILIZE_TIME_MS  30000
 
 // Maximum number of fields that can be read from the largest frame.
 // X003 frame: 32 bytes total, 4 header bytes, 2 checksum bytes → 26 data
 // bytes → 13 uint16 fields.  3003 frame has fewer, but we size for X003.
 #define PMS_MAX_FIELD_INDEX     12   /* 0-based, so fields 0..12 */
 
 static const char *TAG = "PMS7003";
 
 /*
  * Single driver instance.
  * Thread-safety note: all public API functions acquire `lock` before
  * touching `pms_sensor`.  The timer callback runs in the FreeRTOS timer
  * task and therefore also takes the lock.
  */
 static pms_sensor_t   pms_sensor;
 static SemaphoreHandle_t pms_lock;
 
 /* ------------------------------------------------------------------ */
 /* Internal helpers                                                     */
 /* ------------------------------------------------------------------ */
 
 /** Acquire the driver mutex.  Returns ESP_ERR_TIMEOUT on failure. */
 static inline esp_err_t pms_lock_take(void)
 {
     if (!pms_lock) return ESP_ERR_INVALID_STATE;
     return (xSemaphoreTake(pms_lock, pdMS_TO_TICKS(500)) == pdTRUE)
            ? ESP_OK : ESP_ERR_TIMEOUT;
 }
 
 static inline void pms_lock_give(void)
 {
     if (pms_lock) xSemaphoreGive(pms_lock);
 }
 
 /* ------------------------------------------------------------------ */
 /* GPIO helpers                                                         */
 /* ------------------------------------------------------------------ */
 
 static esp_err_t pms_init_control_pin(gpio_num_t pin, uint8_t level)
 {
     if (pin < 0 || pin >= GPIO_NUM_MAX)
         return ESP_ERR_INVALID_ARG;
 
     gpio_config_t cfg = {
         .pin_bit_mask   = 1ULL << pin,
         .mode           = GPIO_MODE_OUTPUT,
         .pull_up_en     = GPIO_PULLUP_DISABLE,
         .pull_down_en   = GPIO_PULLDOWN_DISABLE,
         .intr_type      = GPIO_INTR_DISABLE,
     };
 
     ESP_RETURN_ON_ERROR(gpio_config(&cfg), TAG, "gpio_config failed for pin %d", pin);
     ESP_RETURN_ON_ERROR(gpio_set_level(pin, level), TAG, "gpio_set_level failed for pin %d", pin);
     return ESP_OK;
 }
 
 static inline esp_err_t pms_set_control_pin(gpio_num_t pin, uint32_t state)
 {
     if (pin < 0 || pin >= GPIO_NUM_MAX)
         return ESP_ERR_INVALID_ARG;
     return gpio_set_level(pin, state);
 }
 
 /* ------------------------------------------------------------------ */
 /* UART command                                                         */
 /* ------------------------------------------------------------------ */
 
 static esp_err_t pms_send_cmd(const uint8_t *cmd)
 {
     int written = uart_write_bytes(pms_sensor.uart_port,
                                    (const char *)cmd, PMS_CMD_LEN);
     if (written != PMS_CMD_LEN) {
         ESP_LOGE(TAG, "uart_write_bytes wrote %d / %d bytes", written, PMS_CMD_LEN);
         return ESP_FAIL;
     }
     return ESP_OK;
 }
 
 /* ------------------------------------------------------------------ */
 /* Timer callback  (FreeRTOS timer task context)                        */
 /* ------------------------------------------------------------------ */
 
 static void pms_timer_cb(TimerHandle_t xTimer)
 {
     /* Take the lock from the timer task. */
     if (xSemaphoreTake(pms_lock, pdMS_TO_TICKS(100)) != pdTRUE) {
         ESP_LOGE(TAG, "timer cb: failed to acquire lock");
         return;
     }
 
     switch (pms_sensor.state) {
 
         case PMS_STATE_RESETTING:
             ESP_LOGI(TAG, "reset -> stabilizing");
             pms_sensor.state = PMS_STATE_STABILIZING;
 
             if (xTimerChangePeriod(xTimer,
                                    pdMS_TO_TICKS(PMS_STABILIZE_TIME_MS),
                                    0) != pdPASS) {
                 ESP_LOGE(TAG, "timer change period failed");
             }
             break;
 
         case PMS_STATE_STABILIZING:
             pms_sensor.state = PMS_STATE_ACTIVE;
             ESP_LOGI(TAG, "sensor ready");
             break;
 
         default:
             ESP_LOGW(TAG, "timer fired in unexpected state %d", pms_sensor.state);
             break;
     }
 
     xSemaphoreGive(pms_lock);
 }
 
 /* ------------------------------------------------------------------ */
 /* Frame length & checksum                                              */
 /* ------------------------------------------------------------------ */
 
 static uint8_t pms_get_frame_len(void)
 {
     return (pms_sensor.type == PMS_TYPE_3003)
            ? PMS_3003_FRAME_LEN
            : PMS_X003_FRAME_LEN;
 }
 
 static esp_err_t pms_verify_checksum(const uint8_t *data, uint8_t len)
 {
     /* Minimum plausible frame: start(2) + length(2) + checksum(2) = 6 bytes. */
     if (len < 6) return ESP_ERR_INVALID_SIZE;
 
     uint16_t sum = 0;
     for (int i = 0; i < len - 2; i++) sum += data[i];
 
     uint16_t chk = ((uint16_t)data[len - 2] << 8) | data[len - 1];
     if (sum != chk) {
         ESP_LOGW(TAG, "checksum mismatch: computed 0x%04x, got 0x%04x", sum, chk);
         return ESP_ERR_INVALID_CRC;
     }
     return ESP_OK;
 }
 
 /* ------------------------------------------------------------------ */
 /* Public API                                                           */
 /* ------------------------------------------------------------------ */
 
 esp_err_t pms_reset(void)
 {
     ESP_RETURN_ON_ERROR(pms_lock_take(), TAG, "reset: lock failed");
 
     esp_err_t ret = ESP_OK;
 
     if (pms_sensor.state != PMS_STATE_SLEEP) {
         /* Release lock before calling pms_set_state which re-acquires it. */
         pms_lock_give();
         ESP_RETURN_ON_ERROR(pms_set_state(PMS_STATE_SLEEP), TAG, "reset: sleep failed");
         ESP_RETURN_ON_ERROR(pms_lock_take(), TAG, "reset: lock failed (2)");
     }
 
     /*
      * 10 µs minimum SET-pin deassert time per datasheet.
      * esp_rom_delay_us is safe here because the delay is too short for a
      * vTaskDelay (minimum 1 tick ≈ 1 ms) and we are not in an ISR.
      */
     esp_rom_delay_us(10);
 
     pms_lock_give();
     ESP_RETURN_ON_ERROR(pms_set_state(PMS_STATE_ACTIVE), TAG, "reset: active failed");
     return ret;
 }
 
 esp_err_t pms_set_mode(pms_mode_e mode)
 {
     if (mode != PMS_MODE_PASSIVE && mode != PMS_MODE_ACTIVE)
         return ESP_ERR_INVALID_ARG;
 
     ESP_RETURN_ON_ERROR(pms_lock_take(), TAG, "set_mode: lock failed");
 
     esp_err_t ret = ESP_OK;
 
     if (pms_sensor.state < PMS_STATE_STABILIZING) {
         ESP_LOGW(TAG, "set_mode called in state %d", pms_sensor.state);
         ret = ESP_ERR_INVALID_STATE;
         goto out;
     }
 
     if (pms_sensor.mode == mode)
         goto out;
 
     ret = pms_send_cmd((mode == PMS_MODE_PASSIVE)
                        ? pms_cmd_mode_passive
                        : pms_cmd_mode_active);
     if (ret == ESP_OK)
         pms_sensor.mode = mode;
 
 out:
     pms_lock_give();
     return ret;
 }
 
 esp_err_t pms_set_state(pms_state_e state)
 {
     ESP_RETURN_ON_ERROR(pms_lock_take(), TAG, "set_state: lock failed");
 
     esp_err_t ret = ESP_OK;
 
     if (pms_sensor.state == state)
         goto out;
 
     switch (state) {
 
         case PMS_STATE_SLEEP:
             if (pms_sensor.state == PMS_STATE_RESETTING) {
                 ESP_LOGW(TAG, "cannot sleep while resetting");
                 ret = ESP_ERR_INVALID_STATE;
                 goto out;
             }
 
             if (pms_sensor.set_gpio != GPIO_NUM_NC)
                 ret = pms_set_control_pin(pms_sensor.set_gpio, 0);
             else
                 ret = pms_send_cmd(pms_cmd_state_sleep);
 
             if (ret == ESP_OK)
                 pms_sensor.state = PMS_STATE_SLEEP;
             break;
 
         case PMS_STATE_ACTIVE:
             if (pms_sensor.state != PMS_STATE_SLEEP) {
                 ESP_LOGW(TAG, "can only activate from sleep (current state: %d)",
                          pms_sensor.state);
                 ret = ESP_ERR_INVALID_STATE;
                 goto out;
             }
 
             if (pms_sensor.set_gpio != GPIO_NUM_NC)
                 ret = pms_set_control_pin(pms_sensor.set_gpio, 1);
             else
                 ret = pms_send_cmd(pms_cmd_state_active);
 
             if (ret != ESP_OK) goto out;
 
             if (xTimerChangePeriod(pms_sensor.reset_timer,
                                    pdMS_TO_TICKS(PMS_RESET_TIME_MS),
                                    0) != pdPASS) {
                 ESP_LOGE(TAG, "xTimerChangePeriod failed");
                 ret = ESP_FAIL;
                 goto out;
             }
 
             /* After waking the sensor always comes up in active mode. */
             pms_sensor.state = PMS_STATE_RESETTING;
             pms_sensor.mode  = PMS_MODE_ACTIVE;
             break;
 
         default:
             ESP_LOGE(TAG, "set_state: unknown state %d", state);
             ret = ESP_ERR_INVALID_ARG;
             break;
     }
 
 out:
     pms_lock_give();
     return ret;
 }
 
 /* ------------------------------------------------------------------ */
 /* Getters (lock not strictly required for atomic reads on Xtensa,      */
 /* but taken for correctness on multi-core ESP32 targets)               */
 /* ------------------------------------------------------------------ */
 
 pms_mode_e pms_get_mode(void)
 {
     if (pms_lock_take() != ESP_OK) return PMS_MODE_ACTIVE; /* best-effort */
     pms_mode_e m = pms_sensor.mode;
     pms_lock_give();
     return m;
 }
 
 pms_state_e pms_get_state(void)
 {
     if (pms_lock_take() != ESP_OK) return PMS_STATE_SLEEP;
     pms_state_e s = pms_sensor.state;
     pms_lock_give();
     return s;
 }
 
 uart_port_t pms_get_uart_port(void)  { return pms_sensor.uart_port; }
 pms_type_e  pms_get_type(void)       { return pms_sensor.type; }
 
 /* ------------------------------------------------------------------ */
 /* Parse                                                                */
 /* ------------------------------------------------------------------ */
 
 esp_err_t pms_parse_data(const uint8_t *data, uint8_t len)
 {
     if (!data) return ESP_ERR_INVALID_ARG;
 
     if (data[0] != PMS_START_BYTE_HIGH || data[1] != PMS_START_BYTE_LOW) {
         ESP_LOGW(TAG, "bad start bytes: 0x%02x 0x%02x", data[0], data[1]);
         return ESP_ERR_INVALID_RESPONSE;
     }
 
     ESP_RETURN_ON_ERROR(pms_lock_take(), TAG, "parse_data: lock failed");
 
     esp_err_t ret = ESP_OK;
 
     uint8_t expected = pms_get_frame_len();
     if (len != expected) {
         ESP_LOGW(TAG, "frame length mismatch: got %u, expected %u", len, expected);
         ret = ESP_ERR_INVALID_SIZE;
         goto out;
     }
 
     ret = pms_verify_checksum(data, len);
     if (ret != ESP_OK) goto out;
 
     memcpy(pms_sensor.raw_data, data, len);
 
 out:
     pms_lock_give();
     return ret;
 }
 
 /* ------------------------------------------------------------------ */
 /* Data accessor                                                        */
 /* ------------------------------------------------------------------ */
 
 /**
  * @brief Read a parsed field value.
  *
  * @param field  Field index (0-based). Must be <= PMS_MAX_FIELD_INDEX.
  * @param out    Pointer to receive the uint16 value.
  * @return ESP_OK, ESP_ERR_INVALID_ARG, or ESP_ERR_INVALID_STATE.
  */
 esp_err_t pms_get_data(pms_field_t field, uint16_t *out)
 {
     if (!out) return ESP_ERR_INVALID_ARG;
 
     if ((int)field < 0 || (int)field > PMS_MAX_FIELD_INDEX) {
         ESP_LOGE(TAG, "pms_get_data: field %d out of range (max %d)",
                  (int)field, PMS_MAX_FIELD_INDEX);
         return ESP_ERR_INVALID_ARG;
     }
 
     ESP_RETURN_ON_ERROR(pms_lock_take(), TAG, "get_data: lock failed");
 
     /* Byte offset 4 = first data byte after start(2) + frame-length(2). */
     uint8_t hi = pms_sensor.raw_data[field * 2 + 4];
     uint8_t lo = pms_sensor.raw_data[field * 2 + 5];
     *out = ((uint16_t)hi << 8) | lo;
 
     pms_lock_give();
     return ESP_OK;
 }
 
 /* ------------------------------------------------------------------ */
 /* Init / Deinit                                                        */
 /* ------------------------------------------------------------------ */
 
 esp_err_t pms_init(pms_config_t *cfg)
 {
     if (!cfg) return ESP_ERR_INVALID_ARG;
 
     /* Create mutex before anything else touches pms_sensor. */
     if (!pms_lock) {
         pms_lock = xSemaphoreCreateMutex();
         if (!pms_lock) {
             ESP_LOGE(TAG, "failed to create mutex");
             return ESP_ERR_NO_MEM;
         }
     }
 
     /*
      * Start from a known SLEEP state so that pms_reset() can legally
      * call pms_set_state(PMS_STATE_ACTIVE) without hitting the
      * "can only activate from sleep" guard.
      */
     pms_sensor = (pms_sensor_t){
         .type       = cfg->type,
         .uart_port  = cfg->uart_port,
         .set_gpio   = cfg->set_gpio,
         .reset_gpio = cfg->reset_gpio,
         .mode       = PMS_MODE_ACTIVE,
         .state      = PMS_STATE_SLEEP,   /* <-- fixed: was PMS_STATE_ACTIVE */
     };
 
     if (cfg->set_gpio >= 0 && cfg->set_gpio < GPIO_NUM_MAX)
         ESP_RETURN_ON_ERROR(pms_init_control_pin(cfg->set_gpio, 1),
                             TAG, "set_gpio init failed");
 
     if (cfg->reset_gpio >= 0 && cfg->reset_gpio < GPIO_NUM_MAX)
         ESP_RETURN_ON_ERROR(pms_init_control_pin(cfg->reset_gpio, 1),
                             TAG, "reset_gpio init failed");
 
     pms_sensor.reset_timer = xTimerCreate("pms_reset",
                                           pdMS_TO_TICKS(PMS_RESET_TIME_MS),
                                           pdFALSE,   /* one-shot */
                                           NULL,
                                           pms_timer_cb);
     if (!pms_sensor.reset_timer) {
         ESP_LOGE(TAG, "xTimerCreate failed");
         return ESP_ERR_NO_MEM;
     }
 
     return pms_reset();
 }
 
 esp_err_t pms_deinit(void)
 {
     if (pms_sensor.reset_timer) {
         xTimerStop(pms_sensor.reset_timer, pdMS_TO_TICKS(100));
         xTimerDelete(pms_sensor.reset_timer, pdMS_TO_TICKS(100));
         pms_sensor.reset_timer = NULL;
     }
 
     if (pms_lock) {
         vSemaphoreDelete(pms_lock);
         pms_lock = NULL;
     }
 
     pms_sensor = (pms_sensor_t){0};
     return ESP_OK;
 }