/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "i2c.h"
#include "usart.h"
#include "max30102.h"
#include "processing.h"
#include "spo2.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "sdcard.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId SenseHandle;
osThreadId LogHandle;
osMessageQId queueHandle;
osSemaphoreId SemaphorebinaryHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void datasense(void const * argument);
void datalog(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
		StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
	*ppxIdleTaskStackBuffer = &xIdleStack[0];
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
	/* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of Semaphorebinary */
  osSemaphoreDef(Semaphorebinary);
  SemaphorebinaryHandle = osSemaphoreCreate(osSemaphore(Semaphorebinary), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of queue */
  osMessageQDef(queue, 1, uint32_t);
  queueHandle = osMessageCreate(osMessageQ(queue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of Sense */
  osThreadDef(Sense, datasense, osPriorityHigh, 0, 128);
  SenseHandle = osThreadCreate(osThread(Sense), NULL);

  /* definition and creation of Log */
  osThreadDef(Log, datalog, osPriorityNormal, 0, 128);
  LogHandle = osThreadCreate(osThread(Log), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_datasense */
/**
* @brief Function implementing the myTask01 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_datasense */
void datasense(void const * argument)
{
  /* USER CODE BEGIN datasense */

	 uint32_t heartRate = 0;

  /* Infinite loop */
  for(;;)
  {
	           myprintf("SENSE\r\n\n");
	           heartRate = Process_Max30102A_Data(&hi2c1, &huart2);
	           if (xQueueSend(queueHandle, &heartRate, portMAX_DELAY) == pdPASS) {
//	        	   xSemaphoreGive(SemaphorebinaryHandle);
	           }

	           myprintf("SENSE DONE\r\n\n");
	           osDelay(10);
//	           osThreadTerminate(NULL);
	           vTaskResume(LogHandle);
	          vTaskSuspend(NULL);

  }


  /* USER CODE END datasense */
}

/* USER CODE BEGIN Header_datalog */
/**
* @brief Function implementing the myTask02 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_datalog */
void datalog(void const * argument)
{
  /* USER CODE BEGIN datalog */
	uint32_t heart_rate;


  /* Infinite loop */
  for(;;)
  {
//	  if(xSemaphoreTake(SemaphorebinaryHandle, portMAX_DELAY) == pdTRUE){
	           myprintf("LOGG\r\n\n");
	           if (xQueueReceive(queueHandle, &heart_rate, portMAX_DELAY) == pdPASS) {
//	               myprintf("Received heartrate: %lu\r\n\n", heart_rate);
	               logToSDCard(heart_rate);
	           }
	           myprintf("LOGG DONE\r\n\n");
//	           osThreadTerminate(NULL);
	           vTaskResume(SenseHandle);
	           vTaskSuspend(NULL);
	  }



  /* USER CODE END datalog */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
