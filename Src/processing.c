#include "processing.h"
#include "max30102.h"
#include "sdcard.h"

bool fingerDetected = false;
float lowPassOutput = 0.0;
float highPassOutput = 0.0;
float previousValue = 0.0;
int sampleCount = 0;
float movingAverageBuffer[K_AVERAGING_SAMPLES];
int movingAverageIndex = 0;

// Function to detect if a finger is placed on the sensor
bool detectFinger(float sample) {
    if (sample > FINGER_THRESHOLD_VALUE) {  // Assuming 500 is the threshold for finger detection
        fingerDetected = true;
    } else {
        fingerDetected = false;
        resetFilters();  // Reset filters when no finger is detected
    }
    return fingerDetected;
}

// Reset filters and relevant values
void resetFilters() {
    lowPassOutput = 0.0;
    highPassOutput = 0.0;
    sampleCount = 0;
    movingAverageIndex = 0;
    for (int i = 0; i < K_AVERAGING_SAMPLES; i++) {
        movingAverageBuffer[i] = 0.0;
    }
}

// Low-pass filter
float lowPassFilter(float input) {
    static float prevOutput = 0.0;
    float alpha = exp(-2 * M_PI * K_LOW_PASS_CUTOFF / SAMPLING_RATE);  // 400 Hz sample rate
    float output = alpha * prevOutput + (1 - alpha) * input;
    prevOutput = output;
    return output;
}

// High-pass filter
float highPassFilter(float input) {
    static float prevInput = 0.0;
    static float prevOutput = 0.0;
    float alpha = exp(-2 * M_PI * K_HIGH_PASS_CUTOFF / SAMPLING_RATE);  // 400 Hz sample rate
    float output = alpha * (prevOutput + input - prevInput);
    prevInput = input;
    prevOutput = output;
    return output;
}

// Differentiate the signal
float differentiate(float currentValue) {
    float slope = (currentValue - previousValue) * SAMPLING_RATE;  // 400 Hz sample rate
    previousValue = currentValue;
    return slope;
}

// Detect peaks using zero-crossing detection
bool detectHeartBeat(float slope) {
    static bool wasPositive = false;
    bool heartBeatDetected = false;

    // Check for zero-crossing from positive to negative
    if (wasPositive && slope < 0 && fabs(slope) > K_EDGE_THRESHOLD) {
        heartBeatDetected = true;
    }

    wasPositive = (slope > 0);
    return heartBeatDetected;
}

// Calculate heart rate using time differences between peaks
float calculateHeartRate(int timeDifferenceMs) {
    return 60000.0 / timeDifferenceMs;  // Convert milliseconds to beats per minute
}

// Moving average to smooth heart rate calculation
float movingAverage(float newSample) {
    movingAverageBuffer[movingAverageIndex] = newSample;
    movingAverageIndex = (movingAverageIndex + 1) % K_AVERAGING_SAMPLES;

    float sum = 0.0;
    for (int i = 0; i < K_AVERAGING_SAMPLES; i++) {
        sum += movingAverageBuffer[i];
    }
    return sum / K_AVERAGING_SAMPLES;
}

void MAX30102_ReadFIFOData(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart, uint32_t *red_led, uint32_t *ir_led) {
    uint8_t fifo_data[6];
    uint8_t data_addr = MAX30102_REG_FIFO_DATA;
    // Read 6 bytes from the FIFO Data Register
    HAL_I2C_Mem_Read(hi2c, MAX30102_I2C_ADDRESS, data_addr, 1, fifo_data, 6, HAL_MAX_DELAY);
    *red_led = (fifo_data[0] << 16) | (fifo_data[1] << 8) | fifo_data[2];
    *ir_led = (fifo_data[3] << 16) | (fifo_data[4] << 8) | fifo_data[5];
}

