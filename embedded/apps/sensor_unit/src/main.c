/** 
**************************************************************
* @file embedded/apps/loggy/src/main.c
* @author Shane Baptist - 47484164
* @date 17/05/2025
* @brief Loggy main controlling thread
*************************************************************** 
*/

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "analog_circuitry.h"
#include "lis3dh.h"
#include "temp_sensor.h"
#include "toslink.h"
#include "alarm.h"
#include <zephyr/drivers/uart.h>


// TODO: WAYYYY TOO SLOW
#define SAMPLING_DELAY	100

// control stack size
#define CONTROL_STACK_SIZE	1024
// control thread priority
#define CONTROL_THREAD_PRIORITY 1

// control thread function prototype
void control_thread();

// Define control thread
K_THREAD_DEFINE(control_tid, CONTROL_STACK_SIZE, control_thread, NULL, NULL, NULL, CONTROL_THREAD_PRIORITY, 0, 0);

static const struct device *const uart_dev = DEVICE_DT_GET(DT_NODELABEL(lpuart1));


// Separate uint32 into 4 uint8s, place in variable place in buffer
void separate_u32(uint32_t x, uint8_t* buf, size_t pos) {
	// check if valid buffer insertion
	if (pos > 36) {
		printk("Memory access violation, accessing buffer out of range.\r\n");
		return;
	}
	buf[pos] = (uint8_t)((x >> 24) & 0x000000FF);
	buf[pos + 1] = (uint8_t)((x >> 16) & 0x000000FF);
	buf[pos + 2] = (uint8_t)((x >> 8) & 0x000000FF);
	buf[pos + 3] = (uint8_t)(x & 0x000000FF);
}
// function to transmit char buffer char by char
void print_uart(uint8_t* buf) {
	for (int i = 0; i < PAYLOAD_SIZE; i++) {
		uart_poll_out(uart_dev, buf[i]);
		k_usleep(6000);

	}
}

// Controlling thread
void control_thread() {
	// array to store channel readings
	static int32_t chanReading[8];

	while (1) {
		//TODO: remove debug prints
		// Get channels 1-4 readings
		for (int i = 0; i < 4; i++) {
			k_msgq_peek(chanQPointers[i], &(chanReading[i]));
			printk("Channel %d: %d	mV\r\n", (i + 1), chanReading[i]);
		}

		// Get channels 5-7 readings
		k_msgq_peek(&accelX_msgq, &(chanReading[4]));
		k_msgq_peek(&accelY_msgq, &(chanReading[5]));
		k_msgq_peek(&accelZ_msgq, &(chanReading[6]));
		printk("Channel 5: %d	mm/s^2\r\n", chanReading[4]);
		printk("Channel 6: %d	mm/s^2\r\n", chanReading[5]);
		printk("Channel 7: %d	mm/s^2\r\n", chanReading[6]);

		// Get channel 8 reading
		k_msgq_peek(&temp_msgq, &(chanReading[7]));
		printk("Channel 8: %d	\u00b0C\r\n", chanReading[7]);
		printk("==================================================\r\n");

		k_sched_lock();
		// prevent race condition
		k_mutex_lock(&tx_mutex, K_FOREVER);

		// Place data in tx buffer to be transmitted
		for (int i = 0; i < 8; i++) {
 			separate_u32(chanReading[i], txBuf, (i * 4));
		}

		uart_poll_out(uart_dev, 0x77);
		k_usleep(6000);

		// send data buffer
		print_uart(txBuf);

		// send stop byte
		uart_poll_out(uart_dev, 0x55);
		k_usleep(6000);


		//k_mutex_unlock(&tx_mutex);

		k_sched_unlock();

		// thread delay
		k_msleep(SAMPLING_DELAY);
	}
}