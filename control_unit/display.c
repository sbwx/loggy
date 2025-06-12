/*
 * display.c
 *
 *  Created on: Apr 3, 2025
 *      Author: ching
 */
#include "display.h"
#include "data.h"
#include "sdcard.h"
#include <stdio.h>
#include <string.h>

u8g2_t u8g2;

extern uint8_t pc_buffer[100];
extern uint8_t current_channel;
extern uint8_t current_range;
extern uint8_t current_alarm;
extern uint8_t current_page;
extern uint8_t current_record;
extern uint8_t pc_callback;
extern float read_1, read_2, read_3, read_4, read_5, read_6, read_7, read_8;
extern float htv_1, htv_2, htv_3, htv_4, htv_5, htv_6, htv_7, htv_8;
extern float ltv_1, ltv_2, ltv_3, ltv_4, ltv_5, ltv_6, ltv_7, ltv_8;
extern uint8_t alarm_1h, alarm_2h, alarm_3h, alarm_4h, alarm_5h, alarm_6h, alarm_7h, alarm_8h;
extern uint8_t alarm_1l, alarm_2l, alarm_3l, alarm_4l, alarm_5l, alarm_6l, alarm_7l, alarm_8l;
extern uint8_t alarm_1, alarm_2, alarm_3, alarm_4, alarm_5, alarm_6, alarm_7, alarm_8;
extern uint8_t range_1, range_2, range_3, range_4, range_5, range_6, range_7, range_8;
extern uint8_t unit_1, unit_2, unit_3, unit_4;

int previous_alarm;
uint8_t previous_page = 1;

void init_value()
{
	read_1 = 0;	htv_1 = 5; ltv_1 = 0; range_1 = 1; alarm_1 = 2; unit_1 = 0; alarm_1h = 0; alarm_1l = 0;

	read_2 = 0;	htv_2 = 5; ltv_2 = 0; range_2 = 1; alarm_2 = 2; unit_2 = 0; alarm_2h = 0; alarm_2l = 0;

	read_3 = 0;	htv_3 = 5; ltv_3 = 0; range_3 = 1; alarm_3 = 2; unit_3 = 0; alarm_3h = 0; alarm_3l = 0;

	read_4 = 0;	htv_4 = 5; ltv_4 = 0; range_4 = 1; alarm_4 = 2; unit_4 = 0; alarm_4h = 0; alarm_4l = 0;

	read_5 = 0; htv_5 = 5; ltv_5 = 0; range_5 = 1; alarm_5 = 2; alarm_5h = 0; alarm_5l = 0;

	read_6 = 0; htv_6 = 5; ltv_6 = 0; range_6 = 1; alarm_6 = 2; alarm_6h = 0; alarm_6l = 0;

	read_7 = 0; htv_7 = 5; ltv_7 = 0; range_7 = 1; alarm_7 = 2; alarm_7h = 0; alarm_7l = 0;

	read_8 = 0; htv_8 = 5; ltv_8 = 0; range_8 = 1; alarm_8 = 2; alarm_8h = 0; alarm_8l = 0;
}

void data_table()
{
	u8g2_DrawHLine(&u8g2, 0, 6, 128);
	u8g2_DrawHLine(&u8g2, 0,  18 , 128);
	u8g2_DrawHLine(&u8g2, 0,  30 , 128);
	u8g2_DrawHLine(&u8g2, 0,  42 , 128);
	u8g2_DrawHLine(&u8g2, 0,  54 , 128);
	u8g2_DrawVLine(&u8g2, 6,  6 , 48);
	u8g2_DrawVLine(&u8g2, 46,  0 , 54);
	u8g2_DrawVLine(&u8g2, 73,  0 , 54);
	u8g2_DrawVLine(&u8g2, 100,  0 , 54);
	u8g2_DrawVLine(&u8g2, 114,  0 , 54);
	u8g2_SetFont(&u8g2,u8g2_font_micro_mr);
	u8g2_DrawStr(&u8g2, 1, 5, "PC:");
	u8g2_DrawStr(&u8g2, 22, 5, "S/U:");
	u8g2_DrawStr(&u8g2, 55, 5, "HTV");
	u8g2_DrawStr(&u8g2, 81, 5, "LTV");
	u8g2_DrawStr(&u8g2, 104, 5, "+-");
	u8g2_DrawStr(&u8g2, 118, 5, "!?");
	u8g2_DrawGlyph(&u8g2, 89, 62, 91);
	u8g2_DrawGlyph(&u8g2, 100, 62, 93);
	u8g2_DrawGlyph(&u8g2, 102, 62, 91);
	u8g2_DrawGlyph(&u8g2, 113, 62, 93);
	u8g2_DrawGlyph(&u8g2, 115, 62, 91);
	u8g2_DrawGlyph(&u8g2, 126, 62, 93);
	u8g2_SendBuffer(&u8g2);
}

