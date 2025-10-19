// SPDX-License-Identifier: MPL-2.0
// Copyright (C) 2025 Stratos Thivaios
//
// EDLAVP-ESP-FW - The ESP-IDF Version of the EDLAVP firmware
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "device_id.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_mac.h"

#include <stdbool.h>
#include <stdint.h>

static const char *TAG = "device_id";

static char device_id_string[DEVICE_ID_STRING_LENGTH];
static uint8_t device_mac[6];
static bool is_initialized = false;

void device_id_init(void) {
  // check if the id has already been initialized
  if (is_initialized) {
    ESP_LOGW(TAG, "Device ID already initialized, skipping");
    return;
  }

  // grab the default mac and store it in the device_mac var
  const esp_err_t ret = esp_efuse_mac_get_default(device_mac);

  // if the above has errored, log an error and abort as its a fatal error
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "FATAL: Failed to read MAC address: %s",
             esp_err_to_name(ret));
    // explode lol
    abort();
  }

  // format the id (prepend "EDLAVP")
  snprintf(device_id_string, DEVICE_ID_STRING_LENGTH,
           "EDLAVP-%02X%02X%02X%02X%02X%02X", device_mac[0], device_mac[1],
           device_mac[2], device_mac[3], device_mac[4], device_mac[5]);

  // set is_initialized to true
  is_initialized = true;
  // log that the id has been initialized
  ESP_LOGI(TAG, "Device ID initialized: %s", device_id_string);
}

const char *get_device_id(void) {
  // if not initialized, error
  if (!is_initialized) {
    ESP_LOGE(TAG, "Device ID not initialized! Call device_id_init() first");
    return NULL;
  }

  // return the id string
  return device_id_string;
}