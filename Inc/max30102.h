#ifndef MAX30102_H
#define MAX30102_H
#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <stdio.h>


#define MAX30102_I2C_ADDRESS      0x57 << 1


#define MAX30102_REG_FIFO_WR_PTR      0x04
#define MAX30102_REG_OVF_COUNTER      0x05
#define MAX30102_REG_FIFO_RD_PTR      0x06
#define MAX30102_REG_FIFO_DATA        0x07

#define MAX30102_REG_MODE_CONFIG      0x09
#define MAX30102_REG_SPO2_CONFIG      0x0A
#define MAX30102_REG_LED1_PA          0x0C
#define MAX30102_REG_LED2_PA          0x0D

#define MAX30102_REG_TEMP_CONFIG            0x21  // Temperature Configuration Register
#define MAX30102_REG_TEMP_INT               0x1F  // Temperature Integer Register
#define MAX30102_REG_TEMP_FRAC              0x20  // Temperature Fraction Register
#define MAX30102_REG_INT_ENABLE_2           0x03
#define MAX30102_REG_INT_STATUS_1 			0x00// Interrupt Enable 2 Register
  // Interrupt Status 2 Register

bool MAX30102_Init(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart,  uint16_t size, uint8_t* Data);  // to initilaize the sensor
void MAX30102_ReadRawData(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart, uint32_t *red_led, uint32_t *ir_led); // to read the data from register
void MAX30102_ReadMultipleSamples(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart, uint32_t *red_led, uint32_t *ir_led, uint8_t sample_count);
float MAX30102_ReadTemperature(I2C_HandleTypeDef *hi2c);



void Process_Max30102_Data(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart);
void MAX30102_CalculateHeartRate(uint32_t *ir_buffer, uint32_t *red_buffer, int32_t buffer_length, int32_t *heart_rate, int8_t *hr_valid, int32_t *spo2, int8_t *spo2_valid);
void MAX30102_FindPeaks(int32_t *ir_data, int32_t length, int32_t *peak_locs, int32_t *num_peaks, int32_t min_height, int32_t min_distance);



#endif
