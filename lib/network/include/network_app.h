/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file network_app.h
 * @brief Wi-Fi station bring-up and connection gating for MQTT startup.
 */

#ifndef NETWORK_APP_H
#define NETWORK_APP_H

#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"

/** @brief Event bit: station got IP. */
#define APP_WIFI_CONNECTED_BIT BIT0

/** @brief Event bit: connection failed after retries. */
#define APP_WIFI_FAIL_BIT BIT1

/**
 * @brief Initialize NVS, netif, Wi-Fi driver, and start connection using @c app_config.h credentials.
 * @return true on successful init path, false on fatal error.
 */
bool network_app_init(void);

/**
 * @brief Block until connected or until @p timeout_ms elapses.
 * @param[in] timeout_ms Maximum wait time in milliseconds.
 * @return true if @c APP_WIFI_CONNECTED_BIT is set, false on timeout or failure.
 */
bool network_app_wait_until_connected(uint32_t timeout_ms);

#endif /* NETWORK_APP_H */