// Function to display the heart rate using UART
void displayHeartRate(float heartRate,UART_HandleTypeDef *huart ) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Heart Rate: %.2f BPM\r\n", heartRate);
    HAL_UART_Transmit(huart, (uint8_t*)buffer, strlen(buffer), 1000);
}


// Main function that reads sensor data and processes it for heart rate detection
uint32_t Process_Max30102A_Data(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart) {
    static uint32_t lastBeatTime = 0;
    static int sampleCount = 0;
    uint32_t heartrate = 0;


    uint32_t red_led = 0;
    uint32_t ir_led = 0;

    // Read the Red and IR LED values from the MAX30102 FIFO register
    MAX30102_ReadFIFOData(hi2c, huart, &red_led, &ir_led);

    // Ensure the sample read is valid
    if (ir_led <= 0) {
        return 0;  // Invalid or no data read
    }

    // Get the current time in milliseconds
    int currentTime = HAL_GetTick();

    // Detect if the finger is placed on the sensor using the IR LED value
    if (detectFinger((float)ir_led)) {
        // Apply filters
        float lowPassed = lowPassFilter((float)ir_led);
        float highPassed = highPassFilter(lowPassed);

        // Differentiate the signal
        float slope = differentiate(highPassed);

        // Detect heartbeat
        if (detectHeartBeat(slope)) {
            // Calculate the time difference between heartbeats
            int timeDifferenceMs = currentTime - lastBeatTime;
            lastBeatTime = currentTime;

            // Ensure the time difference is positive
            if (timeDifferenceMs > 0) {
                // Calculate the heart rate
                float heartRate = calculateHeartRate(timeDifferenceMs);

                // Apply a moving average to smooth the heart rate
                if (sampleCount >= K_SAMPLE_THRESHOLD) {
                    heartRate = movingAverage(heartRate);
                    heartRate = 0.025*heartRate;//scaled to adjust the co-efficient
                    heartrate = (uint32_t)heartRate;
                    // Display the heart rate using UART
                    displayHeartRate(heartRate, huart);

                }
                sampleCount++;
            }
        }
    }
    return heartrate;
}

uint32_t Data_Sense(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart){
	 uint32_t red_led = 0;
	 uint32_t ir_led = 0;
	 MAX30102_ReadFIFOData(hi2c, huart, &red_led, &ir_led);
	 return ir_led;

}

uint32_t Data_Process(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart, uint32_t ir_led){

	float heartRate = 0.0;
	static uint32_t lastBeatTime = 0;
	static int sampleCount = 0;

	 // Ensure the sample read is valid
	    if (ir_led <= 0) {
	        return 0;  // Invalid or no data read
	    }

	    // Get the current time in milliseconds
	    int currentTime = HAL_GetTick();

	    // Detect if the finger is placed on the sensor using the IR LED value
	    if (detectFinger((float)ir_led)) {
	        // Apply filters
	        float lowPassed = lowPassFilter((float)ir_led);
	        float highPassed = highPassFilter(lowPassed);

	        // Differentiate the signal
	        float slope = differentiate(highPassed);

	        // Detect heartbeat
	        if (detectHeartBeat(slope)) {
	            // Calculate the time difference between heartbeats
	            int timeDifferenceMs = currentTime - lastBeatTime;
	            lastBeatTime = currentTime;

	            // Ensure the time difference is positive
	            if (timeDifferenceMs > 0) {
	                // Calculate the heart rate
	                float heartRate = calculateHeartRate(timeDifferenceMs);

	                // Apply a moving average to smooth the heart rate
	                if (sampleCount >= K_SAMPLE_THRESHOLD) {
	                    heartRate = movingAverage(heartRate);
	                    heartRate = 0.025*heartRate;//scaled to adjust the co-efficient
	                    heartRate = (uint32_t) heartRate;
	                    // Display the heart rate using UART
	                    displayHeartRate(heartRate, huart);

	                }
	                sampleCount++;
	            }
	        }
	    }
	    return heartRate;

}


