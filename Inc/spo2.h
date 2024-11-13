#ifndef MAX30102_PROCESSING_H
#define MAX30102_PROCESSING_H

#include <math.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdio.h>  // for snprintf

// Constants for filtering and detection (using #define preprocessor directives)
#define K_LOW_PASS_CUTOFF 5.0    // Low-pass filter cutoff frequency (Hz)
#define K_HIGH_PASS_CUTOFF 0.5   // High-pass filter cutoff frequency (Hz)
#define K_EDGE_THRESHOLD 0.5     // Threshold for detecting peaks
#define K_FINGER_COOLDOWN_MS 500 // Cooldown period after finger detection (ms)
#define K_AVERAGING_SAMPLES 50   // Number of samples for moving average
#define K_SAMPLE_THRESHOLD 20    // Threshold for displaying first results
#define FINGER_THRESHOLD_VALUE 1000 // Threshold value for finger detection
#define SAMPLING_RATE 400        // Sampling rate in Hz

// Structure for Min, Max, and Average statistics
typedef struct {
    float min;
    float max;
    float sum;
    int count;
} MinMaxAvgStatistic;

// External variables for statistics (declared in .c file)
extern MinMaxAvgStatistic redStats;
extern MinMaxAvgStatistic irStats;

// External state variables (declared in .c file)
extern bool fingerDetected;
extern float lowPassOutput;
extern float highPassOutput;
extern float previousValue;
extern int sampleCount;
extern float movingAverageBuffer[K_AVERAGING_SAMPLES];
extern int movingAverageIndex;

// Function prototypes
void updateStatistics(MinMaxAvgStatistic *stats, float value);
float calculateDC(MinMaxAvgStatistic *stats);
float calculateAC(MinMaxAvgStatistic *stats);
float calculateSpO2(float redAC, float redDC, float irAC, float irDC);
void resetStatistics(MinMaxAvgStatistic *stats);
bool detectFingers(float sample);
void resetFilterss();
//void MAX30102B_ReadFIFODatas(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart, uint32_t *red_led, uint32_t *ir_led);
void Process_Max30102B_Data(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart);
float lowPassFilter(float input);
float highPassFilter(float input);
float differentiate(float currentValue);
bool detectHeartBeat(float slope);
float calculateHeartRate(int timeDifferenceMs);
float movingAverage(float newSample);
void displayHeartRate(float heartRate, UART_HandleTypeDef *huart);

#endif // MAX30102_PROCESSING_H
