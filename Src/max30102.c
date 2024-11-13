#include "max30102.h"


bool MAX30102_Init(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart, uint16_t size, uint8_t* Data) {

    uint8_t value;
    HAL_StatusTypeDef status;


 //   *********************************Interrupt**********************************************************
    value = 0x01;  // Enable temperature ready interrupt only
       status = HAL_I2C_Mem_Write(hi2c, MAX30102_I2C_ADDRESS, MAX30102_REG_INT_ENABLE_2, 1, &value, 1, HAL_MAX_DELAY);
       if (status != HAL_OK) {
           size = sprintf((char *)Data, "Failed to enable temperature interrupt\r\n");
           HAL_UART_Transmit(huart, Data, size, 1000);
           return false;
       }


   // Enable temperature measurement
	  value = 0x01;  // Enable temperature measurement
	  status = HAL_I2C_Mem_Write(hi2c, MAX30102_I2C_ADDRESS, MAX30102_REG_TEMP_CONFIG, 1, &value, 1, HAL_MAX_DELAY);
	  if (status != HAL_OK) {
		  size = sprintf((char *)Data, "Failed to start temperature measurement\r\n");
		  HAL_UART_Transmit(huart, Data, size, 1000);
		  return false;
	  }


    // ***********************************Interrupt************************************************************

    value = 0x03;
    status = HAL_I2C_Mem_Write(hi2c, MAX30102_I2C_ADDRESS, MAX30102_REG_MODE_CONFIG, 1, &value, 1, HAL_MAX_DELAY);
    if (status != HAL_OK) {
    	size = sprintf((char *)Data, "MAX30102 write to SPO2 mode set failed\r\n");
		HAL_UART_Transmit(huart, Data, size, 1000);
		HAL_Delay(2000);
          return false;
    }

    value = 0x47;
    status = HAL_I2C_Mem_Write(hi2c, MAX30102_I2C_ADDRESS, MAX30102_REG_SPO2_CONFIG, 1, &value, 1, HAL_MAX_DELAY);
    if (status != HAL_OK) {
    	size = sprintf((char *)Data, "MAX30102 SPo2 mode specific parameters write failed\r\n");
		HAL_UART_Transmit(huart, Data, size, 1000);
		HAL_Delay(2000);

        return false;
    }


    value = 0x1F;
    status = HAL_I2C_Mem_Write(hi2c, MAX30102_I2C_ADDRESS, MAX30102_REG_LED1_PA, 1, &value, 1, HAL_MAX_DELAY);
    if (status != HAL_OK) {
        return false;
    }

    value = 0x1F;
    status = HAL_I2C_Mem_Write(hi2c, MAX30102_I2C_ADDRESS, MAX30102_REG_LED2_PA, 1, &value, 1, HAL_MAX_DELAY);
    if (status != HAL_OK) {
        return false;
    }

    return true;
}





void MAX30102_ReadRawData(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart, uint32_t *red_led, uint32_t *ir_led) {
    uint8_t fifo_data[6];
    uint8_t data_addr = MAX30102_REG_FIFO_DATA;
    char debug_msg[150];
    uint16_t size;
    // Read 6 bytes from the FIFO Data Register
    HAL_I2C_Mem_Read(hi2c, MAX30102_I2C_ADDRESS, data_addr, 1, fifo_data, 6, HAL_MAX_DELAY);
    *red_led = (fifo_data[0] << 16) | (fifo_data[1] << 8) | fifo_data[2];
    *ir_led = (fifo_data[3] << 16) | (fifo_data[4] << 8) | fifo_data[5];
    size = sprintf(debug_msg, "Red: %lu, IR: %lu\r\n", *red_led, *ir_led);
    HAL_UART_Transmit(huart, (uint8_t *)debug_msg, size, 1000);
}

void MAX30102_ReadData(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart, uint32_t *red_led, uint32_t *ir_led) {
    uint8_t fifo_data[6];
    uint8_t data_addr = MAX30102_REG_FIFO_DATA;
    // Read 6 bytes from the FIFO Data Register
    HAL_I2C_Mem_Read(hi2c, MAX30102_I2C_ADDRESS, data_addr, 1, fifo_data, 6, HAL_MAX_DELAY);
    *red_led = (fifo_data[0] << 16) | (fifo_data[1] << 8) | fifo_data[2];
    *ir_led = (fifo_data[3] << 16) | (fifo_data[4] << 8) | fifo_data[5];
}

