/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "fdcan.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TEST_MSG_LEN    64
#define CAN_TEST_ID     0x123
#define CAN_TX_TIMEOUT_MS  100
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static uint8_t test_stage = 0;
static uint8_t test_done = 0;
static uint8_t fdcan_started[3] = {0};
static char test_msg[TEST_MSG_LEN];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */
static void Test_FDCAN_Print(const char *name, FDCAN_HandleTypeDef *hfdcan, uint8_t started);
static void Test_UART_Print(const char *name, UART_HandleTypeDef *huart);
static void Delay_ms(uint32_t ms);
static void Debug_Print(const char *message);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_FDCAN1_Init();
  MX_FDCAN2_Init();
  MX_FDCAN3_Init();
  MX_UART4_Init();
  MX_UART5_Init();
  MX_UART7_Init();
  MX_UART8_Init();
  MX_UART9_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_USART6_UART_Init();
  MX_USART10_UART_Init();
  /* USER CODE BEGIN 2 */

  /* Print test banner via UART4 (debug console) */
  Debug_Print("\r\n========================================\r\n");
  Debug_Print("  RC26 Board Test Program\r\n");
  Debug_Print("  MCU: STM32H723ZGTx\r\n");
  Debug_Print("========================================\r\n");

  /* Start FDCAN peripherals and retain each result for the test stage. */
  fdcan_started[0] = (HAL_FDCAN_Start(&hfdcan1) == HAL_OK) ? 1U : 0U;
  fdcan_started[1] = (HAL_FDCAN_Start(&hfdcan2) == HAL_OK) ? 1U : 0U;
  fdcan_started[2] = (HAL_FDCAN_Start(&hfdcan3) == HAL_OK) ? 1U : 0U;

  snprintf(test_msg, TEST_MSG_LEN, "  FDCAN1 start: %s err=0x%08lX\r\n",
           fdcan_started[0] ? "[OK]" : "[FAIL]", (unsigned long)HAL_FDCAN_GetError(&hfdcan1));
  Debug_Print(test_msg);
  snprintf(test_msg, TEST_MSG_LEN, "  FDCAN2 start: %s err=0x%08lX\r\n",
           fdcan_started[1] ? "[OK]" : "[FAIL]", (unsigned long)HAL_FDCAN_GetError(&hfdcan2));
  Debug_Print(test_msg);
  snprintf(test_msg, TEST_MSG_LEN, "  FDCAN3 start: %s err=0x%08lX\r\n",
           fdcan_started[2] ? "[OK]" : "[FAIL]", (unsigned long)HAL_FDCAN_GetError(&hfdcan3));
  Debug_Print(test_msg);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if (test_done)
    {
      continue;
    }

    switch (test_stage)
    {
      /* ===== FDCAN Tests ===== */
      case 0:
        snprintf(test_msg, TEST_MSG_LEN, "\r\n--- CAN Test Start ---\r\n");
        HAL_UART_Transmit(&huart4, (uint8_t *)test_msg, strlen(test_msg), 1000);
        test_stage++;
        break;

      case 1:
        Test_FDCAN_Print("FDCAN1", &hfdcan1, fdcan_started[0]);
        test_stage++;
        break;

      case 2:
        Test_FDCAN_Print("FDCAN2", &hfdcan2, fdcan_started[1]);
        test_stage++;
        break;

      case 3:
        Test_FDCAN_Print("FDCAN3", &hfdcan3, fdcan_started[2]);
        test_stage++;
        break;

      /* ===== UART Tests ===== */
      case 4:
        snprintf(test_msg, TEST_MSG_LEN, "\r\n--- UART Test Start ---\r\n");
        HAL_UART_Transmit(&huart4, (uint8_t *)test_msg, strlen(test_msg), 1000);
        test_stage++;
        break;

      case 5:
        Test_UART_Print("UART4", &huart4);
        test_stage++;
        break;

      case 6:
        Test_UART_Print("UART5", &huart5);
        test_stage++;
        break;

      case 7:
        Test_UART_Print("UART7", &huart7);
        test_stage++;
        break;

      case 8:
        Test_UART_Print("UART8", &huart8);
        test_stage++;
        break;

      case 9:
        Test_UART_Print("UART9", &huart9);
        test_stage++;
        break;

      case 10:
        Test_UART_Print("USART1", &huart1);
        test_stage++;
        break;

      case 11:
        Test_UART_Print("USART2", &huart2);
        test_stage++;
        break;

      case 12:
        Test_UART_Print("USART3", &huart3);
        test_stage++;
        break;

      case 13:
        Test_UART_Print("USART6", &huart6);
        test_stage++;
        break;

      case 14:
        Test_UART_Print("USART10", &huart10);
        test_stage++;
        break;

      /* ===== All Tests Complete ===== */
      case 15:
        snprintf(test_msg, TEST_MSG_LEN, "\r\n========================================\r\n");
        HAL_UART_Transmit(&huart4, (uint8_t *)test_msg, strlen(test_msg), 1000);
        snprintf(test_msg, TEST_MSG_LEN, "  ALL TESTS COMPLETED!\r\n");
        HAL_UART_Transmit(&huart4, (uint8_t *)test_msg, strlen(test_msg), 1000);
        snprintf(test_msg, TEST_MSG_LEN, "========================================\r\n");
        HAL_UART_Transmit(&huart4, (uint8_t *)test_msg, strlen(test_msg), 1000);
        test_done = 1;
        break;

      default:
        break;
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 44;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/**
  * @brief  Test FDCAN by sending a classic CAN frame via TX FIFO
  * @param  name:   display name for debug output
  * @param  hfdcan: pointer to FDCAN handle
  * @retval None
  */
static void Test_FDCAN_Print(const char *name, FDCAN_HandleTypeDef *hfdcan, uint8_t started)
{
  FDCAN_TxHeaderTypeDef TxHeader;
  FDCAN_ProtocolStatusTypeDef protocol_status = {0};
  uint8_t tx_data[8] = {'R', 'C', '2', '6', '_', 'C', 'A', 'N'};
  HAL_StatusTypeDef ret;
  uint32_t tx_buffer;
  uint32_t start_tick;

  if (!started)
  {
    snprintf(test_msg, TEST_MSG_LEN, "  %-8s: QUEUE [SKIP], TX [FAIL: NOT STARTED]\r\n", name);
    Debug_Print(test_msg);
    return;
  }

  TxHeader.Identifier = CAN_TEST_ID;
  TxHeader.IdType = FDCAN_STANDARD_ID;
  TxHeader.TxFrameType = FDCAN_DATA_FRAME;
  TxHeader.DataLength = FDCAN_DLC_BYTES_8;
  TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
  TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
  TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  TxHeader.MessageMarker = 0;

  ret = HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &TxHeader, tx_data);

  if (ret != HAL_OK)
  {
    snprintf(test_msg, TEST_MSG_LEN, "  %-8s: QUEUE [FAIL:%d] err=0x%08lX\r\n",
             name, (int)ret, (unsigned long)HAL_FDCAN_GetError(hfdcan));
    Debug_Print(test_msg);
    return;
  }

  tx_buffer = HAL_FDCAN_GetLatestTxFifoQRequestBuffer(hfdcan);
  start_tick = HAL_GetTick();
  while (HAL_FDCAN_IsTxBufferMessagePending(hfdcan, tx_buffer) != 0U)
  {
    if ((HAL_GetTick() - start_tick) >= CAN_TX_TIMEOUT_MS)
    {
      (void)HAL_FDCAN_AbortTxRequest(hfdcan, tx_buffer);
      (void)HAL_FDCAN_GetProtocolStatus(hfdcan, &protocol_status);
      snprintf(test_msg, TEST_MSG_LEN,
               "  %-8s: QUEUE [OK], TX [TIMEOUT] LEC=%lu BO=%lu\r\n",
               name, (unsigned long)protocol_status.LastErrorCode,
               (unsigned long)protocol_status.BusOff);
      Debug_Print(test_msg);
      return;
    }
  }

  (void)HAL_FDCAN_GetProtocolStatus(hfdcan, &protocol_status);
  if ((hfdcan->Instance->TXBTO & tx_buffer) != 0U)
  {
    snprintf(test_msg, TEST_MSG_LEN, "  %-8s: QUEUE [OK], TX [OK] ID=0x%03X\r\n",
             name, CAN_TEST_ID);
  }
  else
  {
    snprintf(test_msg, TEST_MSG_LEN,
             "  %-8s: QUEUE [OK], TX [FAIL] LEC=%lu BO=%lu\r\n",
             name, (unsigned long)protocol_status.LastErrorCode,
             (unsigned long)protocol_status.BusOff);
  }
  Debug_Print(test_msg);

  Delay_ms(100);
}

