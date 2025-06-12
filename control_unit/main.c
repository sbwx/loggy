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
volatile uint32_t last_software_tick;

extern u8g2_t u8g2;

// main.c
extern uint8_t rx_byte;
extern uint8_t sensor_byte;
extern uint8_t print;
uint8_t cmd_buffer[1000];
uint8_t cmd_index = 0;
uint8_t sensor_buffer[1000];
uint8_t sensor_index = 0;

uint8_t current_page = 1; // ch 1 ~ 4
uint8_t current_range = 1;
uint8_t current_channel = 1;
uint8_t current_alarm = 2; // live
uint8_t current_record = 2; // record stop
uint8_t pc_config = 0; // no config from pc
char timeBuffer[50];
uint8_t alarm = 0;
float pre_1, pre_2, pre_3, pre_4, pre_5, pre_6, pre_7, pre_8;
float read_1, read_2, read_3, read_4, read_5, read_6, read_7, read_8;
float htv_1, htv_2, htv_3, htv_4, htv_5, htv_6, htv_7, htv_8;
float ltv_1, ltv_2, ltv_3, ltv_4, ltv_5, ltv_6, ltv_7, ltv_8;
uint8_t alarm_1h, alarm_2h, alarm_3h, alarm_4h, alarm_5h, alarm_6h, alarm_7h, alarm_8h;
uint8_t alarm_1l, alarm_2l, alarm_3l, alarm_4l, alarm_5l, alarm_6l, alarm_7l, alarm_8l;
uint8_t alarm_1, alarm_2, alarm_3, alarm_4, alarm_5, alarm_6, alarm_7, alarm_8;
uint8_t range_1, range_2, range_3, range_4, range_5, range_6, range_7, range_8;
uint8_t unit_1, unit_2, unit_3, unit_4;

bool button1 = false;
bool button2 = false;
bool button3 = false;
bool button4 = false;
bool button5 = false;

bool pc_gui = false;
bool sensor_data = false;
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

void send_data(uint8_t code)
{
    char buffer[300];

    if (code == 'A') {
        snprintf(buffer, sizeof(buffer), "A,%f,%f,%f,%f,%f,%f,%f,%f\n", read_1, read_2, read_3, read_4, read_5, read_6, read_7, read_8);
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
    } else if (code == 'B') {
        snprintf(buffer, sizeof(buffer), "B,%d,%d,%d,%d,%d,%d,%d,%d\n", alarm_1h, alarm_2h, alarm_3h, alarm_4h, alarm_5h, alarm_6h, alarm_7h, alarm_8h);
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
    } else if (code == 'C') {
        snprintf(buffer, sizeof(buffer), "C,%d,%d,%d,%d,%d,%d,%d,%d\n", alarm_1l, alarm_2l, alarm_3l, alarm_4l, alarm_5l, alarm_6l, alarm_7l, alarm_8l);
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
    } else if (code == 'D') {
        snprintf(buffer, sizeof(buffer), "D,%d,%d,%d,%d,%d,%d,%d,%d\n", range_1, range_2, range_3, range_4, range_5, range_6, range_7, range_8);
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
    } else if (code == 'E') {
        snprintf(buffer, sizeof(buffer), "E,%d,%d,%d,%d,%d,%d,%d,%d\n", alarm_1, alarm_2, alarm_3, alarm_4, alarm_5, alarm_6, alarm_7, alarm_8);
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
    } else if (code == 'F') {
	   snprintf(buffer, sizeof(buffer), "F,0,0,0,0,0,0,0,0\n");
	   HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
   } else if (code == 'G') {
	   snprintf(buffer, sizeof(buffer), "G,0,0,0,0,0,0,0,0\n");
	   HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
   } else if (code == 'H') {
	   snprintf(buffer, sizeof(buffer), "H,%d,0,0,0,0,0,0,0\n", current_channel);
	   HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
   }
}

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
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);//function to enable usart1 interrupt
  HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);//function to enable usart2 interrupt
  HAL_UART_Receive_IT(&huart2, &sensor_byte, 1);

  u8g2_Setup_st7920_s_128x64_f(&u8g2, U8G2_R0, u8x8_spi, u8x8_gpio_and_delay);  // init u8g2 structure
  u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in sleep mode after this,
  u8g2_SetPowerSave(&u8g2, 0);
  init_value();
  data_table();
  display_channel();
  display_alarm();
  sd_record();
  page_one();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if (print == 1)
	  {
		  send_data('A');
		  HAL_Delay(1000);
	  }

	  if (pc_gui)
	  {
		  process_rx_buffer(cmd_buffer);
	  }

	  if (sensor_data)
	  {
		  process_sensor_buffer(sensor_buffer);
	  }

	  memset(timeBuffer, 0, sizeof(timeBuffer));
	  get_time(timeBuffer);
	  update_time(timeBuffer);
//	  update_sensor();
	  button_page();
  	  button_channel();
	  button_range();
	  button_alarm();
	  button_record();


//	  print_value(lastprintTime);
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

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
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

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/* USER CODE BEGIN 4 */


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    // Read encoder outputs (active low)
	uint32_t now = HAL_GetTick();
	if (now - lastBtnTime < 250) return;  // Debounce: ignore if <200ms since last press
	lastBtnTime = now;

    // Map the result

    switch (GPIO_Pin)
	{
		case GPIO_PIN_5:
			button1 = !button1;

			break;
		case GPIO_PIN_6:
			button2 = !button2;

			break;
		case GPIO_PIN_7:
			button3 = !button3;

			break;
		case GPIO_PIN_8:
			button4 = !button4;

			break;
		case GPIO_PIN_9:
			button5 = !button5;

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
