/** 
**************************************************************
* @file embedded/lib/analog_circuitry.c
* @author Shane Baptist - 47484164
* @date 17/05/2025
* @brief Channel 1-4 voltage reading + range switching driver
*************************************************************** 
*/

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include "analog_circuitry.h"
#include "toslink.h"

// Define threads
K_THREAD_DEFINE(adc_tid, ADC_STACK_SIZE, adc_thread, NULL, NULL, NULL, ADC_THREAD_PRIORITY, 0, 0);

// Define message queue
K_MSGQ_DEFINE(chan1_msgq, sizeof(int32_t), 1, 1);
K_MSGQ_DEFINE(chan2_msgq, sizeof(int32_t), 1, 1);
K_MSGQ_DEFINE(chan3_msgq, sizeof(int32_t), 1, 1);
K_MSGQ_DEFINE(chan4_msgq, sizeof(int32_t), 1, 1);

// array of msgq pointers for indexing
struct k_msgq* chanQPointers[4] = {&chan1_msgq, &chan2_msgq, &chan3_msgq, &chan4_msgq};

// get mux gpio devices
static const struct gpio_dt_spec mux0 = GPIO_DT_SPEC_GET(DT_NODELABEL(mux_s0), gpios);
static const struct gpio_dt_spec mux1 = GPIO_DT_SPEC_GET(DT_NODELABEL(mux_s1), gpios);
static const struct gpio_dt_spec mux2 = GPIO_DT_SPEC_GET(DT_NODELABEL(mux_s2), gpios);

// get adc device
static const struct adc_dt_spec ads1119 = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));

 // function to initialise mux select gpios
 void init_mux() {
    gpio_pin_configure_dt(&mux0, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&mux1, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&mux2, GPIO_OUTPUT_INACTIVE);
 }

// function mux output select
void set_mux(uint8_t channel, uint8_t range) {
	if (channel > 3 || range > 1) {
		printk("invalid channel/range\r\n");
		return;
	}
	uint8_t muxIndex = (channel * 2) + range;
	gpio_pin_set_dt(&mux0, muxIndex & 0x01);
	gpio_pin_set_dt(&mux1, (muxIndex >> 1) & 0x01);
	gpio_pin_set_dt(&mux2, (muxIndex >> 2) & 0x01);
}

// Convert 0 - 3.3 to -1 - 1
int32_t convert_1(int32_t val) {
    return ((int32_t)val * 2000 / 3300) - 1000;
}

// Convert 0 - 3.3 to -10 - 10
int32_t convert_10(int32_t val) {
    return ((int32_t)val * 20000 / 3300) - 10000;
}

/*
    remap inputs to:
    screw 1 1V range -> index 2 range 0
    screw 1 10V range -> index 2 range 1

    screw 2 1V range -> index 1 range 0
    screw 2 10V range -> index 1 range 1

    screw 3 1V range -> index 3 range 0
    screw 3 10V range -> index 3 range 1

    screw 4 1V range -> index 0 range 0
    screw 4 10V range -> index 0 range 1
 */
uint8_t hardware_wenti(uint8_t channel) {
    switch (channel) {
        case 0:
            return 2;
        case 1:
            return 1;
        case 2:
            return 3;
        case 3:
            return 0;
        default:
            return 2;
    }
}

// adc thread
void adc_thread() {
    int err;
    int16_t buf;
    int32_t val_mv;
    uint8_t rangeBitmask = 0x0F;

    struct adc_sequence sequence = {
        .buffer = &buf,
        /* buffer size in bytes, not number of samples */
        .buffer_size = sizeof(buf),
    };

    // configure mux gpios
    init_mux();

    /* Configure channel prior to sampling. */
    if (!adc_is_ready_dt(&ads1119)) {
        printk("ADC device not ready\n");
        return;
    }
    err = adc_channel_setup_dt(&ads1119);
    if (err < 0) {
        printk("Could not setup channel");
        return;
    }

    (void)adc_sequence_init_dt(&ads1119, &sequence);
        while (1) {
        //uint32_t helpTime = k_uptime_get_32();
        k_msgq_peek(&range_msgq, &rangeBitmask);
        //TODO:     remove hardcoded range
        rangeBitmask = 0x00;
        for (uint8_t i = 0; i < 4; i++) {
            set_mux(hardware_wenti(i), (1 -((rangeBitmask >> i) & 0x01)));

            // settling time
            k_msleep(MUX_SETTLING_TIME);

            // get adc reading
            err = adc_read_dt(&ads1119, &sequence);

            if (err < 0) {
                printk("Could not read (%d)\n", err);
                continue;
            }

            // Get adc count
            val_mv = (int32_t)buf;

            // Convert adc reading to float in V
            val_mv = (3300 * val_mv) / 32768;
            // Convert to appropriate range
            if (1 -((rangeBitmask >> i) & 0x01)) {
                // 10V range
                val_mv = convert_10(val_mv);
            } else {
                val_mv = convert_1(val_mv);
            }
            
            // Put in message queues
            k_sched_lock();
            k_msgq_purge(chanQPointers[i]);
            k_msgq_put(chanQPointers[i], &val_mv, K_NO_WAIT);
            k_sched_unlock();
        }
    }
}

// Equation for temp from resistance of prtd
/* float callendar_vandusen_equation(float r) {
    const float A = 3.9083E-3;
    const float B = -5.775E-7;
    const float R0 = 1000.0f;

    float T = (-A + sqrt(pow(A, 2) - (4 * B * (1 - (r/R0))))) / (2 * B);
    return T;
}

// Equation for temp from resistance of thermistor
float steinhart_equation(float r) {
    const float A = 3.354016E-3;
    const float B = 2.569850E-4;
    const float C = 2.620131E-6;
    const float D = 6.383091E-8;
    const float R0 = 10000.0f;

    float tempK = (1 / (A + (B * log(r/R0)) + (C * pow(log(r/R0), 2)) + (D * pow(log(r/R0), 3))));
    float tempC = tempK - 273.15;
    return tempC;
}

float steinhart_equation(float v, uint8_t range) {
    float r = 0;

    if (range == 1) {
        r = v / 0.00001f;
    } else {
        r = v / 0.0002f;
    }

    const float A = 3.354016E-3;
    const float B = 2.569850E-4;
    const float C = 2.620131E-6;
    const float D = 6.383091E-8;
    const float R0 = 10000.0f;

    float tempK = (1 / (A + (B * log(r/R0)) + (C * pow(log(r/R0), 2)) + (D * pow(log(r/R0), 3))));
    float tempC = tempK - 273.15;
    return tempC;
} */