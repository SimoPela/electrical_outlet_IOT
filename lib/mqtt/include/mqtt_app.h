/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file mqtt_app.h
 * @brief MQTT client lifecycle: global handle and application start hook.
 */

#ifndef MQTT_APP_H
#define MQTT_APP_H

#include "mqtt_client.h"

/** @brief ESP-IDF MQTT client used by @c comm_task (NULL before @ref mqtt_app_start). */
extern esp_mqtt_client_handle_t g_mqtt_client;

/**
 * @brief Create MQTT client, register event handler, and start the client task.
 *
 * Typically called from @c app_main after Wi-Fi is connected.
 */
void mqtt_app_start(void);

#endif /* MQTT_APP_H */
