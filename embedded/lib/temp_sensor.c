/** 
**************************************************************
* @file embedded/lib/temp_sensor.c
* @author Shane Baptist - 47484164
* @date 17/05/2025
* @brief On-board temperature sensor driver (Channel 8)
*************************************************************** 
*/

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/sensor.h>
#include "temp_sensor.h"

// Define threads
K_THREAD_DEFINE(temp_tid, TEMP_STACK_SIZE, temp_thread, NULL, NULL, NULL, TEMP_THREAD_PRIORITY, 0, 0);

// Define message queue
K_MSGQ_DEFINE(temp_msgq, sizeof(int32_t), 1, 1);

// Get dietemp device
static const struct device *const tempSensor = DEVICE_DT_GET(DT_NODELABEL(die_temp));

void temp_thread() {
	struct sensor_value temp;
	int ret;
    int32_t temperatureFloat;

    // Check device ready
	if (!device_is_ready(tempSensor)) {
		printk("Temperature sensor: device not ready.\n");
		return;
	}

	while (1) {
        // Get sample
		ret = sensor_sample_fetch(tempSensor);
		if (ret) {
			printk("sensor_sample_fetch failed ret %d\n", ret);
			return;
		}

        // get temperature reading
		ret = sensor_channel_get(tempSensor, SENSOR_CHAN_DIE_TEMP, &temp);
        // get convert reading to float
        temperatureFloat = sensor_value_to_milli(&temp) - 13000;

        // Put in message queues
        k_sched_lock();
        k_msgq_purge(&temp_msgq);
        k_msgq_put(&temp_msgq, &temperatureFloat, K_NO_WAIT);
        k_sched_unlock();

        // Temperature sensor sampling delay
		k_msleep(TEMP_SAMPLING_RATE);
	}
}