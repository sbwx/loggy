/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "usart.h"

/* USER CODE BEGIN 0 */
#include "display.h"
#include "data.h"
#include "sdcard.h"
#include "connect.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END 0 */

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

uint8_t rx_byte = 0;              // 用于接收每个字符
extern uint8_t pc_buffer[100];
extern uint8_t cmd_buffer[1000];
extern uint8_t cmd_index;
extern uint8_t current_channel;
extern uint8_t current_range;
extern uint8_t current_alarm;
extern uint8_t current_page;
extern uint8_t current_record;
extern uint32_t read_1, read_2, read_3, read_4, read_5, read_6, read_7, read_8;
extern uint8_t htv_1, htv_2, htv_3, htv_4, htv_5, htv_6, htv_7, htv_8;
extern uint8_t alarm_1h, alarm_2h, alarm_3h, alarm_4h, alarm_5h, alarm_6h, alarm_7h, alarm_8h;
extern uint8_t ltv_1, ltv_2, ltv_3, ltv_4, ltv_5, ltv_6, ltv_7, ltv_8;
extern uint8_t alarm_1l, alarm_2l, alarm_3l, alarm_4l, alarm_5l, alarm_6l, alarm_7l, alarm_8l;
extern uint8_t range_1, range_2, range_3, range_4, range_5, range_6, range_7, range_8;
extern uint8_t alarm_1, alarm_2, alarm_3, alarm_4, alarm_5, alarm_6, alarm_7, alarm_8;
extern uint8_t unit_1, unit_2, unit_3, unit_4;
uint8_t print = 0;
/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}
/* USART2 init function */

void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 4800;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
    PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void process_rx_buffer(uint8_t *rx_buffer) {
    float f1, f2, f3, f4, f5;
    char id = rx_buffer[0];

    if (sscanf((char*)&rx_buffer[2], "%f,%f,%f,%f,%f", &f1, &f2, &f3, &f4, &f5) != 5) {
        return; // 数据格式错误
    }

    switch (id) {
        case 'A':
            htv_1 = f1; ltv_1 = f2; alarm_1 = f3; range_1 = f4; range_5 = f4; unit_1 = f5;
            clear_data(1); data_update(1); break;
        case 'B':
            htv_2 = f1; ltv_2 = f2; alarm_2 = f3; range_2 = f4; range_6 = f4; unit_2 = f5;
            clear_data(2); data_update(2); break;
        case 'C':
            htv_3 = f1; ltv_3 = f2; alarm_3 = f3; range_3 = f4; range_7 = f4; unit_3 = f5;
            clear_data(3); data_update(3); break;
        case 'D':
            htv_4 = f1; ltv_4 = f2; alarm_4 = f3; range_4 = f4; range_8 = f4; unit_4 = f5;
            clear_data(4); data_update(4); break;
        case 'E':
            htv_5 = f1; ltv_5 = f2; alarm_5 = f3;
            clear_data(5); data_update(5); break;
        case 'F':
            htv_6 = f1; ltv_6 = f2; alarm_6 = f3;
            clear_data(6); data_update(6); break;
        case 'G':
            htv_7 = f1; ltv_7 = f2; alarm_7 = f3;
            clear_data(7); data_update(7); break;
        case 'H':
            htv_8 = f1; ltv_8 = f2; alarm_8 = f3;
            clear_data(8); data_update(8); break;
        case 'T':
            print = 0; break;
        case 'I':
            print = 1; break;
        case 'J':
            print = 0; break;
        default:
            // 无效指令
            break;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        if (rx_byte == 'y') {
            display_connect(1);  // GUI 开启连接
        }
        else if (rx_byte == 'z') {
            display_connect(0);  // GUI 断开连接
        }
        else if (rx_byte == '\n') {
            cmd_buffer[cmd_index] = '\0';  // 加字符串终止符
            process_rx_buffer(cmd_buffer); // 👉 使用你写好的函数
            cmd_index = 0;
        }
        else {
            if (cmd_index < sizeof(cmd_buffer) - 1) {
                cmd_buffer[cmd_index++] = rx_byte;
            } else {
                // 防止越界
                cmd_index = 0;
            }
        }

        // 启动下一次接收
        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
    }
}


/* USER CODE END 1 */
