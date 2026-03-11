#include "mqtt_payload.h"
#include <stdio.h>

int mqtt_payload_build(char *buffer,
                       size_t buffer_size,
                       const device_state_t *state)
{
    if (!buffer || !state || buffer_size == 0)
    {
        return -1;
    }

    int written = snprintf(
        buffer,
        buffer_size,
        "{"
        "\"motion_detected\":%s,"
        "\"motion_valid\":%s,"
        "\"motion_fault\":%s,"
        "\"system_ok\":%s,"
        "\"degraded_mode\":%s,"
        "\"alarm_active\":%s"
        "}",
        state->motion_detected ? "true" : "false",
        state->motion_valid ? "true" : "false",
        state->motion_fault ? "true" : "false",
        state->system_ok ? "true" : "false",
        state->degraded_mode ? "true" : "false",
        state->alarm_active ? "true" : "false");

    if (written < 0 || (size_t)written >= buffer_size)
    {
        return -1;
    }

    return written;
}