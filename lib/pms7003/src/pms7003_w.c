/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 *
 * Wrapper over the PMS7003 low-level driver (pms7003.h).
 *
 * In active mode the sensor pushes 32-byte frames every ~2.3 s.
 * pms7003_w_read() drains the UART RX buffer and parses the most
 * recent valid frame.
 *
 * Recovery strategy (pms7003_w_restore):
 *   Level 1 — soft reset (sleep → wake).  Fast, non-destructive.
 *   Level 2 — full deinit + reinit.  Used if level 1 fails or the
 *              driver is in an unrecoverable state.
 *
 * g_degraded is set when level 1 fails and cleared only after a
 * successful level-2 recovery AND a subsequent successful read.
 */

 #include "pms7003_w.h"
 #include "pms7003.h"
 #include "esp32_pinout.h"
 
 #include <driver/uart.h>
 #include <esp_log.h>
 #include <esp_check.h>
 #include <freertos/FreeRTOS.h>
 #include <freertos/task.h>
 #include <esp_err.h>
 
 static const char *TAG = "PMS7003_W";
 
 #define PMS7003_UART_PORT       UART_NUM_2
 #define PMS7003_FRAME_LEN       32
 #define PMS7003_READ_TIMEOUT_MS 150
 
 /*
  * After a reset/reinit the sensor needs ~30 s to stabilize.
  * pms7003_w_restore() does NOT block for that time — it just kicks
  * off the process and returns.  The caller should retry pms7003_w_read()
  * until it stops returning ESP_ERR_NOT_FINISHED.
  *
  * Maximum consecutive restore attempts before giving up entirely.
  */
 #define PMS7003_MAX_RESTORE_ATTEMPTS 3
 
 static bool     g_initialized       = false;
 static bool     g_degraded          = false;  /* true after a failed level-1 reset */
 static uint8_t  g_restore_attempts  = 0;      /* lifetime counter, reset on success */
 
 /* ------------------------------------------------------------------ */
 /* Internal helpers                                                     */
 /* ------------------------------------------------------------------ */
 
 /**
  * @brief Flush the UART RX buffer before reading a fresh frame.
  *
  * After a reset the buffer may contain stale / partial frames left over
  * from before the reset.  Flushing ensures we only parse data produced
  * after recovery.
  */
 static void pms7003_flush_rx(void)
 {
     uart_flush_input(PMS7003_UART_PORT);
 }
 
 /**
  * @brief Level-1 recovery: soft reset (sleep → wake cycle).
  *
  * Uses the existing pms_reset() which drives the SET pin low then high
  * (or sends sleep/wake UART commands if SET pin is NC).
  *
  * @return ESP_OK if the reset was accepted by the driver.
  */
 static esp_err_t pms7003_reset_l1(void)
 {
     ESP_LOGW(TAG, "restore L1: soft reset");
     esp_err_t err = pms_reset();
     if (err != ESP_OK) {
         ESP_LOGE(TAG, "restore L1 failed: %s", esp_err_to_name(err));
         return err;
     }
     pms7003_flush_rx();
     ESP_LOGI(TAG, "restore L1: reset issued, sensor stabilizing");
     return ESP_OK;
 }
 
 /**
  * @brief Level-2 recovery: full deinit + reinit.
  *
  * Destroys all driver state (mutex, timer, GPIO) and rebuilds from
  * scratch.  More disruptive but more likely to clear stuck states.
  *
  * @return ESP_OK if reinit succeeded.
  */
 static esp_err_t pms7003_reset_l2(void)
 {
     ESP_LOGW(TAG, "restore L2: full deinit + reinit");
 
     pms_deinit();
     g_initialized = false;
 
     /* Small gap to let the sensor power-cycle its internal state. */
     vTaskDelay(pdMS_TO_TICKS(200));
 
     pms_config_t cfg = {
         .type       = PMS_TYPE_7003,
         .set_gpio   = PIN_PMS7003_SET,
         .reset_gpio = PIN_PMS7003_RESET,
         .uart_port  = PMS7003_UART_PORT,
     };
 
     esp_err_t err = pms_init(&cfg);
     if (err != ESP_OK) {
         ESP_LOGE(TAG, "restore L2 failed: %s", esp_err_to_name(err));
         return err;
     }
 
     g_initialized = true;
     pms7003_flush_rx();
     ESP_LOGI(TAG, "restore L2: reinit done, sensor stabilizing");
     return ESP_OK;
 }
 
 /* ------------------------------------------------------------------ */
 /* Public API                                                           */
 /* ------------------------------------------------------------------ */
 
 esp_err_t pms7003_w_init(void)
 {
     if (g_initialized)
         return ESP_OK;
 
     pms_config_t cfg = {
         .type       = PMS_TYPE_7003,
         .set_gpio   = PIN_PMS7003_SET,
         .reset_gpio = PIN_PMS7003_RESET,
         .uart_port  = PMS7003_UART_PORT,
     };
 
     esp_err_t err = pms_init(&cfg);
     if (err != ESP_OK) {
         ESP_LOGE(TAG, "pms_init failed: %s", esp_err_to_name(err));
         return err;
     }
 
     g_initialized       = true;
     g_degraded          = false;
     g_restore_attempts  = 0;
     ESP_LOGI(TAG, "PMS7003 initialized (stabilizing ~30 s)");
     return ESP_OK;
 }
 
 /**
  * @brief Multi-level recovery for sensor crashes.
  *
  * Strategy:
  *   1. If not yet degraded → try level-1 (soft reset).
  *      On success: set g_degraded = true (sensor is recovering,
  *      not yet confirmed healthy).
  *      On failure: escalate to level-2 immediately.
  *   2. If already degraded OR level-1 failed → try level-2 (reinit).
  *      On success: g_degraded stays true until a successful read
  *      confirms the sensor is back.
  *      On failure: increment attempt counter.
  *   3. After PMS7003_MAX_RESTORE_ATTEMPTS failures: return
  *      ESP_ERR_NOT_RECOVERABLE so the caller can escalate (e.g. alert,
  *      reboot the MCU, disable the sensor entirely).
  *
  * This function is non-blocking: it kicks off the recovery and returns.
  * The caller must keep calling pms7003_w_read() and check the result.
  */
 esp_err_t pms7003_w_restore(void)
 {
     if (g_restore_attempts >= PMS7003_MAX_RESTORE_ATTEMPTS) {
         ESP_LOGE(TAG, "restore: max attempts (%d) reached, sensor unrecoverable",
                  PMS7003_MAX_RESTORE_ATTEMPTS);
         return ESP_ERR_INVALID_STATE;
     }
 
     g_restore_attempts++;
     ESP_LOGW(TAG, "restore: attempt %d / %d",
              g_restore_attempts, PMS7003_MAX_RESTORE_ATTEMPTS);
 
     esp_err_t err;
 
     if (!g_degraded) {
         /* First failure: try a cheap soft reset. */
         err = pms7003_reset_l1();
         if (err == ESP_OK) {
             /*
              * Mark degraded so we know a subsequent read failure means
              * L1 was not enough and we need L2.
              */
             g_degraded = true;
             return ESP_OK;
         }
         /* L1 failed structurally (driver error) — go straight to L2. */
         ESP_LOGW(TAG, "restore L1 error, escalating to L2");
     }
 
     /* Either already degraded or L1 errored: do full reinit. */
     err = pms7003_reset_l2();
     if (err == ESP_OK) {
         /*
          * g_degraded stays true: it will be cleared only by a successful
          * read, confirming the sensor is actually producing valid data.
          */
         return ESP_OK;
     }
 
     ESP_LOGE(TAG, "restore L2 failed (%d/%d): %s",
              g_restore_attempts, PMS7003_MAX_RESTORE_ATTEMPTS,
              esp_err_to_name(err));
     return err;
 }
 
 /**
  * @brief Returns true if the sensor is in degraded state (recovering).
  *
  * Useful for the caller to decide whether to trust the readings or to
  * log/display a warning.
  */
 bool pms7003_w_is_degraded(void)
 {
     return g_degraded;
 }
 
 /**
  * @brief Read the latest PM data frame from the sensor.
  *
  * Drains the UART RX buffer and parses the most recent valid frame
  * (scanning backwards so we always get the freshest data).
  *
  * @param out  Output structure.  Untouched on error.
  * @return
  *   ESP_OK             — valid data in *out; if g_degraded was set it is
  *                        now cleared (sensor confirmed healthy).
  *   ESP_ERR_NOT_FINISHED — sensor stabilizing or no valid frame yet;
  *                        caller should retry later.
  *   ESP_ERR_INVALID_ARG  — out is NULL.
  *   ESP_ERR_INVALID_STATE — driver not initialized.
  */
 esp_err_t pms7003_w_read(pms7003_data_t *out)
 {
     ESP_RETURN_ON_FALSE(out != NULL,   ESP_ERR_INVALID_ARG,   TAG, "out is NULL");
     ESP_RETURN_ON_FALSE(g_initialized, ESP_ERR_INVALID_STATE, TAG, "not initialized");
 
     if (pms_get_state() != PMS_STATE_ACTIVE)
         return ESP_ERR_NOT_FINISHED;
 
     uint8_t buf[128];
     int len = uart_read_bytes(PMS7003_UART_PORT, buf, sizeof(buf),
                               pdMS_TO_TICKS(PMS7003_READ_TIMEOUT_MS));
     if (len < PMS7003_FRAME_LEN)
         return ESP_ERR_NOT_FINISHED;
 
     /*
      * Scan backwards: the most recent frame is at the tail of the buffer.
      * Stop at the first valid (checksum-passing) frame we find.
      */
     esp_err_t parse_err = ESP_FAIL;
     for (int i = len - PMS7003_FRAME_LEN; i >= 0; i--) {
         if (buf[i] == 0x42 && buf[i + 1] == 0x4D) {
             parse_err = pms_parse_data(&buf[i], PMS7003_FRAME_LEN);
             if (parse_err == ESP_OK)
                 break;
         }
     }
 
     if (parse_err != ESP_OK)
         return ESP_ERR_NOT_FINISHED;
 
     /* Extract fields — pms_get_data now returns esp_err_t. */
     uint16_t pm1 = 0, pm2_5 = 0, pm10 = 0;
     ESP_RETURN_ON_ERROR(pms_get_data(PMS_FIELD_PM1_ATM,   &pm1),  TAG, "get pm1 failed");
     ESP_RETURN_ON_ERROR(pms_get_data(PMS_FIELD_PM2_5_ATM, &pm2_5), TAG, "get pm2_5 failed");
     ESP_RETURN_ON_ERROR(pms_get_data(PMS_FIELD_PM10_ATM,  &pm10), TAG, "get pm10 failed");
 
     out->pm1_0_ug_m3 = (float)pm1;
     out->pm2_5_ug_m3 = (float)pm2_5;
     out->pm10_ug_m3  = (float)pm10;
 
     /*
      * Successful read: sensor is confirmed healthy.
      * Clear degraded flag and reset the attempt counter so future
      * failures start fresh from level-1.
      */
     if (g_degraded) {
         ESP_LOGI(TAG, "sensor recovered successfully");
         g_degraded         = false;
         g_restore_attempts = 0;
     }
 
     return ESP_OK;
 }