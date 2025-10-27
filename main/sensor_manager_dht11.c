// SPDX-License-Identifier: MPL-2.0
// Copyright (C) 2025 Stratos Thivaios
//
// EDLAVP-ESP-FW - The ESP-IDF Version of the EDLAVP firmware
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "sensor_manager_dht11.h"

#include "dht.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "system_state.h"
#include "time.h"
#include "types.h"

static const char *TAG = "sensor_manager_dht11";

void sensor_manager_dht11(void *pvParameters) {
  ESP_LOGI(TAG, "%s task started", TAG);

  while (1) {
    system_wait_for_bits(SYS_BIT_DHT11_READ_REQUESTED, pdTRUE, portMAX_DELAY);
    system_wait_for_bits(SYS_BIT_NTP_SYNCED, pdTRUE, portMAX_DELAY);

    // get readout
    float temperature, humidity;
    time_t now;

#ifdef CONFIG_HARDWARE_DHT11_INTERNAL_PULLUP
    gpio_set_pull_mode(CONFIG_HARDWARE_DHT11_GPIO_PIN, GPIO_PULLUP_ONLY);
#endif

    if (dht_read_float_data(DHT_TYPE_DHT11, CONFIG_HARDWARE_DHT11_GPIO_PIN,
                            &humidity, &temperature) != ESP_OK) {
      ESP_LOGW(TAG, "Could not read data from the DHT11 sensor!");
      continue;
    }
    time(&now);

    const UniversalSingleReadout readout = {.value = humidity,
                                            .timestamp = now,
                                            .sensor_type = "dht11",
                                            .unit = "%"};

    if (readout_queue_send(readout, pdMS_TO_TICKS(100)) != pdPASS) {
      ESP_LOGW(TAG, "Queue full, dropping readout!");
    } else {
      ESP_LOGI(TAG, "READOUT QUEUED -> DHT11: %.2f (humidity)", humidity);
    }

    system_clear_bits(SYS_BIT_DHT11_READ_REQUESTED);
  }
}