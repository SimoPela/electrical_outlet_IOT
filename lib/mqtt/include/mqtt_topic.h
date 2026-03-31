/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file mqtt_topic.h
 * @brief MQTT topic string builders under @c devices/&lt;device_id&gt;/…
 */

#ifndef MQTT_TOPIC_H
#define MQTT_TOPIC_H

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Format @c devices/&lt;device_id&gt;/&lt;suffix&gt; into @p buffer.
 *
 * @param[out] buffer Output buffer.
 * @param[in] size Capacity including NUL.
 * @param[in] device_id Device segment (non-NULL).
 * @param[in] suffix Topic tail after device id (non-NULL).
 * @return Length written (excluding NUL) on success, negative on error.
 */
int mqtt_topic_build(char *buffer, size_t size, const char *device_id, const char *suffix);

/** @brief Topic @c .../telemetry/environment */
int mqtt_topic_environment(char *buffer, size_t size, const char *device_id);

/** @brief Topic @c .../status/system */
int mqtt_topic_system(char *buffer, size_t size, const char *device_id);

/** @brief Topic @c .../event/alarm */
int mqtt_topic_alarm(char *buffer, size_t size, const char *device_id);

#endif /* MQTT_TOPIC_H */
