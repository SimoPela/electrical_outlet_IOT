#ifndef MQTT_PAYLOAD_H
#define MQTT_PAYLOAD_H

#include "device_state.h"
#include <stddef.h>

// Build the MQTT payload from the device state
int mqtt_payload_build(char *buffer, size_t buffer_size, const device_state_t *state);

#endif