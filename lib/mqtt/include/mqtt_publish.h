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
 * @brief Publish audio telemetry JSON.
 * @return 0 on success, negative on failure.
 */
int mqtt_publish_audio(esp_mqtt_client_handle_t client, const char *device_id, const device_state_t *state);

/**
 * @brief Publish aggregate state JSON.
 * @return 0 on success, negative on failure.
 */
int mqtt_publish_state(esp_mqtt_client_handle_t client, const char *device_id, const device_state_t *state);

/** @brief Publish @c status/system JSON. */
int mqtt_publish_system(esp_mqtt_client_handle_t client, const char *device_id, const device_state_t *state);

/** @brief Publish @c status/faults JSON. */
int mqtt_publish_faults(esp_mqtt_client_handle_t client, const char *device_id, const device_state_t *state);

/** @brief Publish @c status/validity JSON. */
int mqtt_publish_validity(esp_mqtt_client_handle_t client, const char *device_id, const device_state_t *state);

/** @brief Publish @c status/last_update JSON. */
int mqtt_publish_last_update(esp_mqtt_client_handle_t client, const char *device_id, const device_state_t *state);

/**
 * @brief Publish device availability (online/offline).
 * @param[in] online True when connected and healthy enough to report.
 * @return 0 on success, negative on failure.
 */
int mqtt_publish_availability(esp_mqtt_client_handle_t client, const char *device_id, bool online);

/**
 * @brief Publish alarm event payload.
 * @return 0 on success, negative on failure.
 */
int mqtt_publish_alarm(esp_mqtt_client_handle_t client, const char *device_id, const device_state_t *state);

/**
 * @brief Publish the standard periodic set (environment, audio, state, system, faults, validity, last_update).
 * @return 0 if all succeeded, negative if any publish failed.
 */
int mqtt_publish_all_periodic(esp_mqtt_client_handle_t client, const char *device_id, const device_state_t *state);

#endif /* MQTT_PUBLISH_H */
