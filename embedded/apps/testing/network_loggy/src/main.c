#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr/drivers/uart.h>

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/sensor.h>

#include <zephyr/drivers/gpio.h>

#include <math.h>

static uint8_t sensor_unit = 0;
static uint8_t control_unit = 0;

static const struct gpio_dt_spec ch1_h = GPIO_DT_SPEC_GET(DT_NODELABEL(ch1_high_led), gpios);
static const struct gpio_dt_spec ch1_l = GPIO_DT_SPEC_GET(DT_NODELABEL(ch1_low_led), gpios);
static const struct gpio_dt_spec ch2_h = GPIO_DT_SPEC_GET(DT_NODELABEL(ch2_high_led), gpios);
static const struct gpio_dt_spec ch2_l = GPIO_DT_SPEC_GET(DT_NODELABEL(ch2_low_led), gpios);
static const struct gpio_dt_spec ch3_h = GPIO_DT_SPEC_GET(DT_NODELABEL(ch3_high_led), gpios);
static const struct gpio_dt_spec ch3_l = GPIO_DT_SPEC_GET(DT_NODELABEL(ch3_low_led), gpios);
static const struct gpio_dt_spec ch4_h = GPIO_DT_SPEC_GET(DT_NODELABEL(ch4_high_led), gpios);
static const struct gpio_dt_spec ch4_l = GPIO_DT_SPEC_GET(DT_NODELABEL(ch4_low_led), gpios);

static const struct gpio_dt_spec* alarm_pointers[8] = {&ch1_h, &ch2_h, &ch3_h, &ch4_h, &ch1_l, &ch2_l, &ch3_l, &ch4_l};

void alarm_led_init() {
	for (uint8_t i = 0; i < 8; i++) {
		gpio_pin_configure_dt(alarm_pointers[i], GPIO_OUTPUT_INACTIVE);
	 }
}

void alarm_set(int index) {
	gpio_pin_set_dt(alarm_pointers[index], 1);
}

void alarm_clear(int index) {
	gpio_pin_set_dt(alarm_pointers[index], 0);
}

#define CH1				0x77
#define CH2				0xFD
#define CH3				0x99
#define CH4				0xAA
#define CH5				0xBB
#define CH6				0xFE
#define CH7				0xDD
#define CH8				0xEE
#define STOP_BYTE		0x41

static uint16_t alarm_bitmask = 0;
static uint8_t ranges = 0;

static const uint8_t channel_headers[8] = {CH1, CH2, CH3, CH4, CH5, CH6, CH7, CH8};

static uint8_t myTurn = 1;
static char uart_char = 0;

static int state = 0;

static uint8_t res_temp = 0;

static uint8_t float_index = 0;
static uint8_t float_part[4] = {0};

static float tempF = 0;
static char type = 100;

static uint32_t helpme = 0;

static float voltage[4] = {0};
static float test[4] = {69.693, 81.420, 57.072, 1.111};

static const struct device *const shell_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_shell_uart));

#ifdef CONFIG_BOARD_NUCLEO_L433RC_P
static const struct device *const uart_dev = DEVICE_DT_GET(DT_NODELABEL(lpuart1));
#endif

#ifdef CONFIG_BOARD_NUCLEO_L432KC
static const struct device *const uart_dev = DEVICE_DT_GET(DT_NODELABEL(usart1));
#endif

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

#define FLOAT_TO_UINT32(f, i) {	\
	float tempFloat = f;	\
	i = *(uint32_t*)&tempFloat;	\
}

#define UINT32_TO_FLOAT(i, f) {	\
	uint32_t tempInt = i;	\
	f = *(float*)&tempInt;	\
}

void separate_u32(uint32_t x, uint8_t* buf) {
	buf[0] = (uint8_t)((x >> 24) & 0x000000FF);
	buf[1] = (uint8_t)((x >> 16) & 0x000000FF);
	buf[2] = (uint8_t)((x >> 8) & 0x000000FF);
	buf[3] = (uint8_t)(x & 0x000000FF);
}

