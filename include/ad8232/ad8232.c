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

AD8232_falgs_t ad8232_init(AD82832_t *ad8232) {

    if ( ad8232 == NULL ) {
        ESP_LOGE(TAG , "ad8232 is null "); 
        return error;
    }

    adc_continuous_handle_cfg_t adc_nHDCF = {
    .conv_frame_size = 256,
    .max_store_buf_size = 1024,
    };

    esp_err_t res =  adc_continuous_new_handle(&adc_nHDCF , &ad8232->adc_cHD );
    if (res =! ESP_OK ) {
        ESP_LOGE(TAG , "Failed to create new adc_c handle",esp_err_to_name(res));
        return error ;
    } ESP_LOGE(TAG, "Creat new adc_c handle sucess");

    adc_digi_pattern_config_t adc_pattern_CF = {
        .atten = ADC_ATTEN_DB_12,
        .bit_width = ADC_BITWIDTH_12,
        .unit = ADC_UNIT_1,
        .channel = ADC_CHANNEL_1
    };

    adc_continuous_config_t adc_CF = {
        .adc_pattern = &adc_pattern_CF ,
        .conv_mode  = ADC_CONV_SINGLE_UNIT_1 ,
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE2 ,
        .pattern_num = 1 ,
        .sample_freq_hz = 10000,
    };

    // creat adc continous 
    adc_continuous_config(ad8232->adc_cHD, &adc_CF);
    
}