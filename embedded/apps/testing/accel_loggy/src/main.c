/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/devicetree.h>
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
 
 static void fetch_and_display(const struct device *sensor)
 {
	 static unsigned int count;
	 struct sensor_value accel[3];
	 struct sensor_value temperature;
	 const char *overrun = "";
	 int rc = sensor_sample_fetch(sensor);
 
	 ++count;
	 if (rc == -EBADMSG) {
		 /* Sample overrun.  Ignore in polled mode. */
		 if (IS_ENABLED(CONFIG_LIS2DH_TRIGGER)) {
			 overrun = "[OVERRUN] ";
		 }
		 rc = 0;
	 }
	 if (rc == 0) {
		 rc = sensor_channel_get(sensor,
					 SENSOR_CHAN_ACCEL_XYZ,
					 accel);
	 }
	 if (rc < 0) {
		 printk("ERROR: Update failed: %d\n", rc);
	 } else {
		 printk("#%u @ %u ms: %sx %f , y %f , z %f",
				count, k_uptime_get_32(), overrun,
				sensor_value_to_double(&accel[0]),
				sensor_value_to_double(&accel[1]),
				sensor_value_to_double(&accel[2]));
	 }
 
	 if (IS_ENABLED(CONFIG_LIS2DH_MEASURE_TEMPERATURE)) {
		 if (rc == 0) {
			 rc = sensor_channel_get(sensor, SENSOR_CHAN_DIE_TEMP, &temperature);
			 if (rc < 0) {
				 printk("\nERROR: Unable to read temperature:%d\n", rc);
			 } else {
				 printk(", t %f\n", sensor_value_to_double(&temperature));
			 }
		 }
 
	 } else {
		 printk("\n");
	 }
 }
 
 int main(void)
 {

	gpio_pin_configure_dt(&alarm_in, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&alarm_clk, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&alarm_latch, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&alarm_enable, GPIO_OUTPUT_INACTIVE);
	 const struct device *const sensor = DEVICE_DT_GET(DT_NODELABEL(accel));
 
	 if (sensor == NULL) {
		 printk("No device found\n");
		 return 0;
	 }
	 if (!device_is_ready(sensor)) {
		 printk("Device %s is not ready\n", sensor->name);
		 return 0;
	 }

	 printk("Polling at 0.5 Hz\n");
	 while (true) {
		 fetch_and_display(sensor);
		 k_msleep(200);
	 }
 }
 