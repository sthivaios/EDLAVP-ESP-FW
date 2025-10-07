// SPDX-License-Identifier: MPL-2.0
// Copyright (C) 2025 Stratos Thivaios
//
// EDLAVP-ESP-FW - The ESP-IDF Version of the EDLAVP firmware
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "ntp_manager.h"

#include "esp_log.h"
#include "esp_netif_sntp.h"
#include "system_state.h"

static const char *TAG = "ntp_manager";
static const char *NTP_SERVER = "pool.ntp.org";
static const uint32_t SYNC_INTERVAL = CONFIG_TIMESYNC_INTERVAL;

void ntp_manager(void *pvParameters) {
  ESP_LOGI(TAG, "%s task started", TAG);

  ESP_LOGI(TAG, "SYNC_INTERVAL runtime value: %u", (unsigned)SYNC_INTERVAL);

  static char strftime_buf[64];
  time_t now;
  struct tm timeinfo;

  while (1) {
    ESP_LOGI(TAG, "Waiting to make sure ESP is connected to the internet");
    system_wait_for_bits(SYS_BIT_GOT_IP, pdTRUE, portMAX_DELAY);
    ESP_LOGI(TAG, "Will now attempt to sync time");

    ESP_LOGV(TAG, "Cleared SYS_BIT_NTP_SYNCED before resyncing time");
    system_clear_bits(SYS_BIT_NTP_SYNCED);

    ESP_LOGI(TAG, "Attempting to resync time with \"%s\"", NTP_SERVER);
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(NTP_SERVER);
    ESP_ERROR_CHECK(esp_netif_sntp_init(&config));
    const esp_err_t ret = esp_netif_sntp_sync_wait(
        pdMS_TO_TICKS(CONFIG_TIMESYNC_TIMEOUT * 1000UL));
    esp_netif_sntp_deinit();

    if (ret == ESP_OK) {
      ESP_LOGI(TAG, "Time synced successfully.");
      system_set_bits(SYS_BIT_NTP_SYNCED);
      ESP_LOGV(TAG, "Set SYS_BIT_NTP_SYNCED.");
    } else {
      ESP_LOGE(TAG, "Failed to sync time in 10 seconds (err=%d)", ret);
      ESP_LOGW(TAG, "Will try to sync again in 10 seconds.");
      vTaskDelay(pdMS_TO_TICKS(CONFIG_TIMESYNC_ERROR_RETRY_COOLDOWN * 1000UL));
      continue;
    }

    time(&now);

    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time UTC is: %s", strftime_buf);

    system_set_bits(SYS_BIT_NTP_SYNCED);
    ESP_LOGV(TAG, "Set SYS_BIT_NTP_SYNCED again.");
    ESP_LOGI(TAG, "Next NTP sync in: %d seconds", SYNC_INTERVAL);

    vTaskDelay(pdMS_TO_TICKS(SYNC_INTERVAL * 1000UL));
  }
}