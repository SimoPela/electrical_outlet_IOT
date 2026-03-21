/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#ifndef MQTT_PAYLOAD_H
#define MQTT_PAYLOAD_H

#include <stddef.h>
#include <stdbool.h>
#include "device_state.h"

/**
 * Build payload for:
 * devices/<device_id>/state
 */
int mqtt_payload_build_state(char *buffer, size_t buffer_size, const device_state_t *state);

/**
 * Build payload for:
 * devices/<device_id>/status/system
 */
int mqtt_payload_build_system(char *buffer, size_t buffer_size, const device_state_t *state);

/**
 * Build payload for:
 * devices/<device_id>/telemetry/environment
 */
int mqtt_payload_build_environment(char *buffer, size_t buffer_size, const device_state_t *state);

/**
 * Build payload for:
 * devices/<device_id>/telemetry/audio
 */
int mqtt_payload_build_audio(char *buffer, size_t buffer_size, const device_state_t *state);

/**
 * Build payload for:
 * devices/<device_id>/status/faults
 */
int mqtt_payload_build_faults(char *buffer, size_t buffer_size, const device_state_t *state);

/**
 * Build payload for:
 * devices/<device_id>/status/validity
 */
int mqtt_payload_build_validity(char *buffer, size_t buffer_size, const device_state_t *state);

/**
 * Build payload for:
 * devices/<device_id>/status/last_update
 */
int mqtt_payload_build_last_update(char *buffer, size_t buffer_size, const device_state_t *state);

/**
 * Build payload for:
 * devices/<device_id>/event/alarm
 */
int mqtt_payload_build_alarm(char *buffer, size_t buffer_size, const device_state_t *state);

/**
 * Build payload for:
 * devices/<device_id>/availability
 */
int mqtt_payload_build_availability(char *buffer, size_t buffer_size, bool online);


#endif
