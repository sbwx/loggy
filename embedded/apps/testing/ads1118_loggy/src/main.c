#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>

#define ADS1118_CONFIG 0x8583

#define SPI_OP  SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB

/*
	SS	MUX2	MUX1	MUX0	PGA2	PGA1	PGA0	MODE	DR2	DR1	DR0	TSMODE	PULL_UP	NOP1	NOP0	RESERVED
*/

/*
	BYTE0
*/
#define CHANNEL_0			0x40
#define CHANNEL_1			0x50
#define CHANNEL_2			0x30
#define CHANNEL_3			0x70

/*
	BYTE1
*/
#define DEFAULT	0x82

uint8_t MUX_SELECT[4] = {CHANNEL_0, CHANNEL_1, CHANNEL_2, CHANNEL_3};

const struct spi_dt_spec ads1118_dev = SPI_DT_SPEC_GET(DT_NODELABEL(ads1118), SPI_OP, 0);

static const struct gpio_dt_spec reset = GPIO_DT_SPEC_GET(DT_NODELABEL(adc_reset), gpios);

uint8_t my_buffer[4] = {0};
uint8_t tx_data[4] = { 0x9F, 0x00, 0x00, 0x00 };
uint8_t rx_data[4] = { 0 };

struct spi_buf tx_buf = {
    .buf = tx_data,
    .len = sizeof(tx_data),
};

struct spi_buf rx_buf = {
    .buf = rx_data,
    .len = sizeof(rx_data),
};

struct spi_buf_set tx = {
    .buffers = &tx_buf,
    .count = 1,
};

struct spi_buf_set rx = {
    .buffers = &rx_buf,
    .count = 1,
};

int main(void)
{
	gpio_pin_configure_dt(&reset, GPIO_OUTPUT_INACTIVE);

	int ret;
	int16_t adcReadings[4] = {0};

	tx_data[1] = DEFAULT;
	tx_data[3] = DEFAULT;

    if (!spi_is_ready_dt(&ads1118_dev)) {
        printk("SPI device not ready\n");
        return 0;
    }

    while (1) {
		for (int i = 0; i < 1; i++) {
			tx_data[0] = MUX_SELECT[i];
			tx_data[2] = MUX_SELECT[i];
			ret = spi_write_dt(&ads1118_dev, &tx);
			if (ret) {
				printk("SPI transmit failed\r\n");
			}
			ret = spi_read_dt(&ads1118_dev, &rx);
			if (ret) {
				printk("SPI receive failed\r\n");
			}
			for (int j = 0; j < 4; j++) {
				printk("RX[%d]:	%x\r\n", j, rx_data[j]);
			}
			adcReadings[i] = sys_get_be16(rx_data);
			printk("ADC Channel %d:	%d\r\n", i, adcReadings[i]);

/* 			//ret = spi_transceive_dt(&ads1118_dev, &tx, &rx);
			if (ret == 0) {
				printk("SPI transceive successful\n");
				for (int j = 0; j < 4; j++) {
					printk("RX[%d]:	%x\r\n", j, rx_data[j]);
				}
				adcReadings[i] = sys_get_be16(rx_data);
				printk("ADC Channel %d:	%d\r\n", ((i == 0) ? 1 : (i - 1)), adcReadings[i]);
			} else {
				printk("SPI transceive failed: %d\n", ret);
			} */
			k_msleep(1000);
		}
    }
    return 0;
}