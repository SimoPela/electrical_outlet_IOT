/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file mqtt_payload.c
 * @brief JSON payload builders for device-centric MQTT topics.
 */

#include "mqtt_payload.h"
#include <stdio.h>
#include "esp_log.h"

static const char *TAG = "MQTT_PAYLOAD";

/**
 * @brief Validate the return value of @c snprintf and log on overflow or error.
 *
 * @param[in] written     Return value of @c snprintf.
 * @param[in] buffer_size Capacity of the target buffer.
 * @return @p written on success, @c -1 on truncation or encoding error.
 */
static int mqtt_payload_check_result(int written, size_t buffer_size)
{
    if (written < 0 || (size_t)written >= buffer_size)
    {
        ESP_LOGE(TAG, "MQTT payload build failed: written = %d, buffer_size = %zu", written, buffer_size);
        return -1;
    }

    return written;
}

/** @copydoc mqtt_payload_build_system */
int mqtt_payload_build_system(char *buffer, size_t buffer_size, const device_state_t *state, bool online)
{
    if (!buffer || !state || buffer_size == 0)
    {
        ESP_LOGE(TAG, "MQTT system payload build failed: buffer=%p state=%p buffer_size=%zu", buffer, state, buffer_size);
        return -1;
    }

    int written = snprintf(
        buffer,
        buffer_size,
        "{"
        "\"system_ok\":%s,"
        "\"degraded_mode\":%s,"
        "\"wifi_connected\":%s,"
        "\"mqtt_connected\":%s,"
        "\"status\":%s"
        "}",
        state->system_ok ? "true" : "false",
        state->degraded_mode ? "true" : "false",
        state->wifi_connected ? "true" : "false",
        state->mqtt_connected ? "true" : "false",
        online ? "online" : "offline");

    return mqtt_payload_check_result(written, buffer_size);
}

/** @copydoc mqtt_payload_build_environment */
int mqtt_payload_build_environment(char *buffer, size_t buffer_size, const device_state_t *state)
{
    if (!buffer || !state || buffer_size == 0)
    {
        ESP_LOGE(TAG, "MQTT environment payload build failed: buffer = %p, state = %p, buffer_size = %zu", buffer, state, buffer_size);
        return -1;
    }

    int written = snprintf(
        buffer,
        buffer_size,
        "{"
        "\"co2_ppm\":%.2f,"
        "\"temperature_scd40\":%.2f,"
        "\"humidity_scd40\":%.2f,"

        "\"temperature_c\":%.2f,"
        "\"humidity_percent\":%.2f,"
        "\"bmp280_pressure_hpa\":%.2f,"
        "\"bmp280_temperature_c\":%.2f,"

        "\"voc_index\":%.2f,"
        "\"nox_index\":%.2f,"

        "\"pm1_0_ug_m3\":%.2f,"
        "\"pm2_5_ug_m3\":%.2f,"
        "\"pm10_ug_m3\":%.2f,"

        "\"light\":[%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f],"
        "\"gas_level_raw\":%.2f,"
        "\"gas_ppm\":%.2f,"
        "\"noise_db\":%.2f"
        "}",
        state->co2_ppm,
        state->temperature_scd40,
        state->humidity_scd40,
        state->temperature_c,
        state->humidity_percent,
        state->bmp280_pressure_hpa,
        state->bmp280_temperature_c,
        state->voc_index,
        state->nox_index,
        state->pm1_0_ug_m3,
        state->pm2_5_ug_m3,
        state->pm10_ug_m3,
        state->light.channels[0],
        state->light.channels[1],
        state->light.channels[2],
        state->light.channels[3],
        state->light.channels[4],
        state->light.channels[5],
        state->light.channels[6],
        state->light.channels[7],
        state->mics5524_gas_level_raw,
        state->mics5524_gas_ppm,
        state->noise_db);

    return mqtt_payload_check_result(written, buffer_size);
}

/** @copydoc mqtt_payload_build_alarm */
int mqtt_payload_build_alarm(char *buffer, size_t buffer_size, const device_state_t *state)
{
    if (!buffer || !state || buffer_size == 0)
    {
        ESP_LOGE(TAG, "MQTT alarm payload build failed: buffer=%p state=%p buffer_size=%zu",
                 buffer, state, buffer_size);
        return -1;
    }

    int written = snprintf(
        buffer,
        buffer_size,
        "{"
        "\"as312_alarm\":%s,"
        "\"mics5524_alarm\":%s,"
        "\"co2_alarm_level\":%s"
        "}",
        state->as312_alarm ? "true" : "false",
        state->mics5524_alarm ? "true" : "false",
        state->co2_alarm_level);

    return mqtt_payload_check_result(written, buffer_size);
}
