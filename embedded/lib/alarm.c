/** 
**************************************************************
* @file embedded/lib/alarm.c
* @author Shane Baptist - 47484164
* @date 17/05/2025
* @brief Alarm LED driver
*************************************************************** 
*/

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include "alarm.h"
#include "toslink.h"

// Define threads
K_THREAD_DEFINE(alarm_tid, ALARM_STACK_SIZE, alarm_thread, NULL, NULL, NULL, ALARM_THREAD_PRIORITY, 0, 0);

// Get led driver gpio devices
static const struct gpio_dt_spec alarm_in = GPIO_DT_SPEC_GET(DT_NODELABEL(alarm_in), gpios);
static const struct gpio_dt_spec alarm_clk = GPIO_DT_SPEC_GET(DT_NODELABEL(alarm_clk), gpios);
static const struct gpio_dt_spec alarm_latch = GPIO_DT_SPEC_GET(DT_NODELABEL(alarm_latch), gpios);
static const struct gpio_dt_spec alarm_enable = GPIO_DT_SPEC_GET(DT_NODELABEL(alarm_enable), gpios);

// function to set leds using led driver (fancy shift register)
void set_alarms(uint16_t bitmask) {
	for (int i = 15; i > -1; i--) {
        // pull clock low
		gpio_pin_set_dt(&alarm_clk, 0);
		k_usleep(1);

        // set input depending on bit
        gpio_pin_set_dt(&alarm_in, (bitmask >> i) & 0x0001);  // MSB first
		k_usleep(1);

        // pull clock high
		gpio_pin_set_dt(&alarm_clk, 1);
		k_usleep(1);
        // reset clock to low
		gpio_pin_set_dt(&alarm_clk, 0);
	}
    // latch data into registers
	gpio_pin_set_dt(&alarm_latch, 1);
	k_usleep(1);
    // reset latch pin to 0
	gpio_pin_set_dt(&alarm_latch, 0);
}

// merges two 8-bit alarm configuration values (high and low) into a single 16-bit value.
// each bit from 'low' is placed at even positions (0, 2, 4, ...)
// each bit from 'high' is placed at odd positions (1, 3, 5, ...)
// resulting in an interleaved pattern: [h7 l7 h6 l6 ... h0 l0]
uint16_t merge_alarm_configs(uint8_t high, uint8_t low) {
    uint16_t merged = 0;
    for (int i = 0; i < 8; i++) {
        // extract bit i from low mask and place it at bit position (2 * i)
        merged |= (((uint16_t)((low >> i) & 0x01)) << (i << 1));
        // extract bit i from high mask and place it at bit position (2 * i + 1)
        merged |= (((uint16_t)((high >> i) & 0x01)) << ((i << 1) + 1));
    }
    return merged;
}

void alarm_thread() {
    // Initialise gpio pins
	gpio_pin_configure_dt(&alarm_in, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&alarm_clk, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&alarm_latch, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&alarm_enable, GPIO_OUTPUT_INACTIVE);

    // variable init
    uint8_t highConfig;
    uint8_t lowConfig;
    uint16_t alarmConfig;

    while (1) {
      // Get high and low alarm states  
        //k_mutex_lock(&rx_mutex, K_FOREVER);
        k_msgq_peek(&high_msgq, &highConfig);
        k_msgq_peek(&low_msgq, &lowConfig);
        //k_mutex_unlock(&rx_mutex);

        // combine into 16 bitmask
        alarmConfig = merge_alarm_configs(highConfig, lowConfig);
        // turn on/off alarms accordingly
        //printk("Alarm:  %d\r\n", alarmConfig);
        set_alarms(alarmConfig); 

        // Alarm update rate
        k_msleep(ALARM_SAMPLING_RATE);
    }
}