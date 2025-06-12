/** 
**************************************************************
* @file embedded/lib/analog_circuitry.h
* @author Shane Baptist - 47484164
* @date 17/05/2025
* @brief Channel 1-4 voltage reading + range switching driver
*************************************************************** 
*/
#include <zephyr/kernel.h>
#include <stdint.h> 

#ifndef ANALOG_CIRCUITRY
#define ANALOG_CIRCUITRY

// Thread stack size
#define ADC_STACK_SIZE 1024
// Thread priority
#define ADC_THREAD_PRIORITY 6

// TODO: wayyyy to slow, hoping settling time is less on PCB
// delay between samples in milliseconds
#define MUX_SETTLING_TIME 30

// Global message queue pointer array for accessing channels 1-4 message queues
extern struct k_msgq* chanQPointers[4];

// Function prototypes
void init_mux();
void set_mux(uint8_t channel, uint8_t range);
void adc_thread();

#endif