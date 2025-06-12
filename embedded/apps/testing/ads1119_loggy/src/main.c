/*
 * Copyright (c) 2020 Libre Solar Technologies GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

 #include <inttypes.h>
 #include <stddef.h>
 #include <stdint.h>
 
 #include <zephyr/device.h>
 #include <zephyr/devicetree.h>
 #include <zephyr/drivers/adc.h>
 #include <zephyr/kernel.h>
 #include <zephyr/sys/printk.h>
 #include <zephyr/sys/util.h>
 #include <zephyr/drivers/sensor.h>

 #include <zephyr/drivers/gpio.h>

 #define ACTIVE_CHANNEL	3
 #define ACTIVE_RANGE	1

 static const struct gpio_dt_spec mux0 = GPIO_DT_SPEC_GET(DT_NODELABEL(mux_s0), gpios);
 static const struct gpio_dt_spec mux1 = GPIO_DT_SPEC_GET(DT_NODELABEL(mux_s1), gpios);
 static const struct gpio_dt_spec mux2 = GPIO_DT_SPEC_GET(DT_NODELABEL(mux_s2), gpios);

 uint8_t channelBitmasks[8] = {0b00000000, 0b00000001};
 
 #if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
	 !DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
 #error "No suitable devicetree overlay specified"
 #endif
 
 #define DT_SPEC_AND_COMMA(node_id, prop, idx) \
	 ADC_DT_SPEC_GET_BY_IDX(node_id, idx),
 
 /* Data of ADC io-channels specified in devicetree. */
 static const struct adc_dt_spec adc_channels[] = {
	 DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
				  DT_SPEC_AND_COMMA)
 };

 void set_mux(uint8_t channel, uint8_t range) {
	if (channel > 3 || range > 1) {
		printk("invalid channel/range\r\n");
		return;
	}
	uint8_t muxIndex = (channel * 2) + range;
	printk("MUX INDEX: %d\r\n", muxIndex);
	gpio_pin_set_dt(&mux0, muxIndex & 0x01);
	gpio_pin_set_dt(&mux1, (muxIndex >> 1) & 0x01);
	gpio_pin_set_dt(&mux2, (muxIndex >> 2) & 0x01);
 }
 
 int main(void) {
	 int err;
	 uint32_t count = 0;
	 uint16_t buf;
	 float voltage_float;
	 struct adc_sequence sequence = {
		 .buffer = &buf,
		 /* buffer size in bytes, not number of samples */
		 .buffer_size = sizeof(buf),
	 };

	gpio_pin_configure_dt(&mux0, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&mux1, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&mux2, GPIO_OUTPUT_INACTIVE);

	// TEST MUX SETTING
	set_mux(ACTIVE_CHANNEL, ACTIVE_RANGE);
 
	 /* Configure channels individually prior to sampling. */
	 for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
		 if (!adc_is_ready_dt(&adc_channels[i])) {
			 printk("ADC controller device %s not ready\n", adc_channels[i].dev->name);
			 return 0;
		 }
		 err = adc_channel_setup_dt(&adc_channels[i]);
		 if (err < 0) {
			 printk("Could not setup channel #%d (%d)\n", i, err);
			 return 0;
		 }
	 }
 
	 while (1) {
		 printk("ADC reading[%u]:\n", count++);
		 for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
			 int32_t val_mv;
 
			 printk("- %s, channel %d: ",
				 adc_channels[i].dev->name,
				 adc_channels[i].channel_id);
 
			 (void)adc_sequence_init_dt(&adc_channels[i], &sequence);
 
			 err = adc_read_dt(&adc_channels[i], &sequence);
			 if (err < 0) {
				 printk("Could not read (%d)\n", err);
				 continue;
			 }
 
			 /*
				 * If using differential mode, the 16 bit value
				 * in the ADC sample buffer should be a signed 2's
				 * complement value.
				 */
			 if (adc_channels[i].channel_cfg.differential) {
				 val_mv = (int32_t)((int16_t)buf);
			 } else {
				 val_mv = (int32_t)buf;
			 }
			 printk("%"PRId32, val_mv);

			 voltage_float = (float)(3.3f * ((float)val_mv / 32768.f));
			 /* conversion to mV may not be supported, skip if not */
			 if (err < 0) {
				 printk(" (value in mV not available)\n");
			 } else {
				 printk(" = %.3f V\n", voltage_float);
			 }
		 }
		 k_sleep(K_MSEC(500));
	 }
	 return 0;
 }
