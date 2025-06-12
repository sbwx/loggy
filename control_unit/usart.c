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
#include "rtc.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "u8g2.h"
extern u8g2_t u8g2;

extern uint8_t current_range;
extern uint8_t current_alarm;
extern uint8_t current_channel;

extern bool pc_gui;
extern bool sensor_data;
uint8_t rx_byte = 0;
uint8_t sensor_byte = 0;
extern uint8_t cmd_buffer[1000];
extern uint8_t cmd_index;
extern uint8_t sensor_buffer[1000];
extern uint8_t sensor_index;
extern uint8_t current_channel;
extern uint8_t current_range;
extern uint8_t current_alarm;
extern uint8_t current_page;
extern uint8_t current_record;
extern float read_1, read_2, read_3, read_4, read_5, read_6, read_7, read_8;
extern float htv_1, htv_2, htv_3, htv_4, htv_5, htv_6, htv_7, htv_8;
extern float ltv_1, ltv_2, ltv_3, ltv_4, ltv_5, ltv_6, ltv_7, ltv_8;
extern uint8_t alarm_1h, alarm_2h, alarm_3h, alarm_4h, alarm_5h, alarm_6h, alarm_7h, alarm_8h;
extern uint8_t alarm_1l, alarm_2l, alarm_3l, alarm_4l, alarm_5l, alarm_6l, alarm_7l, alarm_8l;
extern uint8_t alarm_1, alarm_2, alarm_3, alarm_4, alarm_5, alarm_6, alarm_7, alarm_8;
extern uint8_t range_1, range_2, range_3, range_4, range_5, range_6, range_7, range_8;
extern uint8_t unit_1, unit_2, unit_3, unit_4;
uint8_t print = 0;
/* USER CODE END 0 */

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 4800;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXOVERRUNDISABLE_INIT|UART_ADVFEATURE_DMADISABLEONERROR_INIT;
  huart1.AdvancedInit.OverrunDisable = UART_ADVFEATURE_OVERRUN_DISABLE;
  huart1.AdvancedInit.DMADisableonRxError = UART_ADVFEATURE_DMA_DISABLEONRXERROR;
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
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXOVERRUNDISABLE_INIT|UART_ADVFEATURE_DMADISABLEONERROR_INIT;
  huart2.AdvancedInit.OverrunDisable = UART_ADVFEATURE_OVERRUN_DISABLE;
  huart2.AdvancedInit.DMADisableonRxError = UART_ADVFEATURE_DMA_DISABLEONRXERROR;
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

	float f3, f4, f5;
	int f1, f2;
    char id = rx_buffer[0];

    if (sscanf((char*)&rx_buffer[2], "%d,%d,%f,%f,%f", &f1, &f2, &f3, &f4, &f5) != 5) {
        return;
    }

	current_range = f4;
	current_alarm = f3;
	display_alarm();

	char buffer[10];
	snprintf(buffer, sizeof(buffer), "z\n");
	HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
	pc_gui = false;

    switch (id) {
		case 'y':
			display_connect(1); break;
		case 'z':
			display_connect(0); break;
		case 'I':
			print = 1; break;
		case 'J':
			print = 0; break;
		case 'L':
			HAL_GPIO_DeInit(GPIOB, GPIO_PIN_9);
			current_record = 1;
			sd_record();
			init_csv(); break;
		case 'M':
			GPIO_InitTypeDef GPIO_InitStruct = {0};
			GPIO_InitStruct.Pin = GPIO_PIN_9;
			GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
			GPIO_InitStruct.Pull = GPIO_PULLUP;
			HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
			HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
			current_record = 2;
			sd_record();
			write_csv(read_1, read_2, read_3, read_4, read_5, read_6, read_7, read_8);
			break;
        case 'A':
            htv_1 = f1 / 1000.0f; ltv_1 = f2 / 1000.0f; alarm_1 = f3; range_1 = f4; range_5 = f4; unit_1 = f5;
            if (current_page == 1)
            {
            	current_channel = 1;
            	print_channel(1);
            }
            break;
        case 'B':
            htv_2 = f1 / 1000.0f; ltv_2 = f2 / 1000.0f; alarm_2 = f3; range_2 = f4; range_6 = f4; unit_2 = f5;
            if (current_page == 1)
            {
            	current_channel = 2;
            	print_channel(2);
            }
            break;
        case 'C':
            htv_3 = f1 / 1000.0f; ltv_3 = f2 / 1000.0f; alarm_3 = f3; range_3 = f4; range_7 = f4; unit_3 = f5;
            if (current_page == 1)
            {
            	current_channel = 3;
            	print_channel(3);
            }
            break;
        case 'D':
            htv_4 = f1 / 1000.0f; ltv_4 = f2 / 1000.0f; alarm_4 = f3; range_4 = f4; range_8 = f4; unit_4 = f5;
            if (current_page == 1)
            {
            	current_channel = 4;
            	print_channel(4);
            }
            break;
        case 'E':
            htv_5 = f1 / 1000.0f; ltv_5 = f2 / 1000.0f; alarm_5 = f3;
            if (current_page == 2)
            {
            	current_channel = 5;
            	print_channel(5);
            }
            break;
        case 'F':
            htv_6 = f1 / 1000.0f; ltv_6 = f2 / 1000.0f; alarm_6 = f3;
            if (current_page == 2)
            {
            	current_channel = 6;
            	print_channel(6);
            }
            break;
        case 'G':
            htv_7 = f1 / 1000.0f; ltv_7 = f2 / 1000.0f; alarm_7 = f3;
            if (current_page == 2)
            {
            	current_channel = 7;
            	print_channel(7);
            }
            break;
        case 'H':
            htv_8 = f1 / 1000.0f; ltv_8 = f2 / 1000.0f; alarm_8 = f3;
            if (current_page == 2)
            {
            	current_channel = 8;
            	print_channel(8);
            }
            break;
        case 'T':
        	RTC_TimeTypeDef sTime = {0};
        	RTC_DateTypeDef sDate = {0};

        	sTime.Hours = f3;
        	sTime.Minutes = f4;
        	sTime.Seconds = f5;
        	HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

        	sDate.Year = 25;
        	sDate.Month = f1;
        	sDate.Date = f2;
        	sDate.WeekDay = RTC_WEEKDAY_FRIDAY;
        	HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

        	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, '*');
        	break;

        default:
            break;
    }
    memset(cmd_buffer, 0, sizeof(cmd_buffer));
}

