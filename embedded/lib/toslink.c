/** 
**************************************************************
* @file embedded/lib/toslink.c
* @author Shane Baptist - 47484164
* @date 17/05/2025
* @brief Bidirectional communication protocol driver
*************************************************************** 
*/

#include <stdint.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/atomic.h>
#include "toslink.h"
#include "alarm.h"

// the holy grail
static char myTurn = 1;

// rx payload index
static int rxCount = 0;

// global tx buffer
uint8_t txBuf[PAYLOAD_SIZE] = {0};

// incoming rx buffer
uint8_t rxBuf[INCOMING_PAYLOAD_SIZE] = {0};

// comm sampling led timeout var
static int64_t samplingTick;

// Define threads
K_THREAD_DEFINE(tx_tid, TX_STACK_SIZE, tx_thread, NULL, NULL, NULL, TX_THREAD_PRIORITY, 0, 0);
//K_THREAD_DEFINE(rx_tid, RX_STACK_SIZE, rx_thread, NULL, NULL, NULL, RX_THREAD_PRIORITY, 0, 0);

// Define message queues
K_MSGQ_DEFINE(range_msgq, sizeof(uint8_t), 1, 1);
K_MSGQ_DEFINE(high_msgq, sizeof(uint8_t), 1, 1);
K_MSGQ_DEFINE(low_msgq, sizeof(uint8_t), 1, 1);

// Data sharing primitives to prevent race conditions
struct k_mutex tx_mutex;
//struct k_mutex rx_mutex;

// Atomic variables for volatile interrupt data
atomic_t  rangeMask = 0x00;
atomic_t  highMask = 0x00;
atomic_t  lowMask = 0x00;

// get uart device
static const struct device *const uart_dev = DEVICE_DT_GET(DT_NODELABEL(lpuart1));

// get sampling led device
static const struct gpio_dt_spec samplingLED = GPIO_DT_SPEC_GET(DT_NODELABEL(sampling), gpios);

// translate range buffer into bitmask
uint8_t get_range(uint8_t* buf) {
    uint8_t mask = 0;
    for (int i = 0; i < 4; i++) {
        mask |= (buf[i] & 0x01) << i;
    }
    return mask;
}

// translate high alarm buffer into bitmask
uint8_t get_high(uint8_t* buf) {
    uint8_t mask = 0;
    for (int i = 0; i < 8; i++) {
        mask |= (buf[i + 4] & 0x01) << i;
    }
    return mask;
}

// translate low alarm buffer into bitmask
uint8_t get_low(uint8_t* buf) {
    uint8_t mask = 0;
    for (int i = 0; i < 8; i++) {
        mask |= (buf[i + 12] & 0x01) << i;
    }
    return mask;
}

// UART CALLBACK FUNCTION
void uart_callback(const struct device *dev, void *user_data) {
	ARG_UNUSED(user_data);

	/* Verify uart_irq_update() */
	if (!uart_irq_update(dev)) {
		return;
	}

    // var to store received byte
    char uart_char;

	if (uart_irq_rx_ready(dev)) {
        // write received data into uart_char
        uart_fifo_read(dev, &uart_char, 1);
        //printk("ISR myTurn: %d\r\n", myTurn);
/*         if (uart_char == 0x77) {
            atomic_set(&highMask, 0xFF); 
            atomic_set(&lowMask, 0xFF); 
            myTurn = 1;
        }
        if (uart_char == 0x55) {
            atomic_set(&highMask, 0x55); 
            atomic_set(&lowMask, 0x55); 
            myTurn = 1;
        } */
        // make sure its not my turn
        if (!myTurn) {
            // check for header
            if (uart_char == CONTROL_HEADER) {
        	    samplingTick = k_uptime_get();
                rxCount = 0;
                atomic_set(&highMask, 0xFF); 
                atomic_set(&lowMask, 0xFF); 
                return;
            } else if (uart_char == CONTROL_STOP) {
                //k_mutex_lock(&rx_mutex, K_FOREVER);
                // share received settings with rx thread
                atomic_set(&rangeMask, get_range(rxBuf));
                atomic_set(&highMask, get_high(rxBuf)); 
                atomic_set(&lowMask, get_low(rxBuf)); 

                // Reset payload index
                rxCount = 0;
                // Enter transmission state
                myTurn = 1;
                //k_mutex_unlock(&rx_mutex);
                return;
            } else {
                // add to rx buffer to be processed
                rxBuf[rxCount] = uart_char;
                rxCount++;
            }
        }
	}
/*     // empty fifo
    while (uart_fifo_read(dev, &uart_char, 1)) {
        printk("emptying fifo\r\n");
    } */
}
/* 
// function to transmit char buffer char by char
void print_uart(uint8_t* buf) {
	for (int i = 0; i < PAYLOAD_SIZE; i++) {
		uart_poll_out(uart_dev, buf[i]);
        //k_usleep(TX_DELAY);
	}
} */