//*********in case of external interrupt use the blue push button that tells the STM chip to read all the FIFO 32 samples once and then clear the interrupt********

void MAX30102_ReadMultipleSamples(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart, uint32_t *red_led, uint32_t *ir_led, uint8_t sample_count) {
    uint8_t fifo_data[6];
    for (int i = 0; i < sample_count; i++) {
        HAL_I2C_Mem_Read(hi2c, MAX30102_I2C_ADDRESS, MAX30102_REG_FIFO_DATA, 1, fifo_data, 6, HAL_MAX_DELAY);
        red_led[i] = (fifo_data[0] << 16) | (fifo_data[1] << 8) | fifo_data[2];
        ir_led[i] = (fifo_data[3] << 16) | (fifo_data[4] << 8) | fifo_data[5];
    }
}

//**********************reading temperature registers*************************
float MAX30102_ReadTemperature(I2C_HandleTypeDef *hi2c) {
    uint8_t temp_int, temp_frac;
    float temperature;
    HAL_I2C_Mem_Read(hi2c, MAX30102_I2C_ADDRESS, MAX30102_REG_TEMP_INT, 1, &temp_int, 1, HAL_MAX_DELAY);
    HAL_I2C_Mem_Read(hi2c, MAX30102_I2C_ADDRESS, MAX30102_REG_TEMP_FRAC, 1, &temp_frac, 1, HAL_MAX_DELAY);
    temperature = (float)temp_int + ((float)temp_frac * 0.0625);
    return temperature;
}

void Process_Max30102_Data(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart) {
    uint32_t red_led, ir_led;
    uint32_t ir_buffer[100], red_buffer[100];
    int32_t heart_rate;
    int8_t hr_valid;
    int32_t spo2;
    int8_t spo2_valid;
    int i = 0;


    for (i = 0; i < 100; i++) {
        MAX30102_ReadData(hi2c, huart, &red_led, &ir_led);
        ir_buffer[i] = ir_led;
        red_buffer[i] = red_led;
        HAL_Delay(20);
    }

    MAX30102_CalculateHeartRate(ir_buffer, red_buffer, 100, &heart_rate, &hr_valid, &spo2, &spo2_valid);

    // Output the heart rate and SpO2 values via UART
    char debug_msg[150];
    uint16_t size = sprintf(debug_msg, "Heart Rate: %ld bpm, SpO2: %ld%%\r\n", heart_rate, spo2);
    HAL_UART_Transmit(huart, (uint8_t *)debug_msg, size, 1000);
}

void MAX30102_CalculateHeartRate(uint32_t *ir_buffer, uint32_t *red_buffer, int32_t buffer_length, int32_t *heart_rate, int8_t *hr_valid, int32_t *spo2, int8_t *spo2_valid) {
    int32_t ir_mean = 0;
    int32_t ir_data[buffer_length];
    int32_t peak_locs[15];
    int32_t num_peaks = 0;
    int32_t peak_interval_sum = 0;


    for (int i = 0; i < buffer_length; i++) {
        ir_mean += ir_buffer[i];
    }
    ir_mean /= buffer_length;


    for (int i = 0; i < buffer_length; i++) {
        ir_data[i] = -1 * (ir_buffer[i] - ir_mean);
    }


    MAX30102_FindPeaks(ir_data, buffer_length, peak_locs, &num_peaks, 30, 4);

    // Calculate heart rate if we have enough peaks
    if (num_peaks >= 2) {
        for (int i = 1; i < num_peaks; i++) {
            peak_interval_sum += (peak_locs[i] - peak_locs[i - 1]);
        }
        peak_interval_sum /= (num_peaks - 1);
        *heart_rate = (int32_t)(60 * 100 / peak_interval_sum); // Simplified frequency calculation
        *hr_valid = 1;
    } else {
        *heart_rate = -999;
        *hr_valid = 0;
    }


}

void MAX30102_FindPeaks(int32_t *ir_data, int32_t length, int32_t *peak_locs, int32_t *num_peaks, int32_t min_height, int32_t min_distance) {
    int32_t i = 1, width;
    *num_peaks = 0;

    while (i < length - 1) {
        if (ir_data[i] > min_height && ir_data[i] > ir_data[i - 1]) {
            width = 1;
            while ((i + width) < length && ir_data[i] == ir_data[i + width])
                width++;
            if (ir_data[i] > ir_data[i + width]) {
                peak_locs[(*num_peaks)++] = i;
                i += width + 1;
            } else {
                i += width;
            }
        } else {
            i++;
        }
    }
}

