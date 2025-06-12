/** 
**************************************************************
* @file embedded/lib/temp_sensor.h
* @author Shane Baptist - 47484164
* @date 17/05/2025
* @brief On-board temperature sensor driver (Channel 8)
*************************************************************** 
*/

#ifndef TEMP_SENSOR
#define TEMP_SENSOR

// Thread stack size
#define TEMP_STACK_SIZE 1024
// Thread priority
#define TEMP_THREAD_PRIORITY 8

// delay between samples in milliseconds
#define TEMP_SAMPLING_RATE 5

// Global message queue for temp sensor readings
extern struct k_msgq temp_msgq;

// Function prototypes
void temp_thread();

#endif