int channel_row(uint8_t channel)
{
	int row = 0;

	if (channel == 1 || channel == 5)
	{
		row = 15;
	} else if (channel == 2 || channel == 6)
	{
		row = 27;
	} else if (channel == 3 || channel == 7)
	{
		row = 39;
	} else if (channel == 4 || channel == 8)
	{
		row = 51;
	}

	return row;
}

uint8_t channel_unit(uint8_t channel)
{
	uint8_t unit = 0;

	if (channel == 1 || channel == 5)
	{
		unit = unit_1;
	} else if (channel == 2 || channel == 6)
	{
		unit = unit_2;
	} else if (channel == 3 || channel == 7)
	{
		unit = unit_3;
	} else if (channel == 4 || channel == 8)
	{
		unit = unit_4;
	}

	return unit;
}

void update_read(int row, float read, uint8_t unit)
{
	char buffer[128];
	float read_temp;

 	u8g2_SetFontMode(&u8g2, 1);
	u8g2_SetDrawColor(&u8g2, 0);
	u8g2_DrawBox(&u8g2, 8, (row - 7), 28, 9);
	u8g2_SendBuffer(&u8g2);

 	u8g2_SetFontMode(&u8g2, 0);
	u8g2_SetDrawColor(&u8g2, 1);
	u8g2_SetFont(&u8g2,u8g2_font_micro_mr);
	if (current_page == 1)
	{
		if (unit == 0)
		{
			sprintf(buffer, "%0.3f", read);
			u8g2_DrawStr(&u8g2, 8, row, buffer);
		} else
		{
			read_temp = steinhart_equation(read);
			sprintf(buffer, "%0.3f", read_temp);
			u8g2_DrawStr(&u8g2, 8, row, buffer);
		}
	} else
	{
		sprintf(buffer, "%0.3f", read);
		u8g2_DrawStr(&u8g2, 8, row, buffer);
	}
	u8g2_SendBuffer(&u8g2);
}

void update_htv(int row, float htv)
{
	char buffer[128];

 	u8g2_SetFontMode(&u8g2, 1);
	u8g2_SetDrawColor(&u8g2, 0);
	u8g2_DrawBox(&u8g2, 48, (row - 7), 24, 9);
	u8g2_SendBuffer(&u8g2);

 	u8g2_SetFontMode(&u8g2, 0);
	u8g2_SetDrawColor(&u8g2, 1);
	u8g2_SetFont(&u8g2,u8g2_font_micro_mr);
	sprintf(buffer, "%0.3f", htv);
	u8g2_DrawStr(&u8g2, 48, row, buffer);
}

void update_ltv(int row, float ltv)
{
	char buffer[128];

 	u8g2_SetFontMode(&u8g2, 1);
	u8g2_SetDrawColor(&u8g2, 0);
	u8g2_DrawBox(&u8g2, 75, (row - 7), 24, 9);
	u8g2_SendBuffer(&u8g2);

 	u8g2_SetFontMode(&u8g2, 0);
	u8g2_SetDrawColor(&u8g2, 1);
	u8g2_SetFont(&u8g2,u8g2_font_micro_mr);
	sprintf(buffer, "%0.3f", ltv);
	u8g2_DrawStr(&u8g2, 75, row, buffer);
}

