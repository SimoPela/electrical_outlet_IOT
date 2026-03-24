#ifndef INMP441_W_H
#define INMP441_W_H

#include "esp_err.h"

typedef struct
{
    /** SPL approximatif (dB), formule DS + offset INMP441_SPL_OFFSET_DB (voir inmp441_w.c). */
    float noise_db;
} inmp441_data_t;

esp_err_t inmp441_w_init(void);

esp_err_t inmp441_w_read(inmp441_data_t *out);

#endif /* INMP441_W_H */