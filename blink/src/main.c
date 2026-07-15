#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/gpio_reg.h"
#include "soc/soc.h" 
#include "driver/gptimer.h"
#include "driver/gpio.h"

static gpio_num_t pin = 4;
gptimer_handle_t gptimer = NULL;

// Our timer configuration
gptimer_config_t timer_config = {
  .clk_src    = GPTIMER_CLK_SRC_DEFAULT,  // set our source clock (80 MHz)
  .direction  = GPTIMER_COUNT_UP,         // set our clock direction (obv. want to count up)
  .resolution_hz = 1000000,               // set our precision (minimum time unit) -> 1MHz, 1M ticks
};

gptimer_alarm_config_t alarm_config = {
  .reload_count = 0,                      // on an alarm event, set what to reload value to
  .alarm_count = 500000,                 // set "value to count up to" in ticks
  .flags.auto_reload_on_alarm = true,     // make our timer reload after alarm event is triggered
};

static bool toggle_led(gptimer_handle_t timer,                  // points to the timer that triggered alarm
                       const gptimer_alarm_event_data_t *edata, // struct with details on alarm event
                       void *user_ctx)                          // "user context" for our callback function
{
  // read if bit is on or off
  uint32_t reg = REG_READ(GPIO_OUT_REG);
  int on = reg & (1 << (gpio_num_t)user_ctx);

  // toggle bit
  if (on) {
    REG_WRITE(GPIO_OUT_W1TC_REG, (1 << (gpio_num_t)user_ctx));
  } else {
    REG_WRITE(GPIO_OUT_W1TS_REG, (1 << (gpio_num_t)user_ctx));
  }
  return false;
}

gptimer_event_callbacks_t cbs = {
  .on_alarm = toggle_led,
};

void app_main() {

  // Enable pin 2 as output
  REG_WRITE(GPIO_ENABLE_REG, (1 << pin));

  // Create timer instance
  ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

  // Set timer's alarm action
  ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));

  // Register timer callback function
  // In this case, it's a PIN toggle
  ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, (void*)pin));

  // Enable timer
  ESP_ERROR_CHECK(gptimer_enable(gptimer));

  // Start timer
  ESP_ERROR_CHECK(gptimer_start(gptimer));

  // Keep program going forever
  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
  