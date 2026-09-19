/*
 * @file ad8232.c
 * @brief implement for ad8232.h
 * @author Nguyen Hoang
*/

#include "ad8232.h"
#include <string.h>
#include <esp_err.h>
#include <esp_log.h>

static const char *TAG = "AD8232";


AD8232_status ad8232_init(AD8232_t *ad8232) {

    if ( ad8232 == NULL ) {
        ESP_LOGE(TAG , "ad8232 is null "); 
        return error;
    }

    adc_continuous_handle_cfg_t adc_nHDCF = {
    .conv_frame_size = 256,
    .max_store_buf_size = 1024,
    };

    esp_err_t res =  adc_continuous_new_handle(&adc_nHDCF , &ad8232->adc_cHD );
    if (res != ESP_OK ) {
        ESP_LOGE(TAG , "Failed to create new adc_c handle",esp_err_to_name(res));
        return error ;
    } ESP_LOGE(TAG, "Creat new adc_c handle sucess");

    adc_digi_pattern_config_t adc_pattern_CF = {
        .atten = ADC_ATTEN_DB_12,
        .bit_width = ADC_BITWIDTH_12,
        .unit = ADC_UNIT_1,
        .channel = ad8232->channel_ADC,
    };

    adc_continuous_config_t adc_CF = {
        .adc_pattern = &adc_pattern_CF ,
        .conv_mode  = ADC_CONV_SINGLE_UNIT_1 ,
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE2 ,
        .pattern_num = 1 ,
        .sample_freq_hz = ad8232->sample_freg_hz,
    };
    // creat adc continous 
    res = adc_continuous_config(ad8232->adc_cHD, &adc_CF);  
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to config ADC: %s", esp_err_to_name(res));
        adc_continuous_del_handle(ad8232->adc_cHD);
        return error;
    }

    gpio_config_t io_IN = {
        .pin_bit_mask = (1Ull << ad8232->LO_1_pin) | (1ULL << ad8232->Lo_2_pin) ,
        .intr_type = GPIO_INTR_DISABLE ,
        .mode = GPIO_MODE_INPUT ,
        .pull_down_en = GPIO_PULLDOWN_DISABLE ,
        .pull_up_en   = GPIO_PULLUP_DISABLE   ,
    };
    gpio_config_t io_OUT = {
        .pin_bit_mask = ( 1ULL << ad8232->SDN_pin),
        .mode = GPIO_MODE_OUTPUT ,
        .intr_type = GPIO_INTR_DISABLE ,
        .pull_down_en = GPIO_PULLDOWN_DISABLE ,
        .pull_up_en   = GPIO_PULLUP_DISABLE   ,
    };

    res = gpio_config(&io_IN);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to config input GPIO: %s", esp_err_to_name(res));
        adc_continuous_del_handle(ad8232->adc_cHD);
        return error;
    }

    res = gpio_config(&io_OUT);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to config output GPIO: %s", esp_err_to_name(res));
        adc_continuous_del_handle(ad8232->adc_cHD);
        return error;
    }

    res = adc_continuous_start(ad8232->adc_cHD);  // FIX: start ADC
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start ADC: %s", esp_err_to_name(res));
        adc_continuous_del_handle(ad8232->adc_cHD);
        return error;
    }
    
    
    if(ad8232_isEnable(ad8232) ==disable) return disable ;

    return success;
}

AD8232_status ad8232_isEnable(AD8232_t *ad8232){
    /* |  Lo_1   |   Lo_2   |  SDN 
     * |    1    |     1    |   1
     * |    1    |     0    |   0
     * |    0    |     1    |   0
     * |    0    |     0    |   0
    */
    if( gpio_get_level(ad8232->LO_1_pin) ==1 && gpio_get_level(ad8232->Lo_2_pin) ==1 ) {

        gpio_set_level(ad8232->SDN_pin, 1);
        return enable;

    } else {
        gpio_set_level(ad8232->SDN_pin , 0);
        return disable;
    }
}

AD8232_status ad8232_deinit(AD8232_t *ad8232) {
    esp_err_t res = adc_continuous_stop(ad8232->adc_cHD);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop ADC: %s", esp_err_to_name(res));
        return error;
    }
    
    res = adc_continuous_deinit(ad8232->adc_cHD);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to deinit sensor: %s", esp_err_to_name(res));
        return error;
    }
    
    return success;
}


AD8232_status ad8232_read(AD8232_t *ad8232 ,uint8_t* buffer,uint32_t timeout_wait ) {
    // lock mutex
    if (xSemaphoreTake(ad8232->mutex, pdMS_TO_TICKS(timeout_wait)) == pdFALSE) {
        ESP_LOGE(TAG, "Failed to acquire mutex");
        return busy;
    }

    if (ad8232 == NULL || buffer == NULL) {
        ESP_LOGE(TAG, "Invalid parameters");
        return error;
    }
    
    // Đọc từ ADC continuous
    uint32_t out_length = 0;
    esp_err_t res = adc_continuous_read(
        ad8232->adc_cHD,
        buffer,
        256,  // Đọc tối đa 256 bytes
        &out_length,
        timeout_wait
    );
    
    if (res == ESP_ERR_TIMEOUT) {
        ESP_LOGW(TAG, "Read timeout");
        return time_out;
    } else if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read ADC: %s", esp_err_to_name(res));
        return error;
    }
    
    ESP_LOGI(TAG, "Read %lu bytes", out_length);
    //free mutex
    xSemaphoreGive(ad8232->mutex);
    return success;
}