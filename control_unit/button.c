/*
 * button.c
 *
 *  Created on: Apr 4, 2025
 *      Author: ching
 */

#include "button.h"
#include "display.h"
#include "data.h"
#include "rtc.h"
#include "sdcard.h"
#include "stm32l4xx_hal.h"
#include <stdbool.h>

#include "spi.h"
extern Disk_drvTypeDef  disk;

extern uint8_t current_page;
extern uint8_t current_range;
extern uint8_t current_channel;
extern uint8_t current_alarm;
extern uint8_t current_record;
extern uint8_t pc_buffer[100];
extern char timeBuffer[20];
extern float read_1, read_2, read_3, read_4, read_5, read_6, read_7, read_8;
extern float htv_1, htv_2, htv_3, htv_4, htv_5, htv_6, htv_7, htv_8;
extern float ltv_1, ltv_2, ltv_3, ltv_4, ltv_5, ltv_6, ltv_7, ltv_8;
extern uint8_t range_1, range_2, range_3, range_4, range_5, range_6, range_7, range_8;
extern uint8_t alarm_1, alarm_2, alarm_3, alarm_4, alarm_5, alarm_6, alarm_7, alarm_8;

extern bool button1;
extern bool button2;
extern bool button3;
extern bool button4;
extern bool button5;

void button_page()
{
	if (button1 == true)
	{
		if (current_page == 1) {
			current_page = 2;
			page_two();
			current_channel = 5;
		} else if (current_page == 2) {
			current_page = 1;
			page_one();
			current_channel = 1;
		}
		display_channel();
		send_data('H');
		button1 = false;
	}
}

void button_range()
{
	if (button2 == true)
	{
		if (current_range == 1) {
			current_range = 10;
		} else if (current_range == 10) {
			current_range = 1;
		}
		value_update(current_channel, 1, current_range);
		int row = channel_row(current_channel);
		uint8_t unit = channel_unit(current_channel);
		update_range(row, current_range, unit);
		send_data('D');
		button2 = false;
	}
}

void button_channel()
{
	if (button3 == true)
	{
		channel_update();
		display_channel();
		send_data('H');
		button3 = false;
	}
}

void button_alarm()
{
	if (button4 == true)
	{
		if (current_alarm == 1) {
			current_alarm = 2;

		} else if (current_alarm == 2) {
			current_alarm = 3;

		} else if (current_alarm == 3) {
			current_alarm = 1;

		}
		value_update(current_channel, 2, current_alarm);
		int row = channel_row(current_channel);
		update_status(row, current_alarm);
		display_alarm();
		send_data('E');
		button4 = false;
	}
}

void button_record()
{
	uint32_t timeout = HAL_GetTick() + 500;  // 500ms timeout

	if (button5 == true)
	{
		if (current_record == 1) {
			current_record = 2;
			sd_record();
			send_data('G');
		} else if (current_record == 2) {
			current_record = 1;
			sd_record();
			send_data('F');

			do {
				SD_SPI_Transmit(0xFF);
			} while (HAL_GetTick() < timeout);

			init_csv();
			write_csv(read_1, read_2, read_3, read_4, read_5, read_6, read_7, read_8);
		}
		button5 = false;
	}
}