void update_range(int row, uint8_t range, uint8_t unit)
{
 	u8g2_SetFontMode(&u8g2, 1);
	u8g2_SetDrawColor(&u8g2, 0);
	u8g2_DrawBox(&u8g2, 102, (row - 7), 11, 9);
	u8g2_SendBuffer(&u8g2);

 	u8g2_SetFontMode(&u8g2, 0);
	u8g2_SetDrawColor(&u8g2, 1);
	u8g2_SetFont(&u8g2,u8g2_font_micro_mr);
	if (unit == 0)
	{
		if (range == 1)
		{
			u8g2_DrawStr(&u8g2, 107, row, "1");
		} else if (range == 10)
		{
			u8g2_DrawStr(&u8g2, 104, row, "10");
		}
	} else {
		if (range == 1)
		{
			u8g2_DrawStr(&u8g2, 104, row, "10");
		} else if (range == 10)
		{
			u8g2_DrawStr(&u8g2, 102, row, "200");
		}
	}
}

void update_status(int row, uint8_t alarm)
{
 	u8g2_SetFontMode(&u8g2, 1);
	u8g2_SetDrawColor(&u8g2, 0);
	u8g2_DrawBox(&u8g2, 116, (row - 7), 11, 9);
	u8g2_SendBuffer(&u8g2);

 	u8g2_SetFontMode(&u8g2, 0);
	u8g2_SetDrawColor(&u8g2, 1);
	u8g2_SetFont(&u8g2,u8g2_font_micro_mr);
	if (alarm == 1)
	{
		u8g2_DrawStr(&u8g2, 116, row, "DIS");
	} else if (alarm == 2)
	{
		u8g2_DrawStr(&u8g2, 116, row, "LIV");
	} else if (alarm == 3)
	{
		u8g2_DrawStr(&u8g2, 116, row, "LAT");
	}
}

void update_unit(int row, uint8_t unit)
{
	if (unit == 0)
	{
		u8g2_SetFont(&u8g2,u8g2_font_micro_mr);
		u8g2_DrawStr(&u8g2, 42, row, "V");
	} else
	{
		u8g2_SetFont(&u8g2,u8g2_font_micro_mr);
		u8g2_DrawStr(&u8g2, 42, row, "C");
		u8g2_SetFont(&u8g2,u8g2_font_3x3basic_tr);
		u8g2_DrawGlyph(&u8g2, 38, row, 48);
	}
}

void print_channel(int channel)
{
	if (channel == 1)
	{
		int row = 15;
		update_read(row, read_1, unit_1);
		update_htv(row, htv_1);
		update_ltv(row, ltv_1);
		update_range(row, range_1, unit_1);
		update_status(row, alarm_1);

	} else if (channel == 2)
	{
		int row = 27;
		update_read(row, read_2, unit_2);
		update_htv(row, htv_2);
		update_ltv(row, ltv_2);
		update_range(row, range_2, unit_2);
		update_status(row, alarm_2);

	} else if (channel == 3)
	{
		int row = 39;
		update_read(row, read_3, unit_3);
		update_htv(row, htv_3);
		update_ltv(row, ltv_3);
		update_range(row, range_3, unit_3);
		update_status(row, alarm_3);

	} else if (channel == 4)
	{
		int row = 51;
		update_read(row, read_4, unit_4);
		update_htv(row, htv_4);
		update_ltv(row, ltv_4);
		update_range(row, range_4, unit_4);
		update_status(row, alarm_4);

	} else if (channel == 5)
	{
		int row = 15;
		update_read(row, read_5, 0);
		update_htv(row, htv_5);
		update_ltv(row, ltv_5);
		update_range(row, range_5, unit_1);
		update_status(row, alarm_5);

	} else if (channel == 6)
	{
		int row = 27;
		update_read(row, read_6, 0);
		update_htv(row, htv_6);
		update_ltv(row, ltv_6);
		update_range(row, range_6, unit_2);
		update_status(row, alarm_6);

	} else if (channel == 7)
	{
		int row = 39;
		update_read(row, read_7, 0);
		update_htv(row, htv_7);
		update_ltv(row, ltv_7);
		update_range(row, range_7, unit_3);
		update_status(row, alarm_7);

	} else if (channel == 8)
	{
		int row = 51;
		update_read(row, read_8, 0);
		update_htv(row, htv_8);
		update_ltv(row, ltv_8);
		update_range(row, range_8, unit_4);
		update_status(row, alarm_8);

	}
}

