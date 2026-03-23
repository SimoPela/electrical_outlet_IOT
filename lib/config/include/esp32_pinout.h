/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#ifndef ESP32_PINOUT_H
#define ESP32_PINOUT_H

#include "driver/gpio.h"

/*
 * ============================================================
 * I2C BUS - Environmental sensors
 * SCD40, BMP280, AS7341, SGP41, SHT41
 * ============================================================
 */
#define PIN_I2C_SDA             GPIO_NUM_21
#define PIN_I2C_SCL             GPIO_NUM_22

/*
 * ============================================================
 * UART - PMS7003 particulate sensor
 * ============================================================
 */
 #define PIN_PMS7003_TX_SENSOR   GPIO_NUM_16   // Sensor TX -> ESP32 RX
 #define PIN_PMS7003_RX_SENSOR   GPIO_NUM_17   // Sensor RX -> ESP32 TX
 #define PIN_PMS7003_SET         GPIO_NUM_9
 #define PIN_PMS7003_RESET       GPIO_NUM_10

/*
 * ============================================================
 * GPIO - PIR AS312
 * ============================================================
 */
#define PIN_PIR_OUT             GPIO_NUM_27

/*
 * ============================================================
 * I2S - INMP441 microphone
 * ============================================================
 */
 #define PIN_I2S_WS             GPIO_NUM_25   // word select / LRCLK
 #define PIN_I2S_SCK            GPIO_NUM_26   // bit clock / BCLK
 #define PIN_I2S_SD             GPIO_NUM_33   // serial data from mic

/*
 * ============================================================
 * ADC - MICS-5524 gas sensor
 * ============================================================
 */
#define PIN_MICS_ADC            GPIO_NUM_34
// #define PIN_MICS_EN             GPIO_NUM_32   // optional, if connected

/*
 * ============================================================
 * GPIO - RGB LED
 * ============================================================
 */
#define PIN_LED_R               GPIO_NUM_13
#define PIN_LED_G               GPIO_NUM_12
#define PIN_LED_B               GPIO_NUM_14

/*
 * ============================================================
 * Optional grouped structure
 * Useful if you want to pass pin config around modules
 * ============================================================
 */
typedef struct
{
    gpio_num_t i2c_sda;
    gpio_num_t i2c_scl;

    gpio_num_t pms7003_rx;
    gpio_num_t pms7003_tx;

    gpio_num_t pir_out;

    gpio_num_t i2s_ws;
    gpio_num_t i2s_sck;
    gpio_num_t i2s_sd;

    gpio_num_t mics_adc;
    //gpio_num_t mics_en;

    gpio_num_t led_r;
    gpio_num_t led_g;
    gpio_num_t led_b;
} board_pins_t;

static const board_pins_t BOARD_PINS = {
    .i2c_sda = PIN_I2C_SDA,
    .i2c_scl = PIN_I2C_SCL,

    .pms7003_rx = PIN_PMS7003_RX_SENSOR,
    .pms7003_tx = PIN_PMS7003_TX_SENSOR,

    .pir_out = PIN_PIR_OUT,

    .i2s_ws = PIN_I2S_WS,
    .i2s_sck = PIN_I2S_SCK,
    .i2s_sd = PIN_I2S_SD,

    .mics_adc = PIN_MICS_ADC,
    //.mics_en = PIN_MICS_EN,

    .led_r = PIN_LED_R,
    .led_g = PIN_LED_G,
    .led_b = PIN_LED_B
};

#endif // ESP32_PINOUT_H