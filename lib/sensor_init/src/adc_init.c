/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


 #include "adc_init.h"
 #include "esp_log.h"
 #include "esp_check.h"
 #include "esp_adc/adc_oneshot.h"
 
 static const char *TAG = "ADC_INIT";
 static adc_oneshot_unit_handle_t s_adc_handle = NULL;
 
 esp_err_t adc_init_all(void)
 {
     if (s_adc_handle != NULL) {
         return ESP_OK;
     }
 
     adc_oneshot_unit_init_cfg_t unit_cfg = {
         .unit_id = ADC_UNIT_1,
         .ulp_mode = ADC_ULP_MODE_DISABLE
     };
 
     ESP_RETURN_ON_ERROR(
         adc_oneshot_new_unit(&unit_cfg, &s_adc_handle),
         TAG,
         "Failed to create ADC oneshot unit"
     );
 
     adc_oneshot_chan_cfg_t chan_cfg = {
         .atten = ADC_ATTEN_DB_12,
         .bitwidth = ADC_BITWIDTH_12
     };
 
     ESP_RETURN_ON_ERROR(
         adc_oneshot_config_channel(s_adc_handle, ADC_CHANNEL_6, &chan_cfg),
         TAG,
         "Failed to configure ADC channel 6"
     );
 
     ESP_LOGI(TAG, "ADC initialized");
     return ESP_OK;
 }
 
 adc_oneshot_unit_handle_t adc_get_handle(void)
 {
     return s_adc_handle;
 }

 esp_err_t adc_restore(void)
 {
     if (s_adc_handle != NULL) {
         esp_err_t err = adc_oneshot_del_unit(s_adc_handle);
         s_adc_handle = NULL;
         if (err != ESP_OK) {
             ESP_LOGE(TAG, "adc_oneshot_del_unit failed: %s", esp_err_to_name(err));
             return err;
         }
     }
     return adc_init_all();
 }