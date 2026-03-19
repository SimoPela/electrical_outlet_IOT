/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

 #include "network_app.h"
 #include "app_config.h"
 #include "device_state.h"
 
 #include <stdio.h>
 #include <stdint.h>
 #include <stdbool.h>
 
 #include "esp_event.h"
 #include "esp_log.h"
 #include "esp_netif.h"
 #include "esp_wifi.h"
 #include "nvs_flash.h"
 #include "esp_err.h"
 
 #include "freertos/FreeRTOS.h"
 #include "freertos/event_groups.h"
 #include "freertos/semphr.h"
 
 static const char *TAG = "NETWORK";
 
 static EventGroupHandle_t s_wifi_event_group = NULL;
 static int s_retry_num = 0;
 
 static void wifi_event_handler(void *arg,
                                esp_event_base_t event_base,
                                int32_t event_id,
                                void *event_data)
 {
     (void)arg;
 
     if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
     {
         ESP_LOGI(TAG, "Wi-Fi started, connecting...");
         esp_wifi_connect();
     }
     else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
     {
         xEventGroupClearBits(s_wifi_event_group, APP_WIFI_CONNECTED_BIT);
 
         if (xSemaphoreTake(g_device_state_mutex, portMAX_DELAY) == pdTRUE)
         {
             g_device_state.wifi_connected = false;
             g_device_state.mqtt_connected = false;
             xSemaphoreGive(g_device_state_mutex);
         }
 
         ESP_LOGW(TAG, "Wi-Fi disconnected");
 
         if (s_retry_num < APP_WIFI_MAX_RETRY)
         {
             esp_wifi_connect();
             s_retry_num++;
             ESP_LOGW(TAG, "Wi-Fi reconnect attempt %d/%d", s_retry_num, APP_WIFI_MAX_RETRY);
         }
         else
         {
             xEventGroupSetBits(s_wifi_event_group, APP_WIFI_FAIL_BIT);
             ESP_LOGE(TAG, "Wi-Fi connection failed");
         }
     }
     else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
     {
         ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
 
         ESP_LOGI(TAG, "Wi-Fi connected, got IP: " IPSTR, IP2STR(&event->ip_info.ip));
 
         if (xSemaphoreTake(g_device_state_mutex, portMAX_DELAY) == pdTRUE)
         {
             g_device_state.wifi_connected = true;
             xSemaphoreGive(g_device_state_mutex);
         }
 
         s_retry_num = 0;
         xEventGroupClearBits(s_wifi_event_group, APP_WIFI_FAIL_BIT);
         xEventGroupSetBits(s_wifi_event_group, APP_WIFI_CONNECTED_BIT);
     }
 }
 
 static bool network_nvs_init(void)
 {
     esp_err_t err = nvs_flash_init();
 
     if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
     {
         ESP_LOGW(TAG, "NVS requires erase, reinitializing");
         ESP_ERROR_CHECK(nvs_flash_erase());
         err = nvs_flash_init();
     }
 
     if (err != ESP_OK)
     {
         ESP_LOGE(TAG, "NVS init failed: %s", esp_err_to_name(err));
         return false;
     }
 
     ESP_LOGI(TAG, "NVS initialized");
     return true;
 }
 
 bool network_app_init(void)
 {
     if (!network_nvs_init())
     {
         return false;
     }
 
     s_wifi_event_group = xEventGroupCreate();
     if (s_wifi_event_group == NULL)
     {
         ESP_LOGE(TAG, "Failed to create Wi-Fi event group");
         return false;
     }
 
     ESP_ERROR_CHECK(esp_netif_init());
     ESP_ERROR_CHECK(esp_event_loop_create_default());
     esp_netif_create_default_wifi_sta();
 
     wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
     ESP_ERROR_CHECK(esp_wifi_init(&cfg));
 
     ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                         ESP_EVENT_ANY_ID,
                                                         &wifi_event_handler,
                                                         NULL,
                                                         NULL));
 
     ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                         IP_EVENT_STA_GOT_IP,
                                                         &wifi_event_handler,
                                                         NULL,
                                                         NULL));
 
     wifi_config_t wifi_config = {
         .sta = {
             .threshold.authmode = WIFI_AUTH_WPA2_PSK,
         },
     };
 
     snprintf((char *)wifi_config.sta.ssid,
              sizeof(wifi_config.sta.ssid),
              "%s",
              APP_WIFI_SSID);
 
     snprintf((char *)wifi_config.sta.password,
              sizeof(wifi_config.sta.password),
              "%s",
              APP_WIFI_PASSWORD);
 
     ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
     ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
     ESP_ERROR_CHECK(esp_wifi_start());
 
     ESP_LOGI(TAG, "Wi-Fi init complete");
     return true;
 }
 
 bool network_app_wait_until_connected(uint32_t timeout_ms)
 {
     if (s_wifi_event_group == NULL)
     {
         ESP_LOGE(TAG, "Wi-Fi event group not initialized");
         return false;
     }
 
     EventBits_t bits = xEventGroupWaitBits(
         s_wifi_event_group,
         APP_WIFI_CONNECTED_BIT | APP_WIFI_FAIL_BIT,
         pdFALSE,
         pdFALSE,
         pdMS_TO_TICKS(timeout_ms));
 
     if ((bits & APP_WIFI_CONNECTED_BIT) != 0)
     {
         ESP_LOGI(TAG, "Wi-Fi connection ready");
         return true;
     }
 
     if ((bits & APP_WIFI_FAIL_BIT) != 0)
     {
         ESP_LOGE(TAG, "Wi-Fi failed to connect");
         return false;
     }
 
     ESP_LOGE(TAG, "Wi-Fi connection timeout");
     return false;
 }