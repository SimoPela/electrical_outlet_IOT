/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file app_config_template.h
 * @brief Template for @c app_config.h — copy to @c app_config.h and fill credentials locally.
 */

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/** @name Wi-Fi station @{ */
#define APP_WIFI_SSID       "YOUR_WIFI_SSID"
#define APP_WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"
#define APP_WIFI_MAX_RETRY     10
/** @} */

/** @name MQTT client @{ */
#define APP_MQTT_BROKER_URI "mqtt://192.168.1.10"
#define APP_MQTT_USERNAME   ""
#define APP_MQTT_PASSWORD   ""
#define APP_MQTT_TOPIC_MAX_LEN 128
#define APP_MQTT_PAYLOAD_MAX_LEN 512
/** @} */

/*
 * MQTT QoS (Quality of Service) Levels
 * 
 * +---------+----------------+--------------------------+-------------------------------------------+
 * | QoS     | Guarantee      | Handshake Steps          | Use Case                                  |
 * +---------+----------------+--------------------------+-------------------------------------------+
 * | QoS 0   | At most once   | 1 (Fire and forget)      | Frequent, non-critical data (e.g., telemetry)         |
 * | QoS 1   | At least once  | 2 (PUBACK)               | Critical data where duplicates are acceptable         |
 * | QoS 2   | Exactly once   | 4 (PUBLISH, PUBREC,      | Mission-critical commands (e.g., alarms)              |
 * |         |                | PUBREL, PUBCOMP)         |                                               |
 * +---------+----------------+--------------------------+-------------------------------------------+
 *
 * For reference:
 *   APP_QOS_0: At most once
 *   APP_QOS_1: At least once
 *   APP_QOS_2: Exactly once
 */

#define APP_QOS_0           0
#define APP_QOS_1           1
#define APP_QOS_2           2

#define APP_RETAIN_0        0
#define APP_RETAIN_1        1

/** @name Device identity @{ */
#define APP_DEVICE_ID       "esp32_node_01"
/** @} */

#endif