/* 
 *  @file : library of AD8232 ecg senso
 *  @brief : provide API information for lib using adc contiunous
 *  @author : Nguyen Hoang
 *  @note FOR RTOS
*/
#pragma once
#ifndef ad8232_h
#define ad8232_h

#include <stdint.h>
// adc continuous component
#include <esp_adc/adc_continuous.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

// Define confug for ad8232 

#define ADC_channel     ADC_CHANNEL_6
#define ADC_unit        ADC_UNIT_1
#define ADC_atten       ADC_ATTEN_DB_12
#define ADC_bitWidth     ADC_BITWIDTH_12
#define ADC_freqHZ       100000


#ifdef __cplusplus
extern "C" {
#endif

// flag for AD8232
typedef enum {
    success =1,   
    error = 0 ,
    time_out = -1 ,
    busy = -2
}AD8232_falgs_t;

/*
 *   @brief : Struct config for ADC continuous 
 *      
 *   @param channel : channel you want to read value 
 *   @param sample freg hz : The expected ADC sampling frequency in Hz  
 *    
*/
typedef struct 
{
    adc_continuous_handle_t adc_cHD; 
    uint8_t channel  ;    // Config channel you want
    uint32_t sample_freg_hz  ;  // Config expecred ADC sampling frequency
    SemaphoreHandle_t mutex ;
    TaskHandle_t waitingTask ;
    
}AD82832_t;

/*
 * @brief  Fuction init ad8232 sensor
 * 
 * @param  *ad8232 : address of struct AD82832_t
 * @return flag Operation status
*/
AD8232_falgs_t ad8232_init(AD82832_t *ad8232);

/*
 * @brief Deinit ad8232 sensor and free resource
 * @param *ad8232 address of struct AD8232_t
 * @return flag operatio status
*/

AD8232_falgs_t ad8232_deinit(AD8232_falgs_t *ad8232);

/*
 * @brief Read ADC value from Sensor  
 * 
 * @param *ad8232 : address of struct  AD8232_t
 * @param buffer  : address array arrray which will conclude result of ADC continus. Access to esp_adc/adc_continuous.h, find " adc_continuous_read" to detail more
 * @param timeout_wait : the time to data wait this API in miliseconds. Defualt is 100 ms 
 * 
 * @return flag opration status
*/

AD8232_falgs_t ad8232_read(AD82832_t *ad8232 ,uint8_t* buffer,uint32_t timeout_wait );

//AD8232_falgs_t printAD8232_val(AD82832_t *ad8232);

#ifdef __cplusplus }

#endif

#endif // AD3232