void sd_record()
{
	if (current_record == 1)
	{
		u8g2_SetFontMode(&u8g2, 1);
		u8g2_SetDrawColor(&u8g2, 0);
		u8g2_SetFont(&u8g2, u8g2_font_8x13_mf);
		u8g2_DrawGlyph(&u8g2, 118, 67, 34);
		u8g2_SendBuffer(&u8g2);

		u8g2_SetFontMode(&u8g2, 0);
		u8g2_SetDrawColor(&u8g2, 1);
		u8g2_SetFont(&u8g2, u8g2_font_6x12_m_symbols);
		u8g2_DrawGlyph(&u8g2, 120, 63, 9656);
		u8g2_DrawHLine(&u8g2, 0,  54 , 128);
		u8g2_SendBuffer(&u8g2);
	} else
	{
		u8g2_SetFontMode(&u8g2, 1);
		u8g2_SetDrawColor(&u8g2, 0);
		u8g2_SetFont(&u8g2, u8g2_font_6x12_m_symbols);
		u8g2_DrawGlyph(&u8g2, 120, 63, 9656);
		u8g2_SendBuffer(&u8g2);

		u8g2_SetFontMode(&u8g2, 0);
		u8g2_SetDrawColor(&u8g2, 1);
		u8g2_SetFont(&u8g2, u8g2_font_8x13_mf);
		u8g2_DrawGlyph(&u8g2, 118, 67, 34);
		u8g2_SendBuffer(&u8g2);
	}
}

void update_time(char time[20])
{
	u8g2_SetFontMode(&u8g2, 0);
	u8g2_SetDrawColor(&u8g2, 1);
	u8g2_SetFont(&u8g2,u8g2_font_micro_mr);
	u8g2_DrawStr(&u8g2, 1, 62, time);
	u8g2_SendBuffer(&u8g2);
}

void update_sensor()
{
	u8g2_SetFontMode(&u8g2, 0);
	u8g2_SetDrawColor(&u8g2, 1);
	u8g2_SetFont(&u8g2, u8g2_font_6x12_m_symbols);
	u8g2_DrawGlyph(&u8g2, 5, 34, 10003);
	u8g2_SetFont(&u8g2,u8g2_font_micro_mr);
	u8g2_DrawStr(&u8g2, 2, 25, "S/U");
	u8g2_DrawHLine(&u8g2, 0,  36 , 14);
	u8g2_SendBuffer(&u8g2);
}

