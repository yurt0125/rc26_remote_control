/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, RXSX1281_MD0_Pin|RXSX1281_MD1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, TXSX1281_MD0_Pin|TXSX1281_MD1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : key_3_Pin key_4_Pin key_5_Pin key_6_Pin
                           Dswtich_1_Pin Dswtich_2_Pin Dswtich_3_Pin Dswtich_4_Pin
                           Dswtich_5_Pin Dswtich_6_Pin Dswtich_7_Pin Dswtich_8_Pin
                           key_1_Pin key_2_Pin */
  GPIO_InitStruct.Pin = key_3_Pin|key_4_Pin|key_5_Pin|key_6_Pin
                          |Dswtich_1_Pin|Dswtich_2_Pin|Dswtich_3_Pin|Dswtich_4_Pin
                          |Dswtich_5_Pin|Dswtich_6_Pin|Dswtich_7_Pin|Dswtich_8_Pin
                          |key_1_Pin|key_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : cross_key_SW1_Pin cross_key_SW2_Pin cross_key_SW3_Pin cross_key_SW4_Pin */
  GPIO_InitStruct.Pin = cross_key_SW1_Pin|cross_key_SW2_Pin|cross_key_SW3_Pin|cross_key_SW4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : RXSX1281_MD0_Pin RXSX1281_MD1_Pin */
  GPIO_InitStruct.Pin = RXSX1281_MD0_Pin|RXSX1281_MD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : RXSX1281_AUX_Pin */
  GPIO_InitStruct.Pin = RXSX1281_AUX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(RXSX1281_AUX_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Tswtich1_1_Pin Tswtich1_2_Pin Tswtich2_1_Pin Tswtich2_2_Pin */
  GPIO_InitStruct.Pin = Tswtich1_1_Pin|Tswtich1_2_Pin|Tswtich2_1_Pin|Tswtich2_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : TXSX1281_MD0_Pin TXSX1281_MD1_Pin */
  GPIO_InitStruct.Pin = TXSX1281_MD0_Pin|TXSX1281_MD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : TXSX1281_AUX_Pin */
  GPIO_InitStruct.Pin = TXSX1281_AUX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(TXSX1281_AUX_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Tswtich3_1_Pin Tswtich3_2_Pin Tswtich4_1_Pin Tswtich4_2_Pin */
  GPIO_InitStruct.Pin = Tswtich3_1_Pin|Tswtich3_2_Pin|Tswtich4_1_Pin|Tswtich4_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