/**
  * @brief  Test UART by sending a string via blocking TX
  * @param  name:  display name for debug output
  * @param  huart: pointer to UART handle
  * @retval None
  */
static void Test_UART_Print(const char *name, UART_HandleTypeDef *huart)
{
  HAL_StatusTypeDef ret;
  char uart_msg[TEST_MSG_LEN];

  snprintf(uart_msg, TEST_MSG_LEN, "RC26 UART Test: %s\r\n", name);
  ret = HAL_UART_Transmit(huart, (uint8_t *)uart_msg, strlen(uart_msg), 1000);

  snprintf(test_msg, TEST_MSG_LEN, "  %-8s: TX(%d bytes) %s\r\n",
           name, (int)strlen(uart_msg), (ret == HAL_OK) ? "[OK]" : "[FAIL]");
  Debug_Print(test_msg);

  Delay_ms(50);
}

/**
  * @brief  Simple blocking delay (HAL_Delay alternative)
  * @param  ms: delay in milliseconds
  * @retval None
  */
static void Delay_ms(uint32_t ms)
{
  uint32_t start = uwTick;
  while ((uwTick - start) < ms) { }
}

/**
  * @brief  Send a zero-terminated status message through the UART4 console.
  */
static void Debug_Print(const char *message)
{
  (void)HAL_UART_Transmit(&huart4, (uint8_t *)message, (uint16_t)strlen(message), 1000);
}

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress = 0x08000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_1MB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.AccessPermission = MPU_REGION_PRIV_RW;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
  MPU_InitStruct.BaseAddress = 0x20000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
