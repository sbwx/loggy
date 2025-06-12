/*
 * main.c
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include "drivers/st7920.h"
#include "drivers/sd.h"
#include <zephyr/drivers/uart.h>
#include <zephyr/random/random.h>
#include "drivers/uart.h"


// thread stack size
#define STACK_SIZE 4096

// random thread priority
#define RANDOM_THREAD_PRIORITY 6
// mount thread priority
#define MOUNT_THREAD_PRIORITY 7

// function prototypes
void lcd_thread();
void mount_thread();

// threads
K_THREAD_DEFINE(random_tid, STACK_SIZE, lcd_thread, NULL, NULL, NULL, RANDOM_THREAD_PRIORITY, 0, 0);
K_THREAD_DEFINE(mount_tid, STACK_SIZE, mount_thread, NULL, NULL, NULL, MOUNT_THREAD_PRIORITY, 0, 0);

// buffer to store data for each channel
static float fake_data[8];

// dt for uart
static struct device *const uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

/*
 * Returns a 'random' float
 */
float get_random_float() {
	uint32_t random;
	sys_rand_get(&random, sizeof(random));

	float scaled = ((float)random / (float)UINT32_MAX) * 20.0f - 10.0f;

	return ((int)(scaled * 1000.0f)) / 1000.0f;
}

/*
 * Populates global float buffer with 8 floats
 */
void get_random_buffer() {
    for (size_t i = 0; i < 8; i++) {
        fake_data[i] = get_random_float();
    }
}

/*
 * Test function that adds an entry to the csv file
 */
 void test_sd_card() {
	get_random_buffer();
	csv_add_entry(fake_data);
	k_msleep(10);
}

/*
 * Thread that constantly attempts to mount
 */
void mount_thread() {
	k_msleep(1000);
	csv_create();
	format_csv_header();
}

/*
 * Thread that initialises LCD, UART, basic display layout and constantly displays random floats on the screen
 */
void lcd_thread() {
	/* Verify uart_irq_callback_set() */
	uart_irq_callback_set(uart_dev, uart_fifo_callback);

	/* Enable Tx/Rx interrupt before using fifo */
	/* Verify uart_irq_rx_enable() */
	uart_irq_rx_enable(uart_dev);

	gpio_init();
	k_msleep(10);
	lcd_init();

	set_ddram_address(0, 0);
	write_str("CH1");
	
	set_ddram_address(1, 0);
	write_str("CH2");

	set_ddram_address(2, 0);
	write_str("CH3");

	set_ddram_address(3, 0);
	write_str("CH4");

	set_ddram_address(0, 6);
	write_str("V");
	
	set_ddram_address(1, 6);
	write_str("V");

	set_ddram_address(2, 6);
	write_str("V");

	set_ddram_address(3, 6);
	write_str("V");

	char s[8];
	while (1) {
		if (connectedStatus == 0) {
			set_ddram_address(0, 7);
			write_char(0x09);
		} else if (connectedStatus == 1) {
			set_ddram_address(0, 7);
			write_char(0x07);
		}

		get_random_buffer();
		for (int i = 0; i < 4; i++) {
			set_ddram_address(i, 2);
			write_str(" ");
			set_ddram_address(i, 2);
			sprintf(s, "%7.3f", fake_data[i]);
			write_str(s);
		}
		test_sd_card();
		k_msleep(250);
	}
}