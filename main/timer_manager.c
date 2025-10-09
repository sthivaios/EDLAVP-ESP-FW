#include "timer_manager.h"
#include "esp_timer.h"
#include "system_state.h"

static void sensor_timer_callback(void *arg) {
  system_set_bits(SYS_BIT_SENSOR_READ_REQUESTED);
}

void setup_readout_timer(void) {
  // timer config
  const esp_timer_create_args_t timer_args = {
      .callback = &sensor_timer_callback, // the callback
      .arg = NULL,                        // arguments passed to the callback
      .dispatch_method = ESP_TIMER_TASK,  // run in the esp_timer task
      .name = "sensor_timer"              // timer name for debugging
  };

  // create the timer
  esp_timer_handle_t timer;
  ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer));

  // start timer
  ESP_ERROR_CHECK(
      esp_timer_start_periodic(timer, 10000000)); // 10 seconds in microseconds
}