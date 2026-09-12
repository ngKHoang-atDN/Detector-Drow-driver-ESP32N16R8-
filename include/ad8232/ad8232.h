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
#define ADC_freqHZ       10000


#ifdef __cplusplus
extern "C" {
#endif

// flag for AD8232
typedef enum : uint8_t  {
    enable      = 1  ,
    disable     = 0  ,
    success     = 2  ,   
    error       = 3  ,
    time_out    = 4  ,
    busy        = 5  ,
} AD8232_status;


/*
 *   @brief : Struct config for ADC continuous 
 *      
 *   @param channel : channel you want to read value 
 *   @param sample freg hz : The expected ADC sampling frequency in Hz  
 *   @param Lo_1_pin : GPIO pin of Lo+
 *   @param Lo_2_pin : GPIO pin of Lo-
 *   @param SDN_pin  : GPIO pin of SDN 
*/
typedef struct 
{
    adc_continuous_handle_t adc_cHD; 
    uint8_t channel_ADC  ;    // Config  adc channel you want
    uint16_t sample_freg_hz  ;  // Config expecred ADC sampling frequency
    uint8_t LO_1_pin  ;         
    uint8_t Lo_2_pin  ;
    uint8_t SDN_pin   ;        // shut down drive gpio pin 
    SemaphoreHandle_t mutex ;
    TaskHandle_t waitingTask ;
    
}AD8232_t;

/*
 * @brief  Fuction init ad8232 sensor
 * 
 * @param  *ad8232 : address of struct AD82832_t
 * @return flag Operation status
*/
AD8232_status ad8232_init(AD8232_t *ad8232);

/*
 * @brief Deinit ad8232 sensor and free resource
 * @param *ad8232 address of struct AD8232_t
 * @return flag operatio status
*/
AD8232_status ad8232_deinit(AD8232_t *ad8232);

/*
 * @brief check sensor is have full condition to read if not this fuction will turn off sensor
 * @param *ad8232 : pointer of AD8232_t
 * @return flags operation enable -> is running || disable -> is off
*/
AD8232_status ad8232_isEnable(AD8232_t *ad8232);

/*
 * @brief Read ADC value from Sensor  
 * 
 * @param *ad8232 : address of struct  AD8232_t
 * @param buffer  : address array arrray which will conclude result of ADC continus. Access to esp_adc/adc_continuous.h, find " adc_continuous_read" to detail more
 * @param timeout_wait : the time to data wait this API in miliseconds. Defualt is 100 ms 
 * 
 * @return flag opration status
*/
AD8232_status ad8232_read(AD8232_t *ad8232 ,uint8_t* buffer,uint32_t timeout_wait );

//AD8232_status printAD8232_val(AD82832_t *ad8232);

#ifdef __cplusplus }

#endif

#endif // AD3232