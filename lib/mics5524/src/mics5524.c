#include "mics5524.h"
#include "adc_init.h"

#include "esp_check.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"

static const char *TAG = "MICS5524";
static const adc_channel_t MICS_ADC_CHANNEL = ADC_CHANNEL_6;

float mics5524_read_raw(void)
{
    adc_oneshot_unit_handle_t adc = adc_get_handle();
    if (adc == NULL) {
        ESP_LOGE(TAG, "ADC not initialized");
        return -1.0f;
    }

    int raw = 0;
    esp_err_t err = adc_oneshot_read(adc, MICS_ADC_CHANNEL, &raw);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ADC read failed");
        return -1.0f;
    }

    return (float)raw;
}