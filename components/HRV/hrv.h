/*
 * @file : hrv.h 
 * @brief : Hear rate variability header  
 * @author : Nguyen Hoang
 * Using Pan-Tompkins althorgim 
 * 
*/

#pragma once

#ifndef hrv_h
#define hrv_h

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  @brief HRV feature 
*/

typedef struct {
    float mean_rr;               // RR average ( time to Ri to Rj ) (ms)
    float sdnn;                  // Standard Deviation of NN intervals (ms)
    float rmssd;                 // Root Mean Square of Successive Differences (ms)
    float pnn50;                 // Percentage of successive RR intervals that differ by more than 50 ms (%)
    float heart_rate;            // Hear rate avg follow hrv (BPM)
    float nn50_count;            // Number of pairs of successive NN intervals that differ by more than 50 ms
} HRV_features_t;

typedef struct {
    uint16_t *peaks;             // Indices of R-peaks in array
    uint16_t peak_count;         // Number of peaks
    float heart_rate;            // follow ecg
} ECG_peaks_t;

typedef struct {
    uint16_t sample_rate;        // Sampling frequency (Hz) - ex 250Hz
    uint16_t window_size;        // Windown size for detect peak
    float threshold_factor;      
    float min_peak_distance_ms;  // minimum distan of  2 peak (ms)
} ECG_config_t;


/**
 * @brief Detect R-peak of ECG raw value
 * 
 * @param ecg_data       array ECG ( ADC raw value )
 * @param data_length    number sample
 * @param config         ecg config 
 * @param peaks_out      Struct peak detection
 * 
 * @return true if success fale if failed
*/
bool ecg_detect_peaks(
    const int16_t *ecg_data,
    uint16_t data_length,
    const ECG_config_t *config,
    ECG_peaks_t *peaks_out
);

/**
 * @brief Calculate HRV features R-peak
 * 
 * @param peaks          Mảng chỉ số R-peak
 * @param peak_count     R-peak number
 * @param sample_rate     (Hz)
 * @param features_out   Struct chứa kết quả HRV features
 * 
 * @return bool
*/
bool ecg_extract_hrv_features(
    const uint16_t *peaks,
    uint16_t peak_count,
    uint16_t sample_rate,
    HRV_features_t *features_out
);






#ifdef __cplusplus
}
#endif

#endif