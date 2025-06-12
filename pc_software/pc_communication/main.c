/* USER CODE BEGIN Header */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "rtc.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "display.h"
#include "button.h"
#include "data.h"
#include "getrtc.h"
#include "sdcard.h"
#include "connect.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "u8g2.h"
#include "delay.h"
#include "stm32l4xx.h"
#include <stm32l4xx_it.h>
#include "stm32l4xx_hal.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define LCD_CS_PIN GPIO_PIN_2
#define LCD_CS_PORT GPIOB

#define LCD_RST_PIN GPIO_PIN_4
#define LCD_RST_PORT GPIOA

#define RX_BUFFER_SIZE 100


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint32_t lastBtnTime = 0;  // Put this at the top of your file (global variable)
uint32_t lastprintTime = 0;  // Put this at the top of your file (global variable)
uint8_t rx_buffer[RX_BUFFER_SIZE];
volatile uint32_t last_software_tick;

extern u8g2_t u8g2;

// ✅ main.c
extern uint8_t rx_byte;  // 正确：声明，不加 =
extern uint8_t print;


uint8_t current_page = 1; // ch 1 ~ 4
uint8_t current_range = 1;
uint8_t current_channel = 1;
uint8_t current_alarm = 3; // latch
uint8_t current_record = 2; // record stop
uint8_t pc_config = 0; // no config from pc
static uint8_t pc_buffer[1];
char timeBuffer[50];
uint8_t alarm = 0;
uint32_t read_1, read_2, read_3, read_4, read_5, read_6, read_7, read_8;
uint8_t htv_1, htv_2, htv_3, htv_4, htv_5, htv_6, htv_7, htv_8;
uint8_t alarm_1h, alarm_2h, alarm_3h, alarm_4h, alarm_5h, alarm_6h, alarm_7h, alarm_8h;
uint8_t ltv_1, ltv_2, ltv_3, ltv_4, ltv_5, ltv_6, ltv_7, ltv_8;
uint8_t alarm_1l, alarm_2l, alarm_3l, alarm_4l, alarm_5l, alarm_6l, alarm_7l, alarm_8l;
uint8_t range_1, range_2, range_3, range_4, range_5, range_6, range_7, range_8;
uint8_t alarm_1, alarm_2, alarm_3, alarm_4, alarm_5, alarm_6, alarm_7, alarm_8;
uint8_t unit_1, unit_2, unit_3, unit_4;
uint8_t cmd_buffer[1000];
uint8_t cmd_index = 0;

//static uint8_t myTurn = 0;
//static uint8_t float_index = 0;
//static uint8_t float_part[4] = {0};
//static float tempF = 0;
//static char type = 100;
//static uint32_t helpme = 0;
//static float voltage[4] = {0};
//static uint32_t tempInt = 0;

//#define FLOAT_TO_UINT32(f, i) {	\
//	float tempFloat = f;	\
//	i = *(uint32_t*)&tempFloat;	\
//}
//
//#define UINT32_TO_FLOAT(i, f) {	\
//	uint32_t tempInt = i;	\
//	f = *(float*)&tempInt;	\
//}

//void separate_u32(uint32_t x, uint8_t* buf) {
//	buf[0] = (uint8_t)((x >> 24) & 0x000000FF);
//	buf[1] = (uint8_t)((x >> 16) & 0x000000FF);
//	buf[2] = (uint8_t)((x >> 8) & 0x000000FF);
//	buf[3] = (uint8_t)((x) & 0x000000FF);
//
//}

//uint32_t join_u32(uint8_t* fArray) {
//	return (((uint32_t)fArray[0] << 24) |
//			((uint32_t)fArray[1] << 16) |
//			((uint32_t)fArray[2] << 8)  |
//			((uint32_t)fArray[3]));
//}
//uint8_t check_header(uint8_t header) {
//	switch (header) {
//	case 0x77:
//		return 0;
//	case 0xFD:
//		return 1;
//	case 0x99:
//		return 2;
//	case 0xAA:
//		return 3;
//	case 0xBB:
//		return 4;
//	case 0xFE:
//		return 5;
//	case 0xDD:
//		return 6;
//	case 0xEE:
//		return 7;
//	default:
//		return 200;
//	}
//}

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
  case U8X8_MSG_GPIO_AND_DELAY_INIT:
	  HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_RESET);
	  HAL_Delay(50);
	  HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);
	  HAL_Delay(50);
	  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);// CS (chip select) pin: Output level in arg_int
	  break;
   		// default return value
  case U8X8_MSG_DELAY_MILLI:
	  HAL_Delay(arg_int);// delay arg_int * 1 nano second
       break;
  case U8X8_MSG_GPIO_CS:
	  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, arg_int);// CS (chip select) pin: Output level in arg_int
        break;
  case U8X8_MSG_GPIO_RESET:
  	  HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, arg_int);// reset pin: Output level in arg_int
  	  break;
  case U8X8_MSG_GPIO_DC:
	  //
	  break;
  }
  return 1;
}

