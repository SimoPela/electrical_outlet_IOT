/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file mqtt_payload.h
 * @brief JSON payload builders for device-centric MQTT topics.
 */

#ifndef MQTT_PAYLOAD_H
#define MQTT_PAYLOAD_H

#include <stddef.h>
#include <stdbool.h>
#include "device_state.h"

/**
 * @brief Serialize compact device state for @c devices/&lt;id&gt;/state.
 * @param[out] buffer Output buffer.
 * @param[in] buffer_size Capacity of @p buffer including trailing NUL.
 * @param[in] state Source snapshot.
 * @return Bytes written (excluding NUL) on success, negative on error or overflow.
 */
int mqtt_payload_build_state(char *buffer, size_t buffer_size, const device_state_t *state);

/**
 * @brief Serialize system flags for @c devices/&lt;id&gt;/status/system.
 * @param[out] buffer Output buffer.
 * @param[in] buffer_size Capacity of @p buffer.
 * @param[in] state Source snapshot.
 * @return Bytes written on success, negative on error.
 */
int mqtt_payload_build_system(char *buffer, size_t buffer_size, const device_state_t *state);

/**
 * @brief Serialize environmental telemetry for @c devices/&lt;id&gt;/telemetry/environment.
 * @param[out] buffer Output buffer.
 * @param[in] buffer_size Capacity of @p buffer.
 * @param[in] state Source snapshot.
 * @return Bytes written on success, negative on error.
 */
int mqtt_payload_build_environment(char *buffer, size_t buffer_size, const device_state_t *state);

/**
 * @brief Serialize audio summary for @c devices/&lt;id&gt;/telemetry/audio.
 * @param[out] buffer Output buffer.
 * @param[in] buffer_size Capacity of @p buffer.
 * @param[in] state Source snapshot.
 * @return Bytes written on success, negative on error.
 */
int mqtt_payload_build_audio(char *buffer, size_t buffer_size, const device_state_t *state);

/**
 * @brief Serialize fault flags for @c devices/&lt;id&gt;/status/faults.
 * @param[out] buffer Output buffer.
 * @param[in] buffer_size Capacity of @p buffer.
 * @param[in] state Source snapshot.
 * @return Bytes written on success, negative on error.
 */
int mqtt_payload_build_faults(char *buffer, size_t buffer_size, const device_state_t *state);

/**
 * @brief Serialize validity flags for @c devices/&lt;id&gt;/status/validity.
 * @param[out] buffer Output buffer.
 * @param[in] buffer_size Capacity of @p buffer.
 * @param[in] state Source snapshot.
 * @return Bytes written on success, negative on error.
 */
int mqtt_payload_build_validity(char *buffer, size_t buffer_size, const device_state_t *state);

/**
 * @brief Serialize last-update ticks for @c devices/&lt;id&gt;/status/last_update.
 * @param[out] buffer Output buffer.
 * @param[in] buffer_size Capacity of @p buffer.
 * @param[in] state Source snapshot.
 * @return Bytes written on success, negative on error.
 */
int mqtt_payload_build_last_update(char *buffer, size_t buffer_size, const device_state_t *state);

/**
 * @brief Serialize alarm event for @c devices/&lt;id&gt;/event/alarm.
 * @param[out] buffer Output buffer.
 * @param[in] buffer_size Capacity of @p buffer.
 * @param[in] state Source snapshot.
 * @return Bytes written on success, negative on error.
 */
int mqtt_payload_build_alarm(char *buffer, size_t buffer_size, const device_state_t *state);

/**
 * @brief Serialize LWT-style availability for @c devices/&lt;id&gt;/availability.
 * @param[out] buffer Output buffer.
 * @param[in] buffer_size Capacity of @p buffer.
 * @param[in] online True if device is online.
 * @return Bytes written on success, negative on error.
 */
int mqtt_payload_build_availability(char *buffer, size_t buffer_size, bool online);

#endif /* MQTT_PAYLOAD_H */
