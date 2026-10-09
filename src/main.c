/*
 * ESP-IDF 6.0.1 with platformIO
 * 
 * Board  : Esp32-S3-devkitc-N16R8
 * 
 * Sensor :
 *      + AD8232 ECG Distributor 
 *      + diode led 
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <esp_err.h>
#include <esp_log.h>
#include <inttypes.h> 
#include "ad8232.h"

#define channel_adc ADC_CHANNEL_6 // GPIO num 7 ( esp 32 s3 n16r8)
#define minus GPIO_NUM_10
#define plus GPIO_NUM_11
#define sdn GPIO_NUM_9
#define freq 2000

static AD8232_t ad8232 ;
static const char *TAG = "MAIN";


void ad8232_register() {
    const AD8232_config_t ad8232_cfg = {
        .channel = channel_adc,
        .lo_minus_pin = minus,
        .lo_plus_pin = plus,
        .sample_freq_hz = freq,
        .sdn_pin = sdn 
    };

    AD8232_status st = ad8232_init(&ad8232, &ad8232_cfg);
    if (st != AD8232_OK) {
        ESP_LOGE(TAG, "ad8232_init failed, status=%d", (int)st);
    }
}

static void ECG(void *pvparameter) {
    uint8_t  raw[AD8232_FRAME_BYTES];                 /* 256 bytes */
    uint16_t samples[AD8232_FRAME_BYTES / 4];         /* 64 samples */
    uint32_t len = 0;

    uint32_t sum = 0;          /* sum of the samples in the current group */
    uint32_t count = 0;        /* how many samples are in the group */
    bool     leads_off = false;
    uint32_t last_overruns = 0;

    while (1) {
        AD8232_status st = ad8232_read(&ad8232, raw, sizeof(raw), &len, 100);

        if (st == AD8232_OK) {
            if (leads_off) {
                leads_off = false;
                ESP_LOGI(TAG, "Electrodes connected");
            }

            uint32_t n = ad8232_parse(&ad8232, raw, len, samples,
                                      sizeof(samples) / sizeof(samples[0]));

            /* Average every DECIMATE samples, then print one number */
            for (uint32_t i = 0; i < n; i++) {
                sum += samples[i];
                count++;
                if (count == 4) {
                    printf("%" PRIu32 "\n", sum / 4);
                    sum = 0;
                    count = 0;
                }
            }
        } else if (st == AD8232_ERR_LEADS_OFF) {
            if (!leads_off) {                       /* print only when it changes */
                leads_off = true;
                ESP_LOGW(TAG, "Electrode is off");
            }
            sum = 0;
            count = 0;
        } else if (st == AD8232_ERR_TIMEOUT) {
            ESP_LOGW(TAG, "No data within timeout");
        } else {
            ESP_LOGE(TAG, "Read failed, status=%d", (int)st);
            vTaskDelay(pdMS_TO_TICKS(100));         /* avoid a tight error loop */
        }

        /* Report lost data (reader too slow) only when the counter changes */
        uint32_t overruns = ad8232_overrun_count(&ad8232);
        if (overruns != last_overruns) {
            ESP_LOGW(TAG, "Overruns so far: %" PRIu32, overruns);
            last_overruns = overruns;
        }
    }

}

void app_main(void){ 
    ad8232_register();
    xTaskCreate(ECG , "ecg_task",4096 , NULL,5,NULL);
}