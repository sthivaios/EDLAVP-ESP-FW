// SPDX-License-Identifier: MPL-2.0
// Copyright (C) 2025 Stratos Thivaios
//
// EDLAVP-ESP-FW - The ESP-IDF Version of the EDLAVP firmware
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt_manager.h"
#include "ntp_manager.h"
#include "nvs_flash.h"
#include "sensor_manager.h"
#include "system_state.h"
#include "timer_manager.h"
#include "wifi_manager.h"

#include <time.h>

static const char *TAG = "MAIN";

void app_main(void) {
  // initialize the sensor readout queue
  readout_queue_init();

  // initialize NVS
  ESP_ERROR_CHECK(nvs_flash_init());

  // set the timezone to UTC
  setenv("TZ", "UTC", 1);
  tzset();

  // initialize event group bits
  system_state_init();

  // attempt to start wifi stuff
  wifi_connect();

  // start the ntp_manager task
  TaskHandle_t ntp_manager_handle;
  if (xTaskCreate(ntp_manager, "ntp_manager", NTP_MANAGER_TASK_STACK_SIZE, NULL,
                  3, &ntp_manager_handle) != pdPASS) {
    ESP_LOGE(TAG, "FATAL: Failed to create the ntp_manager task!");
    abort();
  };

  // start the sensor_manager task
  TaskHandle_t sensor_manager_handle;
  if (xTaskCreate(sensor_manager, "sensor_manager",
                  SENSOR_MANAGER_TASK_STACK_SIZE, NULL, 2,
                  &sensor_manager_handle) != pdPASS) {
    ESP_LOGE(TAG, "FATAL: Failed to create the sensor_manager task!");
    abort();
  }

  // start the mqtt_manager task
  TaskHandle_t mqtt_manager_handle;
  if (xTaskCreate(mqtt_manager, "mqtt_manager", MQTT_MANAGER_TASK_STACK_SIZE,
                  NULL, 1, &mqtt_manager_handle) != pdPASS) {
    ESP_LOGE(TAG, "FATAL: Failed to create the mqtt_manager task!");
    abort();
  };

  // setup and start the readout timer
  setup_readout_timer();

  // ReSharper disable once CppDFAEndlessLoop
  while (1) {
    const size_t total_heap = heap_caps_get_total_size(MALLOC_CAP_DEFAULT);
    const size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    const size_t used_heap = total_heap - free_heap;

    const float percentage_used = (float)used_heap / (float)total_heap * 100.0f;

    ESP_LOGW(TAG, "HEAP USAGE: (%d/%d) -> %.2f%c", used_heap, total_heap,
             percentage_used, '%');
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}