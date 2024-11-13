#include "spo2.h"
#include "max30102.h"
#include "processing.h"



MinMaxAvgStatistic redStats = { .min = NAN, .max = NAN, .sum = 0, .count = 0 };
MinMaxAvgStatistic irStats = { .min = NAN, .max = NAN, .sum = 0, .count = 0 };

// Function to update statistics
void updateStatistics(MinMaxAvgStatistic *stats, float value) {
    stats->min = isnan(stats->min) ? value : fmin(stats->min, value);
    stats->max = isnan(stats->max) ? value : fmax(stats->max, value);
    stats->sum += value;
    stats->count++;
}

float calculateDC(MinMaxAvgStatistic *stats) {
    return stats->sum / stats->count;
}

float calculateAC(MinMaxAvgStatistic *stats) {
    return stats->max - stats->min;
}

// Function to calculate SpO2
float calculateSpO2(float redAC, float redDC, float irAC, float irDC) {
	 if (redDC == 0 || irDC == 0) {
	        return NAN;  // Avoid division by zero
	    }
    float R = (redAC / redDC) / (irAC / irDC);
    // Calibration coefficients for MAX30101 sensor
//    float SpO2 = 110.0 - 25 * R;
    float SpO2 = (132.68 - 34.65 * R + 1.59*R*R);
    return SpO2;
}

// Reset statistics
void resetStatistics(MinMaxAvgStatistic *stats) {
    stats->min = NAN;
    stats->max = NAN;
    stats->sum = 0;
    stats->count = 0;
}

// Function to detect if a finger is placed on the sensor
bool detectFingers(float sample) {
    if (sample > FINGER_THRESHOLD_VALUE) {
        fingerDetected = true;
    } else {
        fingerDetected = false;
        resetFilterss();
        resetStatistics(&redStats);
        resetStatistics(&irStats);
    }
    return fingerDetected;
}

// Function to reset filters and relevant values
void resetFilterss() {
    lowPassOutput = 0.0;
    highPassOutput = 0.0;
    sampleCount = 0;
    movingAverageIndex = 0;
    for (int i = 0; i < K_AVERAGING_SAMPLES; i++) {
        movingAverageBuffer[i] = 0.0;
    }
}

// Main function to process MAX30102 data for heart rate and SpO2 detection
void Process_Max30102B_Data(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart) {
    static uint32_t lastBeatTime = 0;
    static int sampleCount = 0;

    uint32_t red_led = 0;
    uint32_t ir_led = 0;

    // Read the Red and IR LED values from the MAX30102 FIFO register
    MAX30102_ReadFIFOData(hi2c, huart, &red_led, &ir_led);

    // Ensure the sample read is valid
    if (ir_led <= 0 || red_led <= 0) {
        return;  // Invalid or no data read
    }

    // Get the current time in milliseconds
    int currentTime = HAL_GetTick();

    // Detect if the finger is placed on the sensor using the IR LED value
    if (detectFingers((float)ir_led)) {
         //Apply filters
        float lowPassedIR = lowPassFilter((float)ir_led);
        float highPassedIR = highPassFilter(lowPassedIR);
        float lowPassedRed = lowPassFilter((float)red_led);
//        float highPassedRed = highPassFilter(lowPassedRed);

        // Update the min, max, and average statistics for AC and DC component calculation
        updateStatistics(&redStats, (float)lowPassedRed);
        updateStatistics(&irStats, (float)lowPassedIR);

        // Differentiate the signal
        float slope = differentiate(highPassedIR);

        // Detect heartbeat
        if (detectHeartBeat(slope)) {
            // Calculate the time difference between heartbeats
            int timeDifferenceMs = currentTime - lastBeatTime;
            lastBeatTime = currentTime;
//
            // Ensure the time difference is positive
            if (timeDifferenceMs > 0) {
//                // Calculate the heart rate
//                float heartRate = calculateHeartRate(timeDifferenceMs);

                // Apply a moving average to smooth the heart rate
                if (sampleCount >= K_SAMPLE_THRESHOLD) {
//                    heartRate = movingAverage(heartRate);

                    // Calculate AC and DC components for both Red and IR LEDs
                    float redAC = calculateAC(&redStats);
                    float redDC = calculateDC(&redStats);
                    float irAC = calculateAC(&irStats);
                    float irDC = calculateDC(&irStats);

                    // Reset statistics after every beat
                    resetStatistics(&redStats);
                    resetStatistics(&irStats);

                    // Calculate SpO2 using the AC and DC components
                    float SpO2 = calculateSpO2(redAC, redDC, irAC, irDC);
                    char buffer[32];
                    snprintf(buffer, sizeof(buffer), "SpO2: %.2f%%\r\n", SpO2);
                    HAL_UART_Transmit(huart, (uint8_t *)buffer, strlen(buffer), 1000);
                }
                sampleCount++;
            }
        }
}
}