void page_one_unit()
{
	u8g2_SetFontMode(&u8g2, 0);
	u8g2_SetDrawColor(&u8g2, 1);
	if (unit_1 == 0)
	{
		u8g2_SetFont(&u8g2,u8g2_font_micro_mr);
		u8g2_DrawStr(&u8g2, 42, 15, "V");
	} else
	{
		u8g2_SetFont(&u8g2,u8g2_font_micro_mr);
		u8g2_DrawStr(&u8g2, 42, 15, "C");
		u8g2_SetFont(&u8g2,u8g2_font_3x3basic_tr);
		u8g2_DrawGlyph(&u8g2, 38, 12, 48);
	}
	if (unit_2 == 0)
	{
		u8g2_SetFont(&u8g2,u8g2_font_micro_mr);
		u8g2_DrawStr(&u8g2, 42, 27, "V");
	} else
	{
		u8g2_SetFont(&u8g2,u8g2_font_micro_mr);
		u8g2_DrawStr(&u8g2, 42, 27, "C");
		u8g2_SetFont(&u8g2,u8g2_font_3x3basic_tr);
		u8g2_DrawGlyph(&u8g2, 38, 24, 48);
	}
	if (unit_3 == 0)
	{
		u8g2_SetFont(&u8g2,u8g2_font_micro_mr);
		u8g2_DrawStr(&u8g2, 42, 39, "V");
	} else {
		u8g2_SetFont(&u8g2,u8g2_font_micro_mr);
		u8g2_DrawStr(&u8g2, 42, 39, "C");
		u8g2_SetFont(&u8g2,u8g2_font_3x3basic_tr);
		u8g2_DrawGlyph(&u8g2, 38, 36, 48);
	}
	if (unit_4 == 0)
	{
		u8g2_SetFont(&u8g2,u8g2_font_micro_mr);
		u8g2_DrawStr(&u8g2, 42, 51, "V");
	} else {
		u8g2_SetFont(&u8g2,u8g2_font_micro_mr);
		u8g2_DrawStr(&u8g2, 42, 51, "C");
		u8g2_SetFont(&u8g2,u8g2_font_3x3basic_tr);
		u8g2_DrawGlyph(&u8g2, 38, 48, 48);
	}
    u8g2_SendBuffer(&u8g2);
}

void unit_clear()
{
	u8g2_SetFontMode(&u8g2, 1);
	u8g2_SetDrawColor(&u8g2, 0);
	u8g2_DrawBox(&u8g2, 36, 8, 9, 9);
	u8g2_DrawBox(&u8g2, 36, 20, 9, 9);
	u8g2_DrawBox(&u8g2, 36, 32, 9, 9);
	u8g2_DrawBox(&u8g2, 36, 44, 9, 9);
    u8g2_SendBuffer(&u8g2);
}

void page_two_unit()
{
	u8g2_SetFontMode(&u8g2, 0);
	u8g2_SetDrawColor(&u8g2, 1);
	u8g2_SetFont(&u8g2,u8g2_font_micro_mr);
	u8g2_DrawStr(&u8g2, 2, 15, "5");
	u8g2_DrawStr(&u8g2, 2, 27, "6");
	u8g2_DrawStr(&u8g2, 2, 39, "7");
	u8g2_DrawStr(&u8g2, 2, 51, "8");
	u8g2_DrawStr(&u8g2, 36, 12, "m");
	u8g2_DrawStr(&u8g2, 39, 14, "/");
	u8g2_DrawStr(&u8g2, 41, 17, "s");
	u8g2_DrawStr(&u8g2, 36, 24, "m");
	u8g2_DrawStr(&u8g2, 39, 26, "/");
	u8g2_DrawStr(&u8g2, 41, 29, "s");
	u8g2_DrawStr(&u8g2, 36, 36, "m");
	u8g2_DrawStr(&u8g2, 39, 38, "/");
	u8g2_DrawStr(&u8g2, 41, 41, "s");
	u8g2_DrawStr(&u8g2, 42, 51, "C");
	u8g2_SetFont(&u8g2,u8g2_font_3x3basic_tr);
	u8g2_DrawGlyph(&u8g2, 38, 48, 48);
	u8g2_SetFont(&u8g2,u8g2_font_minimal3x3_tu);
	u8g2_DrawStr(&u8g2, 43, 13, "2");
	u8g2_DrawStr(&u8g2, 43, 25, "2");
	u8g2_DrawStr(&u8g2, 43, 37, "2");
    u8g2_SendBuffer(&u8g2);
}

void page_one()
{
	unit_clear();

	u8g2_SetFontMode(&u8g2, 0);
	u8g2_SetDrawColor(&u8g2, 1);
	u8g2_SetFont(&u8g2,u8g2_font_micro_mr);
	u8g2_DrawStr(&u8g2, 2, 15, "1");
	u8g2_DrawStr(&u8g2, 2, 27, "2");
	u8g2_DrawStr(&u8g2, 2, 39, "3");
	u8g2_DrawStr(&u8g2, 2, 51, "4");
    u8g2_SendBuffer(&u8g2);

	page_one_unit();
    print_channel(1);
    print_channel(2);
    print_channel(3);
    print_channel(4);
}

