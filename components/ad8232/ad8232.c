/*
 * @file ad8232.c
 * @brief implement for ad8232.h
 * @author Nguyen Hoang
*/

#include "ad8232.h"
#include <string.h>
#include <esp_err.h>
#include <esp_log.h>
#include "soc/soc_caps.h"

static const char *TAG = "AD8232";

/* ISR context */
static bool IRAM_ATTR on_pool_ovf(adc_continuous_handle_t handle,const adc_continuous_evt_data_t *edata, void *user_data)
{
    (void)handle;
    (void)edata ;
    AD8232_t *ad8232 = (AD8232_t *)user_data;
    ad8232->overrun_count++;
    return false;   
}

AD8232_status ad8232_set_enabled(AD8232_t *dev, bool enabled)
{
    if (dev == NULL) {
        return AD8232_ERR_INVALID_ARG;
    }
    gpio_set_level(dev->cfg.sdn_pin, enabled);   
    return AD8232_OK;
}

bool ad8232_isConected(const AD8232_t *ad8232){
    bool LO_plus = (gpio_get_level(ad8232->cfg.lo_plus_pin) == 0) ;
    bool LO_minus = ( gpio_get_level(ad8232->cfg.lo_minus_pin) == 0) ;
    /**  Lo_minus  | Lo_plus  | conected (SDN pin)
     *      0           0           1
     *      1           1           0
     *      0           1           0
     *      1           0           1
     */
    return LO_plus && LO_minus;
};



AD8232_status ad8232_init(AD8232_t *ad8232, const AD8232_config_t *cfg) {

    if ( ad8232 == NULL || cfg == NULL ) {
        ESP_LOGE(TAG , "ad8232 is null "); 
        return AD8232_ERR_INVALID_ARG ;
    }

    memset(ad8232, 0, sizeof(*ad8232));
    ad8232->cfg = *cfg;

    ad8232->mutex = xSemaphoreCreateMutex();
    if (ad8232->mutex == NULL) {
        return AD8232_ERR_NO_MEM;
    }

    adc_continuous_handle_cfg_t handle_cfg = {
    .conv_frame_size = AD8232_FRAME_BYTES,
    .max_store_buf_size = AD8232_POOL_BYTES,
    };

    adc_digi_pattern_config_t adc_pattern_CF = {
        .atten = ADC_ATTEN_DB_12 ,
        .bit_width = ADC_BITWIDTH_12 ,
        .unit = ADC_UNIT_1 ,
        .channel = ad8232->cfg.channel
    } ;

    adc_continuous_config_t adc_CF = {
        .adc_pattern = &adc_pattern_CF ,
        .conv_mode  = ADC_CONV_SINGLE_UNIT_1 ,
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE2 ,
        .pattern_num = 1 ,
        .sample_freq_hz = ad8232->cfg.sample_freq_hz  
    };

    adc_continuous_evt_cbs_t cbs = { .on_pool_ovf = on_pool_ovf };

    gpio_config_t io_in = {
        .pin_bit_mask = (1Ull << ad8232->cfg.lo_plus_pin) | (1ULL << ad8232->cfg.lo_minus_pin) ,
        .intr_type = GPIO_INTR_DISABLE ,
        .mode = GPIO_MODE_INPUT ,
        .pull_down_en = GPIO_PULLDOWN_DISABLE ,
        .pull_up_en   = GPIO_PULLUP_DISABLE   ,
    };
    gpio_config_t io_out = {
        .pin_bit_mask = ( 1ULL << ad8232->cfg.sdn_pin),
        .mode = GPIO_MODE_OUTPUT ,
        .intr_type = GPIO_INTR_DISABLE ,
        .pull_down_en = GPIO_PULLDOWN_DISABLE ,
        .pull_up_en   = GPIO_PULLUP_DISABLE   ,
    };

    AD8232_status st = AD8232_OK;

    if (gpio_config(&io_in) != ESP_OK || gpio_config(&io_out) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to config GPIO");
        st = AD8232_ERR_GPIO;
    } else if (adc_continuous_new_handle(&handle_cfg, &ad8232->adc) != ESP_OK) {
        ad8232->adc = NULL;
        ESP_LOGE(TAG, "Failed to create ADC handle");
        st = AD8232_ERR_ADC;
    } else if (adc_continuous_config(ad8232->adc, &adc_CF) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to config ADC");
        st = AD8232_ERR_ADC;
    } else if (adc_continuous_register_event_callbacks(ad8232->adc, &cbs, ad8232) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register ADC callback");   /* must be before start */
        st = AD8232_ERR_ADC;
    } else if (gpio_set_level(cfg->sdn_pin, 1) != ESP_OK) {   /* chip on */
        st = AD8232_ERR_GPIO;
    } else if (adc_continuous_start(ad8232->adc) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start ADC");
        st = AD8232_ERR_ADC;
    } else {
        ad8232->running = true;
    }
    
    if (st != AD8232_OK) {
        if (ad8232->adc != NULL) {
            adc_continuous_deinit(ad8232->adc);
            ad8232->adc = NULL;
        }
        gpio_set_level(cfg->sdn_pin, 0);
        vSemaphoreDelete(ad8232->mutex);
        ad8232->mutex = NULL;
    }

    return st;
}



