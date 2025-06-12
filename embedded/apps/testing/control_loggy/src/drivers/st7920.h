/*
 * st7920.h
 */

 #include <zephyr/kernel.h>
 #include <zephyr/device.h>
 #include <zephyr/drivers/gpio.h>
 #include <zephyr/devicetree.h>
 #include <stdio.h>
 #include <string.h>

 #define LCD_RESET_NODE  	DT_NODELABEL(lcd_reset)
 #define LCD_RS_NODE			DT_NODELABEL(lcd_rs)
 #define LCD_E_NODE			DT_NODELABEL(lcd_e)
 #define LCD_D0_NODE  		DT_NODELABEL(lcd_d0)
 #define LCD_D1_NODE  		DT_NODELABEL(lcd_d1)
 #define LCD_D2_NODE  		DT_NODELABEL(lcd_d2)
 #define LCD_D3_NODE  		DT_NODELABEL(lcd_d3)
 #define LCD_D4_NODE  		DT_NODELABEL(lcd_d4)
 #define LCD_D5_NODE  		DT_NODELABEL(lcd_d5)
 #define LCD_D6_NODE  		DT_NODELABEL(lcd_d6)
 #define LCD_D7_NODE  		DT_NODELABEL(lcd_d7)

 static const struct gpio_dt_spec reset = GPIO_DT_SPEC_GET(LCD_RESET_NODE, gpios);
 static const struct gpio_dt_spec rs = GPIO_DT_SPEC_GET(LCD_RS_NODE, gpios);
 static const struct gpio_dt_spec e = GPIO_DT_SPEC_GET(LCD_E_NODE, gpios);
 static const struct gpio_dt_spec d0 = GPIO_DT_SPEC_GET(LCD_D0_NODE, gpios);
 static const struct gpio_dt_spec d1 = GPIO_DT_SPEC_GET(LCD_D1_NODE, gpios);
 static const struct gpio_dt_spec d2 = GPIO_DT_SPEC_GET(LCD_D2_NODE, gpios);
 static const struct gpio_dt_spec d3 = GPIO_DT_SPEC_GET(LCD_D3_NODE, gpios);
 static const struct gpio_dt_spec d4 = GPIO_DT_SPEC_GET(LCD_D4_NODE, gpios);
 static const struct gpio_dt_spec d5 = GPIO_DT_SPEC_GET(LCD_D5_NODE, gpios);
 static const struct gpio_dt_spec d6 = GPIO_DT_SPEC_GET(LCD_D6_NODE, gpios);
 static const struct gpio_dt_spec d7 = GPIO_DT_SPEC_GET(LCD_D7_NODE, gpios);

 static const struct gpio_dt_spec* data_pointers[8] = {&d0, &d1, &d2, &d3, &d4, &d5, &d6, &d7};

 static uint8_t rows[4] = {0x80, 0x90, 0x88, 0x98};

 void gpio_init() {
   gpio_pin_configure_dt(&reset, GPIO_OUTPUT_INACTIVE);
   gpio_pin_configure_dt(&rs, GPIO_OUTPUT_INACTIVE);
   gpio_pin_configure_dt(&e, GPIO_OUTPUT_INACTIVE);

   for (uint8_t i = 0; i < 8; i++) {
      gpio_pin_configure_dt(data_pointers[i], GPIO_OUTPUT_INACTIVE);
   }
}

 void unset_data() {
	for (int i = 0; i < 8; i++) {
      gpio_pin_set_dt(data_pointers[i], 0);
	}
}

 void set_data(uint8_t bits) {
   k_usleep(1);
   gpio_pin_set_dt(&e, 1);

   for (int i = 0; i < 8; i++) {
      gpio_pin_set_dt(data_pointers[i], (bits >> i) & 0x01);
   }
   k_usleep(1);

   gpio_pin_set_dt(&e, 0);
   k_usleep(1);

   unset_data();
}

 void lcd_init() {
   // reset high
   k_msleep(50);
   gpio_pin_set_dt(&reset, 1);

   // FUNCTION SET
   // set data
   gpio_pin_set_dt(&rs, 0);
   set_data(0x30);

   k_usleep(110);

   // FUNCTION SET
   // set data
   gpio_pin_set_dt(&rs, 0);
   set_data(0x30);

   k_usleep(40);

   // DISPLAY SETTINGS
   // set data
   gpio_pin_set_dt(&rs, 0);
   set_data(0x0C);

   k_usleep(110);

   // DISPLAY CLEAR
   // set data
   gpio_pin_set_dt(&rs, 0);
   set_data(0x01);

   k_msleep(20);

   // ENTRY MODE
   // set data
   gpio_pin_set_dt(&rs, 0);
   set_data(0x06);
}

 /*

 80, 81, 82, 83, 84, 85, 86, 87     First row

 90, 91, 92, 93, 94, 95, 96 ,97

 88, 89, 8A, 8B, 8C, 8D, 8E, 8F

 98, 99, 9A, 9B, 9C, 9D, 9E, 9F     Fourth row
 
 */
 void set_ddram_address(uint8_t row, uint8_t col) {
   gpio_pin_set_dt(&rs, 0);

   // row + column = position
   set_data(rows[row] | col);
   k_usleep(1);
}

 void write_char(uint8_t c) {
   // set data
   gpio_pin_set_dt(&rs, 1);
   set_data(c);
   k_usleep(1);

   // reset rs
   gpio_pin_set_dt(&rs, 0);
}

 void write_str(char* str) {
   while (*str) {
      write_char(*str);
      str++;
   }
}