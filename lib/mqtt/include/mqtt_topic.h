/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#ifndef MQTT_TOPIC_H
#define MQTT_TOPIC_H

#include <stddef.h>
#include <stdbool.h>

/**
 * Build a generic topic:
 * devices/<device_id>/<suffix>
 */
 int mqtt_topic_build(char *buffer, size_t size, const char *device_id, const char *suffix);

 /**
 * Topic helpers
 */
int mqtt_topic_state(char *buffer, size_t size, const char *device_id);
int mqtt_topic_environment(char *buffer, size_t size, const char *device_id);
int mqtt_topic_audio(char *buffer, size_t size, const char *device_id);
int mqtt_topic_system(char *buffer, size_t size, const char *device_id);
int mqtt_topic_faults(char *buffer, size_t size, const char *device_id);
int mqtt_topic_validity(char *buffer, size_t size, const char *device_id);
int mqtt_topic_last_update(char *buffer, size_t size, const char *device_id);
int mqtt_topic_alarm(char *buffer, size_t size, const char *device_id);
int mqtt_topic_availability(char *buffer, size_t size, const char *device_id);




#endif // MQTT_TOPIC_H