uint32_t join_u32(uint8_t* fArray) {
	return (((uint32_t)fArray[0] << 24) |
			((uint32_t)fArray[1] << 16) |
			((uint32_t)fArray[2] << 8) 	|
			((uint32_t)fArray[3]) );
}

//	Thermistor conversion

float steinhart_equation(float r) {
	const float A = 3.354016E-3;
	const float B = 2.569850E-4;
	const float C = 2.620131E-6;
    const float D = 6.383091E-8;
	const float R0 = 10000.0;

	float tempK = (1 / (A + (B * log(r/R0)) + (C * pow(log(r/R0), 2)) + (D * pow(log(r/R0), 3))));
    float tempC = tempK - 273.15;
    return -tempC;
}

/*
	FOR control_unit UNIT!!!!
 */
 uint8_t check_header_control(uint8_t header) {
	switch (header) {
		case CH1:
			return 0;
		case CH2:
			return 1;
		case CH3:
			return 2;
		case CH4:
			return 3;
		case CH5:
			return 4;
		case CH6:
			return 5;
		case CH7:
			return 6;
		case CH8:
			return 7;
		default:
			return 200;
	}
 }

// UART CALLBACK FUNCTION
void uart_callback(const struct device *dev, void *user_data) {
	//printk("ISR\r\n");
	ARG_UNUSED(user_data);

	/* Verify uart_irq_update() */
	if (!uart_irq_update(dev)) {
		return;
	}

	/* Verify uart_irq_rx_ready() */
	if (uart_irq_rx_ready(dev)) {
		// write received data into uart_char
		uart_fifo_read(dev, &uart_char, 1);
		printk("Received: %x\r\n", uart_char);

		if (uart_char == STOP_BYTE) {
			myTurn = 1;
			//printk("Stop byte received: myTurn is %d\r\n", myTurn);
			float_index = 0;
			if (sensor_unit) {
				for (int i = 0; i < 4; i++) {
					float_part[i] = 0;
				}
			}
			return;
		}
		if (myTurn == 1) {
			float_index = 0;
			return;
		}
		if (control_unit) {
			if ((type == 100) || (type == 200)) {
				type = check_header_control(uart_char);
				float_index = 0;
				return;
			} else {
				float_part[float_index] = uart_char;
				//printk("Float part %d: %x\r\n", float_index, uart_char);
				if (float_index >= 3) {
					float_index = 0;
					UINT32_TO_FLOAT(join_u32(float_part), tempF);
					if (res_temp) {
						printk("Channel %d: %.1fC\r\n", (type + 1), steinhart_equation(tempF * 100000));
					} else {
						printk("Channel %d: %.3fV\r\n", (type + 1), tempF);
					}
					type = 100;
					//k_msleep(10000000000000);
				}
				float_index++;
			}
		}
		if (sensor_unit) {
			if ((type == 100) || (type == 200)) {
				type = check_header_control(uart_char);
				return;
			} else {
				if (uart_char & 0b00000001) {
					// high triggered
					alarm_bitmask |= (0b0000000000000001 << (type + 8));
					printk("HIGH Triggered: %d\r\n", type);
					alarm_set(type);
				} else {
					// high not triggered
					alarm_bitmask &= ~(0b0000000000000001 << (type + 8));
					alarm_clear(type);
				}
				if (uart_char & 0b00000010) {
					// low triggered
					alarm_bitmask |= (0b0000000000000001 << type);
					printk("LOW Triggered: %d\r\n", type);
					alarm_set(type + 4);
				} else {
					//low not triggered
					alarm_bitmask &= ~(0b0000000000000001 << type);
					alarm_clear(type + 4);
				}
				if ((type <= 3)) {
					if (uart_char & 0b00000100) {
						// range setting 10V
						ranges |= (0b00000001 << type);
					} else {
						// range setting 1V
						ranges &= ~(0b00000001 << type);
					}
				}
				printk("Alarm bits: %x\r\n", alarm_bitmask);
				printk("Range bits: %x\r\n", ranges);
				type = 100;
			}
		}
	}
}

