/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define key_3_Pin GPIO_PIN_2
#define key_3_GPIO_Port GPIOE
#define key_4_Pin GPIO_PIN_3
#define key_4_GPIO_Port GPIOE
#define key_5_Pin GPIO_PIN_4
#define key_5_GPIO_Port GPIOE
#define key_6_Pin GPIO_PIN_5
#define key_6_GPIO_Port GPIOE
#define Dswtich_1_Pin GPIO_PIN_6
#define Dswtich_1_GPIO_Port GPIOE
#define cross_key_SW1_Pin GPIO_PIN_0
#define cross_key_SW1_GPIO_Port GPIOC
#define cross_key_SW2_Pin GPIO_PIN_1
#define cross_key_SW2_GPIO_Port GPIOC
#define cross_key_SW3_Pin GPIO_PIN_2
#define cross_key_SW3_GPIO_Port GPIOC
#define cross_key_SW4_Pin GPIO_PIN_3
#define cross_key_SW4_GPIO_Port GPIOC
#define joystick_1_Pin GPIO_PIN_3
#define joystick_1_GPIO_Port GPIOA
#define joystick_2_Pin GPIO_PIN_4
#define joystick_2_GPIO_Port GPIOA
#define joystick_3_Pin GPIO_PIN_5
#define joystick_3_GPIO_Port GPIOA
#define joystick_4_Pin GPIO_PIN_6
#define joystick_4_GPIO_Port GPIOA
#define Dswtich_2_Pin GPIO_PIN_7
#define Dswtich_2_GPIO_Port GPIOE
#define Dswtich_3_Pin GPIO_PIN_8
#define Dswtich_3_GPIO_Port GPIOE
#define Dswtich_4_Pin GPIO_PIN_9
#define Dswtich_4_GPIO_Port GPIOE
#define Dswtich_5_Pin GPIO_PIN_10
#define Dswtich_5_GPIO_Port GPIOE
#define Dswtich_6_Pin GPIO_PIN_11
#define Dswtich_6_GPIO_Port GPIOE
#define Dswtich_7_Pin GPIO_PIN_12
#define Dswtich_7_GPIO_Port GPIOE
#define Dswtich_8_Pin GPIO_PIN_13
#define Dswtich_8_GPIO_Port GPIOE
#define RXSX1281_MD0_Pin GPIO_PIN_12
#define RXSX1281_MD0_GPIO_Port GPIOB
#define RXSX1281_MD1_Pin GPIO_PIN_13
#define RXSX1281_MD1_GPIO_Port GPIOB
#define RXSX1281_AUX_Pin GPIO_PIN_14
#define RXSX1281_AUX_GPIO_Port GPIOB
#define RXSX1281_AUX_EXTI_IRQn EXTI15_10_IRQn
#define Tswtich1_1_Pin GPIO_PIN_8
#define Tswtich1_1_GPIO_Port GPIOA
#define Tswtich1_2_Pin GPIO_PIN_9
#define Tswtich1_2_GPIO_Port GPIOA
#define Tswtich2_1_Pin GPIO_PIN_10
#define Tswtich2_1_GPIO_Port GPIOA
#define Tswtich2_2_Pin GPIO_PIN_11
#define Tswtich2_2_GPIO_Port GPIOA
#define TXSX1281_MD0_Pin GPIO_PIN_10
#define TXSX1281_MD0_GPIO_Port GPIOC
#define TXSX1281_MD1_Pin GPIO_PIN_11
#define TXSX1281_MD1_GPIO_Port GPIOC
#define TXSX1281_AUX_Pin GPIO_PIN_12
#define TXSX1281_AUX_GPIO_Port GPIOC
#define TXSX1281_AUX_EXTI_IRQn EXTI15_10_IRQn
#define Tswtich3_1_Pin GPIO_PIN_4
#define Tswtich3_1_GPIO_Port GPIOB
#define Tswtich3_2_Pin GPIO_PIN_5
#define Tswtich3_2_GPIO_Port GPIOB
#define Tswtich4_1_Pin GPIO_PIN_6
#define Tswtich4_1_GPIO_Port GPIOB
#define Tswtich4_2_Pin GPIO_PIN_7
#define Tswtich4_2_GPIO_Port GPIOB
#define key_1_Pin GPIO_PIN_0
#define key_1_GPIO_Port GPIOE
#define key_2_Pin GPIO_PIN_1
#define key_2_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
