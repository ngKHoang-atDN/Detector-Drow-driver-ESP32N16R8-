/*
 * ESP-IDF 6.0.1 with platformIO
 * 
 * Board  : Esp32-S3-devkitc-N16R8
 * 
 * Sensor :
 *      + AD8232 ECG Distributor 
 *      + MAX30102 
 *      + diode led 
 *      
 * 
 * 
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_continuous.h"


void app_main(void){
    // define handle
    adc_continuous_handle_t adc_nHD;
    // config handle
    adc_continuous_handle_cfg_t adc_nHDCF = {
        .conv_frame_size = 256,
        .max_store_buf_size = 1024,
    };
    // create a new handle 
    adc_continuous_new_handle(&adc_nHDCF,&adc_nHD);

    // config pattern ADC 
    adc_digi_pattern_config_t adc_pattern_CF = {
        .atten = ADC_ATTEN_DB_12,
        .bit_width = ADC_BITWIDTH_12,
        .unit = ADC_UNIT_1,
        .channel = ADC_CHANNEL_6
    };
    // adc continous config 
    adc_continuous_config_t adc_CF = {
        .adc_pattern = &adc_pattern_CF ,
        .conv_mode  = ADC_CONV_SINGLE_UNIT_1 ,
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE2 ,
        .pattern_num = 1 ,
        .sample_freq_hz = 10000,
    };
    // creat adc continous 
    adc_continuous_config(adc_nHD, &adc_CF);

    uint8_t buffer[256];
    uint32_t ret_num ;

    adc_continuous_start(adc_nHD);
    for(;;) {
        adc_continuous_read(adc_nHD,buffer,sizeof(buffer),&ret_num,100);
        adc_digi_output_data_t *p = (adc_digi_output_data_t *)buffer;

        uint32_t num_samples = ret_num / sizeof(adc_digi_output_data_t);

        for (int i = 0; i < num_samples; i++)
        {
            int value = p[i].type2.data;

            printf("CH%d = %d\n", value);
        }
    }

}