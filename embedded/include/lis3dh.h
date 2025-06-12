/** 
**************************************************************
* @file embedded/lib/lis2dh.h
* @author Shane Baptist - 47484164
* @date 17/05/2025
* @brief Accelerometer driver (Channel 5-7)
*************************************************************** 
*/

#ifndef ACCELEROMETER
#define ACCELEROMETER

// Thread stack size
#define ACCEL_STACK_SIZE 1024
// Thread priority
#define ACCEL_THREAD_PRIORITY 7

// delay between samples in milliseconds
#define ACCEL_SAMPLING_RATE 5

// Global message queues for accelerometer channels
extern struct k_msgq accelX_msgq;
extern struct k_msgq accelY_msgq;
extern struct k_msgq accelZ_msgq;

// Function prototypes
void accel_thread();

#endif