#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "wifi_manager.h"
#include <stdio.h>

static const char *TAG = "MAIN";

void app_main(void)
{
  ESP_LOGI(TAG, "Starting WiFi");

  // Initialize NVS (required)
  nvs_flash_init();

  // Connect to WiFi
  wifi_connect();
}