void print_uart(char* buf) {
	int msg_len = strlen(buf);
	for (int i = 0; i < msg_len; i++) {
		uart_poll_out(uart_dev, buf[i]);
	}
}

int main() {
	#ifdef CONFIG_BOARD_NUCLEO_L432KC
	control_unit = 1;
	#endif

	#ifdef CONFIG_BOARD_NUCLEO_L433RC_P
	control_unit = 1;
	#endif

	// THERMISTOR YES/NO
	res_temp = 0;

	uint32_t tempV = 0;

	uint8_t separated[4] = {0};
	int64_t prevTick = k_uptime_get();
	/* Verify uart_irq_callback_set() */
	//uart_irq_rx_enable(uart_dev);

	uart_irq_callback_set(uart_dev, uart_callback);

	if (control_unit) {
		myTurn = 0;
		/* Enable Tx/Rx interrupt before using fifo */
		/* Verify uart_irq_rx_enable() */
		uart_irq_rx_enable(uart_dev);
	}

	int err;
	uint32_t count = 0;
	uint16_t buf;
	struct adc_sequence sequence = {
		.buffer = &buf,
		/* buffer size in bytes, not number of samples */
		.buffer_size = sizeof(buf),
	};

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

	alarm_led_init();

	while (1) {
		switch(state) {
			case 0:
				if (sensor_unit) {
					for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
						int32_t val_mv;
			
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
						//printk("%"PRId32, val_mv);
						err = adc_raw_to_millivolts_dt(&adc_channels[i],
											&val_mv);
						/* conversion to mV may not be supported, skip if not */
						if (err < 0) {
							printk(" (value in mV not available)\n");
						} else {
							voltage[i] = (val_mv / 1000.0f);
						}
					}
				}
				state++;
				break;
			case 1:
				if (myTurn == 1) {
					// disable uart rx
					uart_irq_rx_disable(uart_dev);
					k_msleep(3);
					if (control_unit) {

/* 						uart_poll_out(uart_dev, CH2);
						uart_poll_out(uart_dev, 0b00000111); */

						// transmit stop byte through uart
						uart_poll_out(uart_dev, STOP_BYTE);
						printk("Stop byte sent from control unit\r\n");
					} else {
						for (int i = 0; i < 4; i++) {
							// debug send data
							//printk("Sending data...(%d)\r\n", i);
	
							// send header char
							//printk("Header: %x\r\n", (channel_headers[i]));
							uart_poll_out(uart_dev, channel_headers[i]);
	
							// separate and send buffer
							FLOAT_TO_UINT32(voltage[i], helpme);
							separate_u32(helpme, separated);

							//printk("Part 1: %x\r\n", separated[0]);
							//printk("Part 2: %x\r\n", separated[1]);
							//printk("Part 3: %x\r\n", separated[2]);
							//printk("Part 4: %x\r\n", separated[3]);
							print_uart(separated);
						}
						uart_poll_out(uart_dev, STOP_BYTE);
						printk("Stop byte sent from sensor unit\r\n");
					}
					k_msleep(3);
					uart_irq_rx_enable(uart_dev);
					myTurn = 0;
					prevTick = k_uptime_get();
				}
				state++;
				break;
			case 2:
				if (control_unit) {
					// check if waiting for data AND has been longer than 1 second
					if (myTurn == 0 && (k_uptime_get() - prevTick > 1000)) {
						//printk("FORCED TIMEOUT! Sending stop byte.\n");
						myTurn = 1; // reclaim turn after timeout
						prevTick = k_uptime_get(); // reset timeout reference
					}
				}
				/*
								if (sensor_unit) {
					for (int i = 0; i < 4; i++) {
						test[i] = test[i] + 0.001f;
					}
				}	
				*/
		
				state = 0;
				break;
			case 3: 
				break;
		}
		k_msleep(3);
	}
	return 0;
}