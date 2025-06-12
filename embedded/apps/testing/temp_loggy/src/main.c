#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/sensor.h>

static const struct device *const tempSensor = DEVICE_DT_GET(DT_NODELABEL(die_temp));

int main(void)
{
	struct sensor_value temp;
	int ret;

	if (!device_is_ready(tempSensor)) {
		printk("sensor: device not ready.\n");
		return 0;
	}

	while (1) {
		ret = sensor_sample_fetch(tempSensor);
		if (ret) {
			printk("sensor_sample_fetch failed ret %d\n", ret);
			return 0;
		}

		ret = sensor_channel_get(tempSensor, SENSOR_CHAN_DIE_TEMP, &temp);

		printk("Temperature: %f\n", sensor_value_to_double(&temp));

		k_sleep(K_MSEC(500));
	}
	return 0;
}