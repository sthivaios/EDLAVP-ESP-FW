// Copyright (C) 2025 Stratos Thivaios
//
// This file is part of "EDLAVP-ESP-FW".
//
// "EDLAVP-ESP-FW" is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option) any
// later version.
//
// "EDLAVP-ESP-FW" is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// "EDLAVP-ESP-FW". If not, see <https://www.gnu.org/licenses/>.

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