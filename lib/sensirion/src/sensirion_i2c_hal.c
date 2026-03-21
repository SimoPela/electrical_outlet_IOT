/*
 * Copyright (c) 2018, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

 #include "sensirion_i2c_hal.h"
 #include "sensirion_common.h"
 #include "sensirion_config.h"
 
 #include "driver/i2c.h"
 #include "esp_rom_sys.h"
 #include "freertos/FreeRTOS.h"
 #include "freertos/task.h"
 
 #define SENSIRION_I2C_PORT    I2C_NUM_0
 #define SENSIRION_I2C_TIMEOUT pdMS_TO_TICKS(20)
 
 int16_t sensirion_i2c_hal_select_bus(uint8_t bus_idx) {
     (void)bus_idx;
     return NOT_IMPLEMENTED_ERROR; /* single bus — not needed */
 }
 
 void sensirion_i2c_hal_init(void) {
     /* Bus already initialized by i2c_init_all() in app_main — no-op */
 }
 
 void sensirion_i2c_hal_free(void) {
     /* Bus lifecycle managed by i2c_init.c — no-op */
 }
 
 int8_t sensirion_i2c_hal_read(uint8_t address, uint8_t* data, uint8_t count) {
     esp_err_t err = i2c_master_read_from_device(
         SENSIRION_I2C_PORT,
         address,
         data,
         count,
         SENSIRION_I2C_TIMEOUT
     );
     return (err == ESP_OK) ? 0 : -1;
 }
 
 int8_t sensirion_i2c_hal_write(uint8_t address, const uint8_t* data,
                                uint8_t count) {
     esp_err_t err = i2c_master_write_to_device(
         SENSIRION_I2C_PORT,
         address,
         data,
         count,
         SENSIRION_I2C_TIMEOUT
     );
     return (err == ESP_OK) ? 0 : -1;
 }
 
 void sensirion_i2c_hal_sleep_usec(uint32_t useconds) {
     if (useconds >= 1000) {
         vTaskDelay(pdMS_TO_TICKS(useconds / 1000));
     } else {
         esp_rom_delay_us(useconds);
     }
 }