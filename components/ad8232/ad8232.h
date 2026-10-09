/**
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




/* One DMA frame (bytes). Must be a multiple of SOC_ADC_DIGI_RESULT_BYTES (4). */
#define AD8232_FRAME_BYTES      256
/* Driver pool = 4 frames, i.e. tolerance for a late reader. */
#define AD8232_POOL_BYTES       2048
/* LOD+/LOD- go HIGH when an electrode is disconnected (AD8232 datasheet). */
#define AD8232_LEADS_OFF_L 0


#ifdef __cplusplus
extern "C" {
#endif

// flag for AD8232
typedef enum {
    AD8232_OK = 0,
    AD8232_ERR_INVALID_ARG,
    AD8232_ERR_INVALID_STATE,
    AD8232_ERR_NO_MEM,
    AD8232_ERR_BUSY,        /* could not get the mutex in time            */
    AD8232_ERR_TIMEOUT,     /* no ADC data within the timeout             */
    AD8232_ERR_OVERRUN,     /* driver pool overflowed: gap in the stream  */
    AD8232_ERR_LEADS_OFF,   /* electrode disconnected, data discarded     */
    AD8232_ERR_ADC,
    AD8232_ERR_GPIO,
} AD8232_status;


/**
 *   @brief : Struct config for ADC continuous 
 *      
 *   @param channel : channel you want to read value 
 *   @param sample freg hz : The expected ADC sampling frequency in Hz  
 *   @param Lo_plus_pin : GPIO pin of Lo+
 *   @param Lo_minus_pin : GPIO pin of Lo-
 *   @param SDN_pin  : GPIO pin of SDN 
*/
typedef struct {
    adc_channel_t channel;      /* ADC1 channel, e.g. ADC_CHANNEL_6                 */
    uint32_t sample_freq_hz;    /* S3 continuous mode: 611 .. 83333 Hz              */
    gpio_num_t lo_plus_pin;     /* LO+                                              */
    gpio_num_t lo_minus_pin;    /* LO-                                              */
    gpio_num_t sdn_pin;         /* SDN, active low shutdown (HIGH = chip running)   */
} AD8232_config_t;
 

typedef struct {
    AD8232_config_t cfg;
    adc_continuous_handle_t adc;
    SemaphoreHandle_t mutex;
    volatile uint32_t overrun_count;    /* written from ISR, read from tasks */
    bool running;                       /* protected by mutex                */
} AD8232_t;


/**
 * @brief  Fuction init ad8232 sensor
 * 
 * @param  *ad8232 : address of struct AD82832_t
 * @return flag Operation status
*/
AD8232_status ad8232_init(AD8232_t *ad8232, const AD8232_config_t *cfg);

/**
 * @brief Deinit ad8232 sensor and free resource
 * @param *ad8232 address of struct AD8232_t
 * @return flag operatio status
*/
AD8232_status ad8232_deinit(AD8232_t *ad8232 );

/** 
 * @brief check sensor is conected by lo_plus and lo_minus is conected in body 
 * @param *ad8232 : pointer of AD8232_t
 * @return  enable -> is running || disable -> is off
*/
bool ad8232_isConected(const AD8232_t *ad8232);

/**
 * @brief Turn on/off sensor by control SDN pin 
 */
AD8232_status ad8232_set_enabled(AD8232_t *ad8232, bool enabled);

/**
 * @brief Read raw ADC bytes.
 * @param buf        destination buffer
 * @param buf_size   size of @p buf in bytes (rounded down to a multiple of 4)
 * @param out_len    number of valid bytes written to @p buf (0 on error)
 * @param timeout_ms total budget for mutex + ADC wait
 */
AD8232_status ad8232_read(AD8232_t *ad8232, uint8_t *buf, uint32_t buf_size,uint32_t *out_len, uint32_t timeout_ms);

/** Convert raw bytes to 12-bit samples of this device's channel. Returns count. */
uint32_t ad8232_parse(const AD8232_t *ad8232, const uint8_t *raw, uint32_t raw_len,uint16_t *samples, uint32_t max_samples);
 
/** Number of pool overflows since init (each one is a gap in the ECG). */
static inline uint32_t ad8232_overrun_count(const AD8232_t *ad8232)
{
    return ad8232->overrun_count;
}


//AD8232_status printAD8232_val(AD82832_t *ad8232);

#ifdef __cplusplus 
}

#endif

#endif // AD8232