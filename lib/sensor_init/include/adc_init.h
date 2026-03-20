/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


 #ifndef ADC_INIT_H
 #define ADC_INIT_H
 
 #include "esp_err.h"
 #include "esp_adc/adc_oneshot.h"
 
 esp_err_t adc_init_all(void);
 adc_oneshot_unit_handle_t adc_get_handle(void);
 
 #endif