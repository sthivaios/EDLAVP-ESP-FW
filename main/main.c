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
#include "ntp_manager.h"
#include "nvs_flash.h"
#include "system_state.h"
#include "wifi_manager.h"
#include <stdio.h>

// static const char *TAG = "MAIN";

void app_main(void) {
  // initialize NVS
  nvs_flash_init();

  // initialize event group bits
  system_state_init();

  // attempt to start wifi stuff
  wifi_connect();

  // start the ntp_manager task
  TaskHandle_t ntp_manager_handle;
  xTaskCreate(ntp_manager, "ntp_manager", NTP_MANAGER_TASK_STACK_SIZE, NULL, 1,
              &ntp_manager_handle);
}