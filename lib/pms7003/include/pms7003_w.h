/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file pms7003_w.h
 * @brief Project wrapper around @c pms7003.h: init/read/restore API
 *        and atmospheric PM values.
 *
 * Recovery strategy implemented in pms7003_w_restore():
 *   Level 1 — soft reset (sleep → wake cycle via SET pin or UART command).
 *              Fast and non-destructive.  Used on the first failure.
 *   Level 2 — full deinit + reinit.  Used when level 1 is insufficient
 *              or the driver is in an unrecoverable internal state.
 *
 * After a successful pms7003_w_read() following a recovery, the
 * degraded flag is cleared automatically and the attempt counter is reset.
 * After PMS7003_MAX_RESTORE_ATTEMPTS consecutive failures,
 * pms7003_w_restore() returns ESP_ERR_NOT_RECOVERABLE.
 */

 #ifndef PMS7003_W_H
 #define PMS7003_W_H
 
 #include <stdbool.h>
 #include "esp_err.h"
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 /* ------------------------------------------------------------------ */
 /* Data types                                                           */
 /* ------------------------------------------------------------------ */
 
 /** @brief PM mass concentrations (atmospheric environment fields from PMS frame). */
 typedef struct
 {
     float pm1_0_ug_m3;  /**< PM1.0  atmospheric concentration [µg/m³] */
     float pm2_5_ug_m3;  /**< PM2.5  atmospheric concentration [µg/m³] */
     float pm10_ug_m3;   /**< PM10   atmospheric concentration [µg/m³] */
 } pms7003_data_t;
 
 /* ------------------------------------------------------------------ */
 /* API                                                                  */
 /* ------------------------------------------------------------------ */
 
 /**
  * @brief Initialize the PMS7003 sensor.
  *
  * Configures SET/RESET GPIOs, creates the internal FreeRTOS timer and
  * mutex, then triggers a reset sequence.  The sensor needs ~30 s to
  * stabilize after power-on; reads attempted before that return
  * @c ESP_ERR_NOT_FINISHED.
  *
  * @note The UART port must already be installed before calling this
  *       function (e.g. via uart_init_all).
  * @note Calling this function a second time while already initialized
  *       is a no-op and returns @c ESP_OK.
  *
  * @return ESP_OK on success, or an ESP-IDF / driver error code.
  */
 esp_err_t pms7003_w_init(void);
 
 /**
  * @brief Multi-level recovery for sensor crashes or communication errors.
  *
  * This function is non-blocking: it initiates the recovery and returns
  * immediately.  The sensor may need up to ~30 s to stabilize; the caller
  * should keep calling pms7003_w_read() and act on the result.
  *
  * Recovery levels:
  *   - Level 1 (first call after a failure): soft reset only.
  *     Sets the degraded flag; does NOT clear the attempt counter.
  *   - Level 2 (subsequent calls, or if level 1 fails structurally):
  *     full deinit + reinit.
  *
  * The degraded flag is cleared, and the attempt counter is reset, only
  * after a successful pms7003_w_read() confirms the sensor is healthy.
  *
  * @return
  *   - ESP_OK                   recovery initiated successfully.
  *   - ESP_ERR_NOT_RECOVERABLE  maximum attempt count reached; caller
  *                              should escalate (system reboot, alert…).
  *   - Other ESP-IDF error      driver-level failure during reinit.
  */
 esp_err_t pms7003_w_restore(void);
 
 /**
  * @brief Read the latest PM data from the PMS7003.
  *
  * Drains the UART RX buffer and scans backwards for the most recent
  * valid 32-byte frame (start-byte match + checksum).  Extracts PM1.0,
  * PM2.5 and PM10 atmospheric-environment values.
  *
  * On the first successful read after a recovery the degraded flag is
  * cleared automatically.
  *
  * @param[out] out  Destination structure.  Untouched on any error.
  *
  * @return
  *   - ESP_OK                valid data written to *out.
  *   - ESP_ERR_INVALID_ARG   @p out is NULL.
  *   - ESP_ERR_INVALID_STATE driver not initialized.
  *   - ESP_ERR_NOT_FINISHED  sensor still stabilizing, or no valid
  *                           frame available yet; retry later.
  */
 esp_err_t pms7003_w_read(pms7003_data_t *out);
 
 /**
  * @brief Check whether the sensor is in degraded (recovering) state.
  *
  * Returns true from the moment pms7003_w_restore() is called until
  * a subsequent pms7003_w_read() succeeds.  Useful for suppressing
  * alarms or annotating readings during recovery.
  *
  * @return true if the sensor is recovering, false if healthy.
  */
 bool pms7003_w_is_degraded(void);
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif /* PMS7003_W_H */