uint8_t u8x8_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
  case  U8X8_MSG_BYTE_SEND:
	  HAL_SPI_Transmit(&hspi1, (uint8_t *)arg_ptr, arg_int, 1000);
	  break;
  case  U8X8_MSG_BYTE_SET_DC:
	  //
	  break;
  case U8X8_MSG_BYTE_START_TRANSFER:
	  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
	  break;
  case U8X8_MSG_BYTE_END_TRANSFER:
	  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
	  break;
  }
  return 1;
}

void send_fake_data(void)
{
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "A,23.5,24.1,25.0,26.3,-1.2,0.0,1.1,28.4\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
}

//void PB10_On(void) {
//    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET); // Set pin high
//}
//
//void PB10_Off(void) {
//    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET); // Set pin low
//}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

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
  MX_FATFS_Init();
  MX_RTC_Init();
  MX_USART1_UART_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);//function to enable usart3 interrupt
  HAL_UART_Receive_IT(&huart1, &rx_byte, 1);  // 初始启动接收

  u8g2_Setup_st7920_s_128x64_f(&u8g2, U8G2_R0, u8x8_spi, u8x8_gpio_and_delay);  // init u8g2 structure
  u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in sleep mode after this,
  u8g2_SetPowerSave(&u8g2, 0);
  u8g2_ClearBuffer(&u8g2);
  data_table();
  stop_record();
  display_channel();
  display_alarm();

//  uint8_t buffer[1];
//  buffer[0] = 0x55;
//  HAL_UART_Transmit(&huart2, buffer, 1, 0xFFFF);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
//	  HAL_UART_Transmit(&huart2, buffer, 1, 0xFFFF);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if (print == 1)
	      {
	          send_fake_data();
	          HAL_Delay(1000); // 每秒回传一次
	      }
	  //HAL_UART_Receive_IT(&huart1, pc_buffer, 1);
	  memset(timeBuffer, 0, sizeof(timeBuffer));
	  get_time(timeBuffer);
	  update_time(timeBuffer);
//	  update_sensor();
//	  update_alarm(alarm);

	  print_value(lastprintTime);
	  u8g2_SendBuffer(&u8g2);

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

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
////	HAL_UART_Transmit(&huart2, pc_buffer, 100, 0xFFFF);
////	pc_communication();
////	if (pc_buffer[0] == 0x55) {
////		//myTurn = 1;
////		float_index = 0;
////		return;
////	}
////	/*
////	if (myTurn == 1) {
////		float_index = 0;
////		return;
////	}
////	*/
////	if ((type == 100) || (type == 200)) {
////		type = check_header(pc_buffer[0]);
////		float_index = 0;
////		return;
////	} else {
//	if (pc_buffer[0] == 0x77) {
////		read_1 = 1;
//		read_1 |= pc_buffer[1] << 24;
//		read_1 |= pc_buffer[2] << 16;
//		read_1 |= pc_buffer[3] << 8;
//		read_1 |= pc_buffer[4];
//	}
//	if (pc_buffer[0] == 0x42) {
//		read_2 = 1;
//	}
//	if (pc_buffer[0] == 0x8b) {
//		read_3 = 1;
//	}
//	if (pc_buffer[0] == 0x62) {
//		read_4 = 1;
//	}
// //	if (pc_buffer[0] == 0xd1) {
////		htv_1 = 1;
////	}
////		float_part[float_index] = pc_buffer[0];
////		if ((float_index) >= 3) {
////			float_index = 0;
////			UINT32_TO_FLOAT(join_u32(float_part), tempF);
////
////			if (tempF == 69.693f) {
////				read_1 = 22;
////
////			}
////
////			// WRITE TEMPF TO APPROPRIATE CHANNEL HERE
////
////			type = 100;
////
////		}
////		float_index++;
////	}
//
//
//	clear_data(1);
//	data_update(1);
//	HAL_UART_Receive_IT(&huart2, pc_buffer, 1);
//
//}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    // Read encoder outputs (active low)

	uint32_t now = HAL_GetTick();
	if (now - lastBtnTime < 200) return;  // Debounce: ignore if <200ms since last press
	lastBtnTime = now;

    // Map the result

    switch (GPIO_Pin)
	{
		case GPIO_PIN_5:
			button_page();

			break;
		case GPIO_PIN_6:
			button_channel();

			break;
		case GPIO_PIN_7:
			button_range();

			break;
		case GPIO_PIN_8:
			button_alarm();

			break;
		case GPIO_PIN_9:
			button_record();

			break;
		default:
			break;
	}

}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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

  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