// TX thread
void tx_thread() {
    //k_mutex_init(&tx_mutex);
    // TX timeout to reinitiate comms
	int64_t prevTick = k_uptime_get();
    
    while (1) {
/*         atomic_set(&highMask, 0x00); 
        atomic_set(&lowMask, 0x00);  */
        //printk("TX myTurn: %d\r\n", myTurn);
        //if (myTurn) {   
            //
            // k_mutex_lock(&tx_mutex, K_FOREVER);
            //k_sched_lock();

            // header
            uart_poll_out(uart_dev, SENSOR_HEADER);
            //k_usleep(TX_DELAY);

            // send data buffer
            print_uart(txBuf);

            // send stop byte
            uart_poll_out(uart_dev, SENSOR_STOP);
            //k_usleep(TX_DELAY);

            // hand over
			//prevTick = k_uptime_get();
            //myTurn = 0;
            //k_sched_unlock();
            //k_mutex_unlock(&tx_mutex);
        //} else {
            // check timeout
/*             if (k_uptime_get() - prevTick > RX_TIMEOUT) {
                myTurn = 1; // reclaim turn after timeout
                prevTick = k_uptime_get(); // reset timeout reference
            }
//} */
            k_usleep(1000);
    }
}

// RX thread
void rx_thread() {
    // Set rx interrupt callback
	uart_irq_callback_set(uart_dev, uart_callback);

    // Enable rx
	uart_irq_rx_enable(uart_dev);

    // Init sampling led
	gpio_pin_configure_dt(&samplingLED, GPIO_OUTPUT_INACTIVE);

    //
    // k_mutex_init(&rx_mutex);

    // temp variables
    uint8_t range = 0x00;
    uint8_t high = 0x00;
    uint8_t low = 0x00;

    // sampling led flash at visible rate
    uint64_t readableTick = k_uptime_get();

    // check sampling led device ready
    if (!device_is_ready(samplingLED.port)) {
        printk("LED device not ready!\n");
    }

    while (1) {
        // prevent race condition
        
        //k_mutex_lock(&rx_mutex, K_FOREVER);
        k_sched_lock();

        // get value from interrupt
        range = atomic_get(&rangeMask);
        high = atomic_get(&highMask);
        low = atomic_get(&lowMask);

        // put in message queues
        k_msgq_purge(&range_msgq);
        k_msgq_put(&range_msgq, &range, K_NO_WAIT);

        k_msgq_purge(&high_msgq);
        k_msgq_put(&high_msgq, &high, K_NO_WAIT);

        k_msgq_purge(&low_msgq);
        k_msgq_put(&low_msgq, &low, K_NO_WAIT);

        k_sched_unlock();
       // k_mutex_unlock(&rx_mutex);
        
        // Check sampling led 
        if (k_uptime_get() - samplingTick > 200) {
            // if more than 200ms, turn off sampling led
            gpio_pin_set_dt(&samplingLED, 0);
        } else {
            // flash sampling led every 150ms
            if (k_uptime_get() - readableTick > 150) {
                gpio_pin_toggle_dt(&samplingLED);
                readableTick = k_uptime_get(); // reset timeout reference
            }
        }
        k_msleep(10);
    }
}