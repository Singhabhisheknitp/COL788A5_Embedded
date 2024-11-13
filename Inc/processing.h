#ifndef HEART_RATE_PROCESSOR_H
#define HEART_RATE_PROCESSOR_H

#include <math.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include <string.h>


// Constants for filtering and detection (using #define preprocessor directives)
#define K_LOW_PASS_CUTOFF 5.0    // Low-pass filter cutoff frequency (Hz)
#define K_HIGH_PASS_CUTOFF 0.5   // High-pass filter cutoff frequency (Hz)
#define K_EDGE_THRESHOLD 0.5     // Threshold for detecting peaks
#define K_FINGER_COOLDOWN_MS 500 // Cooldown period after finger detection (ms)
#define K_AVERAGING_SAMPLES 50   // Number of samples for moving average
#define K_SAMPLE_THRESHOLD 20
#define FINGER_THRESHOLD_VALUE 1000
#define SAMPLING_RATE 400// Threshold for displaying first results

// Variables for state management (externally declared in .c file)
extern bool fingerDetected;
extern float lowPassOutput;
extern float highPassOutput;
extern float previousValue;
extern int sampleCount;
extern float movingAverageBuffer[K_AVERAGING_SAMPLES];
extern int movingAverageIndex;

// Function prototypes
bool detectFinger(float sample);
void resetFilters();
float lowPassFilter(float input);
float highPassFilter(float input);
float differentiate(float currentValue);
bool detectHeartBeat(float slope);
float calculateHeartRate(int timeDifferenceMs);
float movingAverage(float newSample);
void MAX30102_ReadFIFOData(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart, uint32_t *red_led, uint32_t *ir_led);
uint32_t Process_Max30102A_Data(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart);
void displayHeartRate(float heartRate, UART_HandleTypeDef *huart );
uint32_t Data_Sense(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart);
uint32_t Data_Process(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart, uint32_t ir_led);


#endif // HEART_RATE_PROCESSOR_H