AD8232_status ad8232_deinit(AD8232_t *ad8232) {

    if ( ad8232 == NULL || ad8232->mutex == NULL) return AD8232_ERR_INVALID_ARG ;

    xSemaphoreTake(ad8232->mutex, portMAX_DELAY);

    AD8232_status st = AD8232_OK;

    if (ad8232->running) {
        if (adc_continuous_stop(ad8232->adc) == ESP_OK) {
            ad8232->running = false;
        } else {
            ESP_LOGE(TAG, "Failed to stop ADC");
            st = AD8232_ERR_ADC;
        }
    }
    if (ad8232->adc != NULL) {
        if (adc_continuous_deinit(ad8232->adc) == ESP_OK) {
            ad8232->adc = NULL;
        } else {
            ESP_LOGE(TAG, "Failed to deinit ADC");
            st = AD8232_ERR_ADC;
        }
    }
    gpio_set_level(ad8232->cfg.sdn_pin, 0);   

    SemaphoreHandle_t mutex = ad8232->mutex;
    ad8232->mutex = NULL;
    xSemaphoreGive(mutex);
    vSemaphoreDelete(mutex);
    return st;
}


AD8232_status ad8232_read(AD8232_t *ad8232, uint8_t *buf, uint32_t buf_size, uint32_t *out_len, uint32_t timeout_ms)
{
    if (ad8232 == NULL || ad8232->mutex == NULL || buf == NULL || out_len == NULL) {
        return AD8232_ERR_INVALID_ARG;
    }
    *out_len = 0;

    buf_size -= buf_size % SOC_ADC_DIGI_RESULT_BYTES;   /* 1 sample = 4 bytes */
    if (buf_size == 0) {
        return AD8232_ERR_INVALID_ARG;
    }


    TickType_t start = xTaskGetTickCount();
    if (xSemaphoreTake(ad8232->mutex, pdMS_TO_TICKS(timeout_ms)) != pdTRUE) {
        return AD8232_ERR_BUSY;
    }
    uint32_t used_ms = pdTICKS_TO_MS(xTaskGetTickCount() - start);
    uint32_t left_ms = (used_ms >= timeout_ms) ? 0 : (timeout_ms - used_ms);

    
    AD8232_status st = AD8232_OK;
    uint32_t got = 0;

    if (!ad8232->running) {
        st = AD8232_ERR_INVALID_STATE;
    } else {
        esp_err_t res = adc_continuous_read(ad8232->adc, buf, buf_size, &got, left_ms);

        if (res == ESP_ERR_TIMEOUT) {
            st = AD8232_ERR_TIMEOUT;
        } else if (res == ESP_ERR_INVALID_STATE) {  
            st = AD8232_ERR_OVERRUN;
        } else if (res != ESP_OK) {
            st = AD8232_ERR_ADC;
        } else if (!ad8232_isConected(ad8232)) {     
            st = AD8232_ERR_LEADS_OFF;
        } else {
            *out_len = got;
        }
    }

    xSemaphoreGive(ad8232->mutex);
    return st;
}

uint32_t ad8232_parse(const AD8232_t *ad8232, const uint8_t *raw, uint32_t raw_len, uint16_t *samples, uint32_t max_samples)
{
    uint32_t count = 0;

    for (uint32_t i = 0; i + SOC_ADC_DIGI_RESULT_BYTES <= raw_len && count < max_samples;
         i += SOC_ADC_DIGI_RESULT_BYTES) {

        adc_digi_output_data_t sample;
        memcpy(&sample, &raw[i], sizeof(sample));   /* raw[i] may not be 4-byte aligned */

        if (sample.type2.unit == 0 &&
            sample.type2.channel == (ad8232->cfg.channel & 0x7)) {
            samples[count++] = sample.type2.data;
        }
    }
    return count;
}
