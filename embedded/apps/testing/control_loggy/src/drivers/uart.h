/*
 * uart.h
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// buffer to store uart data
static char fifo_data[1] = {0};

extern uint8_t connectedStatus = 0;

/*
 * uart irq callback function
 */
static void uart_fifo_callback(const struct device *dev, void *user_data) {
	ARG_UNUSED(user_data);

	/* Verify uart_irq_update() */
	if (!uart_irq_update(dev)) {
		printf("retval should always be 1\n");
		return;
	}

	/* Verify uart_irq_rx_ready() */
	if (uart_irq_rx_ready(dev)) {
		/* Verify uart_fifo_read() */
		uart_fifo_read(dev, &fifo_data, 1);
        if (fifo_data[0] == '1') {
            connectedStatus = 1;
        } else if (fifo_data[0] == '2') {
            connectedStatus = 0;
        }
	}
}