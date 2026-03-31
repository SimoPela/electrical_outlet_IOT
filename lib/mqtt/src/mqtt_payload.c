/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#include "mqtt_payload.h"
#include <stdio.h>
#include "esp_log.h"

static const char *TAG = "MQTT_PAYLOAD";

// this function checks the result of the snprintf function
static int mqtt_payload_check_result(int written, size_t buffer_size) 
{
    if (written < 0 || (size_t)written >= buffer_size)
    {
        ESP_LOGE(TAG, "MQTT payload build failed: written = %d, buffer_size = %zu", written, buffer_size);
        return -1;
    }

    return written;
}

// this function builds the payload for the state topic
int mqtt_payload_build_state(char *buffer, size_t buffer_size, const device_state_t *state)
{
    if (!buffer || !state || buffer_size == 0)
    {
        ESP_LOGE(TAG, "MQTT state payload build failed: buffer=%p state=%p buffer_size=%zu", buffer, state, buffer_size);
        return -1;
    }

    int written = snprintf(
        buffer,
        buffer_size,
        "{"
        "\"system_ok\":%s,"
        "\"degraded_mode\":%s"
        "}",
        state->system_ok ? "true" : "false",
        state->degraded_mode ? "true" : "false");

    return mqtt_payload_check_result(written, buffer_size);
}

int mqtt_payload_build_system(char *buffer, size_t buffer_size, const device_state_t *state)
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
        "\"mqtt_connected\":%s"
        "}",
        state->system_ok ? "true" : "false",
        state->degraded_mode ? "true" : "false",
        state->wifi_connected ? "true" : "false",
        state->mqtt_connected ? "true" : "false");

    return mqtt_payload_check_result(written, buffer_size);
}

// this function builds the payload for the environment topic
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
        "\"gas_ppm\":%.2f"
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
        state->mics5524_gas_ppm);

    return mqtt_payload_check_result(written, buffer_size);
}

// this function builds the payload for the audio topic
int mqtt_payload_build_audio(char *buffer, size_t buffer_size, const device_state_t *state)
{
    if (!buffer || !state || buffer_size == 0)
    {
        ESP_LOGE(TAG, "MQTT audio payload build failed: buffer = %p, state = %p, buffer_size = %zu", buffer, state, buffer_size);
        return -1;
    }

    int written = snprintf(
        buffer,
        buffer_size,
        "{"
        "\"noise_db\":%.2f"
        "}",
        state->noise_db);

    return mqtt_payload_check_result(written, buffer_size);
}

// this function builds the payload for the faults topic
int mqtt_payload_build_faults(char *buffer, size_t buffer_size, const device_state_t *state)
{
    if (!buffer || !state || buffer_size == 0)
    {
        ESP_LOGE(TAG, "MQTT faults payload build failed: buffer = %p, state = %p, buffer_size = %zu", buffer, state, buffer_size);
        return -1;
    }

    int written = snprintf(
        buffer,
        buffer_size,
        "{"
        "\"as312_fault\":%s,"
        "\"mics5524_fault\":%s,"
        "\"sgp41_fault\":%s,"
        "\"sht41_fault\":%s,"
        "\"bmp280_fault\":%s,"
        "\"scd40_fault\":%s,"
        "\"pms7003_fault\":%s,"
        "\"as7341_fault\":%s,"
        "\"inmp441_fault\":%s"  
        "}",
        state->as312_fault ? "true" : "false",
        state->mics5524_fault ? "true" : "false",
        state->sgp41_fault ? "true" : "false",
        state->sht41_fault ? "true" : "false",
        state->bmp280_fault ? "true" : "false",
        state->scd40_fault ? "true" : "false",
        state->pms7003_fault ? "true" : "false",
        state->as7341_fault ? "true" : "false",
        state->inmp441_fault ? "true" : "false");

    return mqtt_payload_check_result(written, buffer_size);
}

// this function builds the payload for the validity topic
int mqtt_payload_build_validity(char *buffer, size_t buffer_size, const device_state_t *state)
{
    if (!buffer || !state || buffer_size == 0)
    {
        ESP_LOGE(TAG, "MQTT validity payload build failed: buffer = %p, state = %p, buffer_size = %zu", buffer, state, buffer_size);
        return -1;
    }

    int written = snprintf(
        buffer,
        buffer_size,
        "{"
        "\"as312_valid\":%s,"
        "\"mics5524_valid\":%s,"
        "\"sgp41_valid\":%s,"
        "\"sht41_valid\":%s,"
        "\"bmp280_valid\":%s,"
        "\"scd40_valid\":%s,"
        "\"pms7003_valid\":%s,"
        "\"as7341_valid\":%s,"
        "\"inmp441_valid\":%s"
        "}",
        state->as312_valid ? "true" : "false",
        state->mics5524_valid ? "true" : "false",
        state->sgp41_valid ? "true" : "false",
        state->sht41_valid ? "true" : "false",
        state->bmp280_valid ? "true" : "false",
        state->scd40_valid ? "true" : "false",
        state->pms7003_valid ? "true" : "false",
        state->as7341_valid ? "true" : "false",
        state->inmp441_valid ? "true" : "false");

    return mqtt_payload_check_result(written, buffer_size);
}

// this function builds the payload for the last update topic
int mqtt_payload_build_last_update(char *buffer, size_t buffer_size, const device_state_t *state)
{
    if (!buffer || !state || buffer_size == 0)
    {
        ESP_LOGE(TAG, "MQTT last update payload build failed: buffer = %p, state = %p, buffer_size = %zu", buffer, state, buffer_size);
        return -1;
    }

    int written = snprintf(
        buffer,
        buffer_size,
        "{"
        "\"as312_last_update\":%lu,"
        "\"mics5524_last_update\":%lu,"
        "\"sgp41_last_update\":%lu,"
        "\"sht41_last_update\":%lu,"
        "\"bmp280_last_update\":%lu,"
        "\"scd40_last_update\":%lu,"
        "\"pms7003_last_update\":%lu,"
        "\"as7341_last_update\":%lu,"
        "\"inmp441_last_update\":%lu"
        "}",
        (unsigned long)state->as312_last_update,
        (unsigned long)state->mics5524_last_update,
        (unsigned long)state->sgp41_last_update,
        (unsigned long)state->sht41_last_update,
        (unsigned long)state->bmp280_last_update,
        (unsigned long)state->scd40_last_update,
        (unsigned long)state->pms7003_last_update,
        (unsigned long)state->as7341_last_update,
        (unsigned long)state->inmp441_last_update);

    return mqtt_payload_check_result(written, buffer_size);
}

// this function builds the payload for the alarm topic
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
        "\"mics5524_alarm\":%s"
        "}",
        state->as312_alarm ? "true" : "false",
        state->mics5524_alarm ? "true" : "false");

    return mqtt_payload_check_result(written, buffer_size);
}

// this function builds the payload for the availability topic
int mqtt_payload_build_availability(char *buffer, size_t buffer_size, bool online)
{
    if (!buffer || buffer_size == 0)
    {
        ESP_LOGE(TAG, "MQTT availability payload build failed: buffer=%p buffer_size=%zu",
                 buffer, buffer_size);
        return -1;
    }

    int written = snprintf(buffer, buffer_size, "%s", online ? "online" : "offline");

    return mqtt_payload_check_result(written, buffer_size);
}