void process_sensor_buffer(uint8_t *s_buffer)
{
	int f1, f2, f3, f4, f5, f6, f7, f8;
    char buffer[1000];

    f1 = (s_buffer[3] | ((s_buffer[2]) << 8) | ((s_buffer[1]) << 16) | ((s_buffer[0]) << 24));
    f2 = (s_buffer[7] | ((s_buffer[6]) << 8) | ((s_buffer[5]) << 16) | ((s_buffer[4]) << 24));
    f3 = (s_buffer[11] | ((s_buffer[10]) << 8) | ((s_buffer[9]) << 16) | ((s_buffer[8]) << 24));
    f4 = (s_buffer[15] | ((s_buffer[14]) << 8) | ((s_buffer[13]) << 16) | ((s_buffer[12]) << 24));
    f5 = (s_buffer[19] | ((s_buffer[18]) << 8) | ((s_buffer[17]) << 16) | ((s_buffer[16]) << 24));
    f6 = (s_buffer[23] | ((s_buffer[22]) << 8) | ((s_buffer[21]) << 16) | ((s_buffer[20]) << 24));
    f7 = (s_buffer[27] | ((s_buffer[26]) << 8) | ((s_buffer[25]) << 16) | ((s_buffer[24]) << 24));
    f8 = (s_buffer[31] | ((s_buffer[30]) << 8) | ((s_buffer[29]) << 16) | ((s_buffer[28]) << 24));

	read_1 = f1 / 1000.0f; read_2 = f2 / 1000.0f; read_3 = f3 / 1000.0f; read_4 = f4 / 1000.0f;
	read_5 = f5 / 1000.0f; read_6 = f6 / 1000.0f; read_7 = f7 / 1000.0f; read_8 = f8 / 1000.0f;

	check_alarm();

	if (current_page == 1)
	{
		update_read(15, read_1, unit_1);
		update_read(27, read_2, unit_2);
		update_read(39, read_3, unit_3);
		update_read(51, read_4, unit_4);
	} else {
		update_read(15, read_5, unit_1);
		update_read(27, read_6, unit_2);
		update_read(39, read_7, unit_3);
		update_read(51, read_8, unit_4);
	}

    buffer[0] = 0x99;
    if (range_1 == 10)
    {
        buffer[1] = 0;
    } else
    {
        buffer[1] = 1;
    }
    if (range_2 == 10)
    {
        buffer[2] = 0;
    } else
    {
        buffer[2] = 1;
    }
    if (range_3 == 10)
    {
        buffer[3] = 0;
    } else
    {
        buffer[3] = 1;
    }
    if (range_4 == 10)
    {
        buffer[4] = 0;
    } else
    {
        buffer[4] = 1;
    }
    buffer[5] = alarm_1h;
    buffer[6] = alarm_2h;
    buffer[7] = alarm_3h;
    buffer[8] = alarm_4h;
    buffer[9] = alarm_5h;
    buffer[10] = alarm_6h;
    buffer[11] = alarm_7h;
    buffer[12] = alarm_8h;
    buffer[13] = alarm_1l;
    buffer[14] = alarm_2l;
    buffer[15] = alarm_3l;
    buffer[16] = alarm_4l;
    buffer[17] = alarm_5l;
    buffer[18] = alarm_6l;
    buffer[19] = alarm_7l;
    buffer[20] = alarm_8l;
    buffer[21] = 0xFD;

//    HAL_Delay(2);
    HAL_UART_Transmit(&huart2, (uint8_t*)buffer, 22, HAL_MAX_DELAY);

	sensor_data = false;
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
    memset(sensor_buffer, 0, sizeof(sensor_buffer));
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
    	uint8_t tmp = rx_byte;
        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);

    	if (tmp == 'y' || tmp == 'z' || tmp == 'I' || tmp == 'J' || tmp == 'L' || tmp == 'M' ||
    			tmp == 'A' || tmp == 'B' || tmp == 'C' || tmp == 'D' || tmp == 'E' || tmp == 'F' || tmp == 'G' || tmp == 'H' || tmp == 'T') {
    		cmd_buffer[0] = tmp;
    		cmd_index = 0;
    	}
		if (tmp == 'k') {
			pc_gui = true;
			cmd_index = 0;
		} else if (tmp == ',' && cmd_index == 0) {
		} else {
			cmd_buffer[cmd_index++] = tmp;
		}
    }
    if (huart->Instance == USART2)
    {
  	    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);

    	uint8_t tmp2 = sensor_byte;
        HAL_UART_Receive_IT(&huart2, &sensor_byte, 1);

    	if (tmp2 == 0x77) {
    		sensor_index = 0;
    	} else if (tmp2 == 0x55) {
			sensor_data = true;
			sensor_index = 0;
		} else {
			sensor_buffer[sensor_index++] = tmp2;

		}
    }
}


/* USER CODE END 1 */
