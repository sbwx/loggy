#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/gpio.h>

static const struct gpio_dt_spec alarm_in = GPIO_DT_SPEC_GET(DT_NODELABEL(alarm_in), gpios);
static const struct gpio_dt_spec alarm_clk = GPIO_DT_SPEC_GET(DT_NODELABEL(alarm_clk), gpios);
static const struct gpio_dt_spec alarm_latch = GPIO_DT_SPEC_GET(DT_NODELABEL(alarm_latch), gpios);
static const struct gpio_dt_spec alarm_enable = GPIO_DT_SPEC_GET(DT_NODELABEL(alarm_enable), gpios);

void set_alarms(uint16_t bitmask) {
	for (int i = 0; i < 16; i++) {
		gpio_pin_set_dt(&alarm_clk, 0);
		k_usleep(1);
		if (bitmask & (0x0001 << i)) {
			printk("LED %d ON\r\n", i);
			gpio_pin_set_dt(&alarm_in, 1);
		} else {
			gpio_pin_set_dt(&alarm_in, 0);
		}
		k_usleep(1);
		gpio_pin_set_dt(&alarm_clk, 1);
		k_usleep(1);
		gpio_pin_set_dt(&alarm_clk, 0);
	}
	gpio_pin_set_dt(&alarm_latch, 1);
	k_usleep(1);
	gpio_pin_set_dt(&alarm_latch, 0);
}

int main() {
	gpio_pin_configure_dt(&alarm_in, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&alarm_clk, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&alarm_latch, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&alarm_enable, GPIO_OUTPUT_INACTIVE);
	
	while (1) {
		for (int i = 0; i < 16; i++) {
			printk("Alarm bitmask:	%d\r\n", (0x0001 << i));
			set_alarms(0x0001 << i);
			k_msleep(500);
		}
	}
	return 0;
}