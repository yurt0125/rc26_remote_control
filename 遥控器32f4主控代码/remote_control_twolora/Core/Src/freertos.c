/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "adc.h"
#include "communication.h"
#include "gpio_button.h"
#include "joystick.h"
#include "adc_switch.h"
#include "tjc_huart_hmi.h"
#include "stm32f4xx.h"
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
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Display */
osThreadId_t DisplayHandle;
const osThreadAttr_t Display_attributes = {
  .name = "Display",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Button */
osThreadId_t ButtonHandle;
const osThreadAttr_t Button_attributes = {
  .name = "Button",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for RXComunication */
osThreadId_t RXComunicationHandle;
const osThreadAttr_t RXComunication_attributes = {
  .name = "RXComunication",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for TxBufferToDMA */
osThreadId_t TxBufferToDMAHandle;
const osThreadAttr_t TxBufferToDMA_attributes = {
  .name = "TxBufferToDMA",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void Display_task(void *argument);
void Button_task(void *argument);
void RXComunication_task(void *argument);
void TxBufferToDMA_task(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

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

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of Display */
  DisplayHandle = osThreadNew(Display_task, NULL, &Display_attributes);

  /* creation of Button */
  ButtonHandle = osThreadNew(Button_task, NULL, &Button_attributes);

  /* creation of RXComunication */
  RXComunicationHandle = osThreadNew(RXComunication_task, NULL, &RXComunication_attributes);

  /* creation of TxBufferToDMA */
  TxBufferToDMAHandle = osThreadNew(TxBufferToDMA_task, NULL, &TxBufferToDMA_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_Display_task */
/**
* @brief Function implementing the Display thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Display_task */
void Display_task(void *argument)
{
  /* USER CODE BEGIN Display_task */
  HMI_Task_Init(&huart2);
  /* Infinite loop */
  for(;;)
  {
    HMI_Task_Loop();
    osDelay(1);
  }
  /* USER CODE END Display_task */
}

/* USER CODE BEGIN Header_Button_task */
/**
* @brief Function implementing the Button thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Button_task */
void Button_task(void *argument)
{
  /* USER CODE BEGIN Button_task */
  Joystick_Task_Init(&hadc2);
  Key_Task_Init(&hadc1);
  Button_Task_Init();
  /* Infinite loop */
  for(;;)
  {
    Joystick_Task_Loop();
    Key_Task_Loop();
    Button_Task_Loop();
//    Communication_Task_Loop();
    osDelay(1);
  }
  /* USER CODE END Button_task */
}

/* USER CODE BEGIN Header_RXComunication_task */
/**
* @brief Function implementing the RXComunication thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_RXComunication_task */
void RXComunication_task(void *argument)
{
  /* USER CODE BEGIN RXComunication_task */
  /* Infinite loop */
  Communication_Task_Init(&huart6, &huart3);
  for(;;)
  {
    Communication_Task_Loop();
    osDelay(1);
  }
  /* USER CODE END RXComunication_task */
}

/* USER CODE BEGIN Header_TxBufferToDMA_task */
/**
* @brief Function implementing the TxBufferToDMA thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_TxBufferToDMA_task */
void TxBufferToDMA_task(void *argument)
{
  /* USER CODE BEGIN TxBufferToDMA_task */
  /* Infinite loop */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; 
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  Comm_Timer_Callback_Wrapper();
	TxBufferToDMA(&huart6);
  for(;;)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    TxBufferToDMA(&huart6);
    
    // //先拿当前拍数
    // uint32_t current_cyccnt = DWT->CYCCNT;
    // // 凭借无符号数减法的特性，即使 current 折返归零了，减去 last_tx 依然是准确的差值跑过的拍数！
    // tx_stamp = (current_cyccnt - last_tx_stamp) / 168; 
    // // 更新上一拍
    // last_tx_stamp = current_cyccnt;
  }
  /* USER CODE END TxBufferToDMA_task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

