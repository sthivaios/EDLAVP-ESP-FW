// SPDX-License-Identifier: MPL-2.0
// Copyright (C) 2025 Stratos Thivaios
//
// EDLAVP-ESP-FW - The ESP-IDF Version of the EDLAVP firmware
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "sensor_manager_ds18b20.h"

#include "ds18b20.h"
#include "esp_err.h"
#include "esp_log.h"
#include "onewire_bus_impl_rmt.h"
#include "onewire_device.h"
#include "system_state.h"
#include "time.h"
#include "types.h"

static const char *TAG = "sensor_manager_ds18b20";

static DS18B20Sensor sensor;

void sensor_manager_ds18b20(void *pvParameters) {
  ESP_LOGI(TAG, "%s task started", TAG);

  // install 1-wire bus
  onewire_bus_handle_t bus = NULL;
  onewire_bus_config_t bus_config = {
      .bus_gpio_num = CONFIG_HARDWARE_DS18B20_GPIO_PIN,
      .flags = {
          .en_pull_up = true, // enable the internal pull-up resistor in case
                              // the external device didn't have one
      }};
  onewire_bus_rmt_config_t rmt_config = {.max_rx_bytes = ONEWIRE_MAX_RX_BYTES};
  ESP_ERROR_CHECK(onewire_new_bus_rmt(&bus_config, &rmt_config, &bus));

  onewire_device_iter_handle_t iter = NULL;
  onewire_device_t next_onewire_device;
  esp_err_t search_result = ESP_OK;

  // create 1-wire device iterator, which is used for device search
  ESP_ERROR_CHECK(onewire_new_device_iter(bus, &iter));
  ESP_LOGI(TAG, "Device iterator created, searching for a DS18B20...");
  do {
    search_result = onewire_device_iter_get_next(iter, &next_onewire_device);
    if (search_result == ESP_OK) {
      // found a new device, let's check if we
      // can upgrade it to a DS18B20
      ds18b20_config_t ds_cfg = {};
      onewire_device_address_t address;
      // check if the device is a DS18B20, if so, return the ds18b20 handle
      if (ds18b20_new_device_from_enumeration(&next_onewire_device, &ds_cfg,
                                              &sensor.handle) == ESP_OK) {
        ds18b20_get_device_address(sensor.handle, &address);
        sensor.address = address;
        ESP_LOGI(TAG, "Found a DS18B20 at address: %016llX", address);
      } else {
        ESP_LOGW(TAG, "Found an unknown OneWire device, address: %016llX",
                 next_onewire_device.address);
      }
    }
  } while (search_result != ESP_ERR_NOT_FOUND);

  if (sensor.handle == NULL) {
    ESP_LOGW(TAG, "No DS18B20 found! Suspending sensor_manager_ds18b20 task.");
    vTaskSuspend(NULL);
  }

  ESP_ERROR_CHECK(onewire_del_device_iter(iter));
  ESP_LOGI(TAG, "Searching done, DS18B20 sensor found");

  while (1) {
    system_wait_for_bits(SYS_BIT_DS18B20_READ_REQUESTED, pdTRUE, portMAX_DELAY);

    system_wait_for_bits(SYS_BIT_NTP_SYNCED, pdTRUE, portMAX_DELAY);
    float temperature;

    time_t now;
    time(&now);

    ESP_ERROR_CHECK(ds18b20_trigger_temperature_conversion_for_all(bus));
    ESP_ERROR_CHECK(ds18b20_get_temperature(sensor.handle, &temperature));

    const UniversalSingleReadout readout = {.value = temperature,
                                            .timestamp = now,
                                            .sensor_type = "ds18b20",
                                            .unit = "C"};

    if (readout_queue_send(readout, pdMS_TO_TICKS(100)) != pdPASS) {
      ESP_LOGW(TAG, "Queue full, dropping readout!");
    } else {
      ESP_LOGI(TAG, "READOUT QUEUED -> DS18B20: %.2f", temperature);
    }

    system_clear_bits(SYS_BIT_DS18B20_READ_REQUESTED);
  }
}