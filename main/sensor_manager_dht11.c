// SPDX-License-Identifier: MPL-2.0
// Copyright (C) 2025 Stratos Thivaios
//
// EDLAVP-ESP-FW - The ESP-IDF Version of the EDLAVP firmware
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "sensor_manager_dht11.h"

#include "esp_err.h"
#include "esp_log.h"
#include "system_state.h"
#include "time.h"
#include "types.h"

static const char *TAG = "sensor_manager_dht11";

void sensor_manager_dht11(void *pvParameters) {
  ESP_LOGI(TAG, "%s task started", TAG);

  // get sensor

  while (1) {
    system_wait_for_bits(SYS_BIT_SENSOR_READ_REQUESTED, pdTRUE, portMAX_DELAY);
    system_wait_for_bits(SYS_BIT_NTP_SYNCED, pdTRUE, portMAX_DELAY);

    // get readout

    const UniversalSingleReadout readout = {.value = temperature,
                                            .timestamp = now,
                                            .sensor_type = "dht11",
                                            .unit = "%"};

    if (readout_queue_send(readout, pdMS_TO_TICKS(100)) != pdPASS) {
      ESP_LOGW(TAG, "Queue full, dropping readout!");
    } else {
      ESP_LOGI(TAG, "READOUT QUEUED -> DS18B20: %.2f", temperature);
    }

    system_clear_bits(SYS_BIT_SENSOR_READ_REQUESTED);
  }
}