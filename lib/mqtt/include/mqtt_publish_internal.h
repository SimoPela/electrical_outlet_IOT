/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file mqtt_publish_internal.h
 * @brief Low-level MQTT publish helpers (topic/payload builder callbacks).
 */

#ifndef MQTT_PUBLISH_INTERNAL_H
#define MQTT_PUBLISH_INTERNAL_H

#include "mqtt_client.h"
#include "device_state.h"

/**
 * @brief Publish a raw UTF-8 payload to a full topic string.
 *
 * @param[in] client ESP-IDF MQTT client handle.
 * @param[in] topic  Null-terminated MQTT topic.
 * @param[in] payload Null-terminated JSON or text body.
 * @param[in] qos    MQTT QoS level (0–2).
 * @param[in] retain MQTT retain flag.
 * @return 0 on success, negative value on failure.
 */
int mqtt_publish_raw(esp_mqtt_client_handle_t client, const char *topic, const char *payload, int qos, int retain);

/**
 * @brief Build topic and payload via callbacks, then publish.
 *
 * @param[in] client ESP-IDF MQTT client handle.
 * @param[in] device_id Logical device id (topic segment).
 * @param[in] topic_builder Writes topic into buffer; returns length or negative on error.
 * @param[in] payload_builder Writes JSON into buffer from @p state; returns length or negative.
 * @param[in] state Snapshot to serialize.
 * @param[in] qos MQTT QoS.
 * @param[in] retain MQTT retain.
 * @return 0 on success, negative on build or publish failure.
 */
int mqtt_publish_with_builder(esp_mqtt_client_handle_t client,
                              const char *device_id,
                              int (*topic_builder)(char *, size_t, const char *),
                              int (*payload_builder)(char *, size_t, const device_state_t *),
                              const device_state_t *state,
                              int qos,
                              int retain);

#endif /* MQTT_PUBLISH_INTERNAL_H */