void page_two()
{
	u8g2_SetFontMode(&u8g2, 0);
	u8g2_SetDrawColor(&u8g2, 1);
	u8g2_SetFont(&u8g2,u8g2_font_micro_mr);
	u8g2_DrawStr(&u8g2, 2, 15, "5");
	u8g2_DrawStr(&u8g2, 2, 27, "6");
	u8g2_DrawStr(&u8g2, 2, 39, "7");
	u8g2_DrawStr(&u8g2, 2, 51, "8");
    u8g2_SendBuffer(&u8g2);

	page_two_unit();
    print_channel(5);
    print_channel(6);
    print_channel(7);
    print_channel(8);
}

void display_channel()
{
	int display_char = current_channel + 48;
	u8g2_SetFontMode(&u8g2, 0);
	u8g2_SetDrawColor(&u8g2, 1);
	u8g2_SetFont(&u8g2,u8g2_font_micro_mr);
	u8g2_DrawGlyph(&u8g2, 95, 62, display_char);
	u8g2_SendBuffer(&u8g2);
}

void display_alarm()
{
	u8g2_SetFontMode(&u8g2, 0);
	u8g2_SetDrawColor(&u8g2, 1);
	u8g2_SetFont(&u8g2,u8g2_font_micro_mr);

	u8g2_DrawStr(&u8g2, 108, 62, "!");

	u8g2_SendBuffer(&u8g2);
}

void display_connect(int connect)
{
	if (connect == 1)
	{
		u8g2_SetFontMode(&u8g2, 0);
		u8g2_SetDrawColor(&u8g2, 1);
		u8g2_SetFont(&u8g2, u8g2_font_6x12_m_symbols);
		u8g2_DrawGlyph(&u8g2, 13, 6, 8680);
		u8g2_DrawHLine(&u8g2, 0, 6, 128);
		u8g2_SendBuffer(&u8g2);// Send buffer to display
	} else {
		u8g2_SetFontMode(&u8g2, 1);
		u8g2_SetDrawColor(&u8g2, 0);
		u8g2_SetFont(&u8g2, u8g2_font_6x12_m_symbols);
		u8g2_DrawGlyph(&u8g2, 13, 6, 8680);
		u8g2_SendBuffer(&u8g2);// Send buffer to display
		u8g2_SetFontMode(&u8g2, 0);
		u8g2_SetDrawColor(&u8g2, 1);
		u8g2_SendBuffer(&u8g2);// Send buffer to display
	}
}

void switch_page()
{

	if (current_page != previous_page)
	{

		if (current_page == 1)
		{
			page_one();
			current_channel = 1;
		} else
		{
			page_two();
			current_channel = 5;
		}

		previous_page = current_page;
	}
}

void print_value(uint32_t lastprintTime)
{
	  uint32_t now = HAL_GetTick();

	  if (now - lastprintTime < 200) return;  // Debounce: ignore if <200ms since last press
	  lastprintTime = now;

//	  char buffer1[128];
//	  char buffer2[128];
//	  char buffer3[128];
//	  char buffer4[128];
//	  char buffer5[128];
//	  char buffer6[128];
//	  char buffer7[128];
//	  char buffer8[128];
//
//	  sprintf(buffer1, "%lu", read_1);
//	  sprintf(buffer2, "%lu", read_2);
//	  sprintf(buffer3, "%lu", read_3);
//	  sprintf(buffer4, "%lu", read_4);
//	  sprintf(buffer5, "%lu", read_5);
//	  sprintf(buffer6, "%lu", read_6);
//	  sprintf(buffer7, "%lu", read_7);
//	  sprintf(buffer8, "%lu", read_8);

	  if (current_record == 1){

		  write_csv(read_1, read_2, read_3, read_4, read_5, read_6, read_7, read_8);
	  }
}
