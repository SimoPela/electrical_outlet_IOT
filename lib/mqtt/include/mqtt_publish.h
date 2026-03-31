/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file mqtt_publish.h
 * @brief High-level MQTT publish API for telemetry, status, events, and batched periodic publish.
 */

#ifndef MQTT_PUBLISH_H
#define MQTT_PUBLISH_H

#include <stddef.h>
#include <stdbool.h>
#include "device_state.h"
#include "mqtt_client.h"

/**
 * @brief Publish environmental telemetry JSON.
 * @return 0 on success, negative on failure.
 */
int mqtt_publish_environment(esp_mqtt_client_handle_t client, const char *device_id, const device_state_t *state);

/**
 * @brief Publish system status payload.
 * @return 0 on success, negative on failure.
 */
 int mqtt_publish_system(esp_mqtt_client_handle_t client, const char *device_id, const device_state_t *state);

/**
 * @brief Publish alarm event payload.
 * @return 0 on success, negative on failure.
 */
int mqtt_publish_alarm(esp_mqtt_client_handle_t client, const char *device_id, const device_state_t *state);

#endif /* MQTT_PUBLISH_H */
