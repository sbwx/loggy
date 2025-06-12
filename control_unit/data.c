/*
 * data.c
 *
 *  Created on: Apr 4, 2025
 *      Author: ching
 */

#include "data.h"
#include <math.h>

extern uint8_t current_page;
extern uint8_t current_range;
extern uint8_t current_channel;
extern uint8_t current_alarm;
extern float read_1, read_2, read_3, read_4, read_5, read_6, read_7, read_8;
extern float htv_1, htv_2, htv_3, htv_4, htv_5, htv_6, htv_7, htv_8;
extern float ltv_1, ltv_2, ltv_3, ltv_4, ltv_5, ltv_6, ltv_7, ltv_8;
extern uint8_t range_1, range_2, range_3, range_4, range_5, range_6, range_7, range_8;
extern uint8_t alarm_1h, alarm_2h, alarm_3h, alarm_4h, alarm_5h, alarm_6h, alarm_7h, alarm_8h;
extern uint8_t alarm_1l, alarm_2l, alarm_3l, alarm_4l, alarm_5l, alarm_6l, alarm_7l, alarm_8l;
extern uint8_t alarm_1, alarm_2, alarm_3, alarm_4, alarm_5, alarm_6, alarm_7, alarm_8;

void value_update(uint16_t channel, uint16_t item, uint16_t value)
{

	if (channel == 1)
	{
		if (item == 1) // range
		{
			range_1 = value;
			range_5 = value;
		} else if (item == 2) // alarm
		{
			alarm_1 = value;
		}
	} else if (channel == 2)
	{
		if (item == 1) // range
		{
			range_2 = value;
			range_6 = value;
		} else if (item == 2) // alarm
		{
			alarm_2 = value;
		}
	} else if (channel == 3)
	{
		if (item == 1) // range
		{
			range_3 = value;
			range_7 = value;
		} else if (item == 2) // alarm
		{
			alarm_3 = value;
		}
	} else if (channel == 4)
	{
		if (item == 1) // range
		{
			range_4 = value;
			range_8 = value;
		} else if (item == 2) // alarm
		{
			alarm_4 = value;
		}
	} else if (channel == 5)
	{
		if (item == 1) // range
		{
			range_1 = value;
			range_5 = value;
		} else if (item == 2) // alarm
		{
			alarm_5 = value;
		}
	} else if (channel == 6)
	{
		if (item == 1) // range
		{
			range_2 = value;
			range_6 = value;
		} else if (item == 2) // alarm
		{
			alarm_6 = value;
		}
	} else if (channel == 7)
	{
		if (item == 1) // range
		{
			range_3 = value;
			range_7 = value;
		} else if (item == 2) // alarm
		{
			alarm_7 = value;
		}
	} else if (channel == 8)
	{
		if (item == 1) // range
		{
			range_4 = value;
			range_8 = value;
		} else if (item == 2) // alarm
		{
			alarm_8 = value;
		}
	}
}

void channel_update()
{
	if (current_channel == 1)
	{
		current_channel = 2;
		current_range = 1;
		current_alarm = 3;
	} else if (current_channel == 2)
	{
		current_channel = 3;
		current_range = 1;
		current_alarm = 3;
	} else if (current_channel == 3)
	{
		current_channel = 4;
		current_range = 1;
		current_alarm = 3;
	} else if (current_channel == 4)
	{
		current_channel = 1;
		current_range = 1;
		current_alarm = 3;
	} else if (current_channel == 5)
	{
		current_channel = 6;
		current_range = 1;
		current_alarm = 3;
	} else if (current_channel == 6)
	{
		current_channel = 7;
		current_range = 1;
		current_alarm = 3;
	} else if (current_channel == 7)
	{
		current_channel = 8;
		current_range = 1;
		current_alarm = 3;
	} else if (current_channel == 8)
	{
		current_channel = 5;
		current_range = 1;
		current_alarm = 3;
	}
}

float steinhart_equation(float r) {
	const float A = 3.354016E-3;
	const float B = 2.569850E-4;
	const float C = 2.620131E-6;
	const float D = 6.383091E-8;
	const float R0 = 10000.0;

	float tempK = (1 / (A + (B * log(r/R0)) + (C * pow(log(r/R0), 2)) + (D * pow(log(r/R0), 3))));
	float tempC = tempK - 273.15;
	return -tempC;
}

void check_alarm()
{
	// channel 1
	if (read_1 > htv_1)
	{
		alarm_1h = 1;
	} else
	{
		alarm_1h = 0;
	}

	if (read_1 < ltv_1)
	{
		alarm_1l = 1;
	} else
	{
		alarm_1l = 0;
	}

	// channel 2
	if (read_2 > htv_2)
	{
		alarm_2h = 1;
	}  else
	{
		alarm_2h = 0;
	}

	if (read_2 < ltv_2)
	{
		alarm_2l = 1;
	} else
	{
		alarm_2l = 0;
	}

	// channel 3
	if (read_3 > htv_3)
	{
		alarm_3h = 1;
	} else
	{
		alarm_3h = 0;
	}

	if (read_3 < ltv_3)
	{
		alarm_3l = 1;
	} else
	{
		alarm_3l = 0;
	}

	// channel 4
	if (read_4 > htv_4)
	{
		alarm_4h = 1;
	} else
	{
		alarm_4h = 0;
	}

	if (read_4 < ltv_4)
	{
		alarm_4l = 1;
	} else
	{
		alarm_4l = 0;
	}

	// channel 5
	if (read_5 > htv_5)
	{
		alarm_5h = 1;
	} else
	{
		alarm_5h = 0;
	}

	if (read_5 < ltv_5)
	{
		alarm_5l = 1;
	} else
	{
		alarm_5l = 0;
	}

	// channel 6
	if (read_6 > htv_6)
	{
		alarm_6h = 1;
	} else
	{
		alarm_6h = 0;
	}

	if (read_6 < ltv_6)
	{
		alarm_6l = 1;
	} else
	{
		alarm_6l = 0;
	}

	// channel 7
	if (read_7 > htv_7)
	{
		alarm_7h = 1;
	} else
	{
		alarm_7h = 0;
	}

	if (read_7 < ltv_7)
	{
		alarm_7l = 1;
	} else
	{
		alarm_7l = 0;
	}

	// channel 8
	if (read_8 > htv_8)
	{
		alarm_8h = 1;
	} else
	{
		alarm_8h = 0;
	}

	if (read_8 < ltv_8)
	{
		alarm_8l = 1;
	} else
	{
		alarm_8l = 0;
	}
}
