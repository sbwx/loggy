/** 
**************************************************************
* @file embedded/lib/alarm.h
* @author Shane Baptist - 47484164
* @date 17/05/2025
* @brief Alarm LED driver
*************************************************************** 
*/

#ifndef THRESH_ALARM
#define THRESH_ALARM

// Thread stack size
#define ALARM_STACK_SIZE 1024
// Thread priority
#define ALARM_THREAD_PRIORITY 9

// delay between samples in milliseconds
#define ALARM_SAMPLING_RATE 5

// Function prototypes
extern void set_alarms(uint16_t bitmask);
extern uint16_t merge_alarm_configs(uint8_t high, uint8_t low);
extern void alarm_thread();

#endif