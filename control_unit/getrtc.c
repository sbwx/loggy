/*
 * rtc.c
 *
 *  Created on: Apr 14, 2025
 *      Author: ching
 */
#include "rtc.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>

extern RTC_HandleTypeDef hrtc;

void get_time(char *datetime)
{
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;

    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);

    datetime[0] = '\0'; // Clear buffer

	sprintf(datetime, "%04d/%02d/%02d %02d:%02d:%02d",
				2000 + date.Year, date.Month, date.Date,
				time.Hours, time.Minutes, time.Seconds);
}

void get_time_print(char *datetime)
{
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;

    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);

    datetime[0] = '\0'; // Clear buffer

	int ms = (int)((255 - time.SubSeconds) * 1000 / 256);

	sprintf(datetime, "%04d-%02d-%02d_%02d-%02d-%02d.%03d",
				2000 + date.Year, date.Month, date.Date,
				time.Hours, time.Minutes, time.Seconds, ms);
}

void get_time_csv(char *datetime)
{
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;

	HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);

    datetime[0] = '\0'; // Clear buffer

	sprintf(datetime, "%04d-%02d-%02d_%02d-%02d-%02d",
				2000 + date.Year, date.Month, date.Date,
				time.Hours, time.Minutes, time.Seconds);
}
