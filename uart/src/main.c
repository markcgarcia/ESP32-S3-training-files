/* This code attempts to use the registers on the ESP32
   in order to enable and use UART communication.
   Run the Wokwi diagram to see the serial monitor changes. */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/system_reg.h"
#include "soc/uart_reg.h"
#include "soc/soc.h"
#include "soc/io_mux_reg.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_sig_map.h"
#include "driver/gpio.h"

// Standard putchar function (learned on Arduino Uno)
void UART_putchar(char c) {
    // wait to empty if it's full

    // Read register FIRST  .. REG_READ
    // Shift to end  SECOND ...CNT_S represents the amount to shift it s.t. it's at LSB position
    // Mask out      THIRD  ...CNT_V represents the bitmask to extract
    while ((REG_READ(UART_STATUS_REG(0)) >> UART_TXFIFO_CNT_S & UART_TXFIFO_CNT_V) > 0);
    REG_WRITE(UART_FIFO_REG(0), c);
}

// Standard putstr function
void UART_putstr(char* s) {
    while (*s != 0) {
        UART_putchar(*s);
        s++;
    }
}
void app_main() {
    // Tried to use SET_BIT and SET_FIELD for registers that had
    // other important fields in them.

    // Enable the clock and release peripheral from reset state
    REG_SET_BIT(SYSTEM_PERIP_CLK_EN0_REG, SYSTEM_UART_CLK_EN);
    REG_SET_BIT(SYSTEM_PERIP_RST_EN0_REG, SYSTEM_UART_RST);
    REG_CLR_BIT(SYSTEM_PERIP_RST_EN0_REG, SYSTEM_UART_RST);

    // Set the UART clock source.
    REG_SET_FIELD(UART_CLK_CONF_REG(0), UART_SCLK_SEL, 1);
    REG_SET_BIT(UART_CLK_CONF_REG(0), UART_TX_SCLK_EN);
    REG_SET_BIT(UART_CLK_CONF_REG(0), UART_RX_SCLK_EN);

    // Set the baud rate
    REG_SET_FIELD(UART_CLKDIV_REG(0), UART_CLKDIV, 694);
    REG_SET_FIELD(UART_CLKDIV_REG(0), UART_CLKDIV_FRAG, 7);

    // Tx on GPIO1, Rx on GPIO3
    REG_WRITE(GPIO_OUT_W1TS_REG, (1 << GPIO_NUM_1));

    // Configure IO Mux
    REG_SET_FIELD(IO_MUX_GPIO1_REG, MCU_SEL, 1);
    REG_SET_FIELD(IO_MUX_GPIO3_REG, MCU_SEL, 1);
    // Configure IO MUX to enable input and output
    REG_SET_FIELD(IO_MUX_GPIO1_REG, FUN_DRV, 3);
    REG_SET_BIT(IO_MUX_GPIO3_REG, FUN_IE);

    // Route signals
    REG_WRITE(GPIO_FUNC1_OUT_SEL_CFG_REG, U0TXD_OUT_IDX);
    REG_WRITE(GPIO_FUNC0_IN_SEL_CFG_REG + U0RXD_IN_IDX*4, 3); // write GPIO pin num to in

    // Keep program going forever
    while (1) {
        UART_putstr("Hello, world!\r\n");
        vTaskDelay(200 / portTICK_PERIOD_MS);   // 200ms
    }
}
  