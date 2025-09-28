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

#include "ntp_manager.h"

#include "esp_log.h"
#include "esp_netif_sntp.h"
#include "system_state.h"

static const char *TAG = "ntp_manager";
static const char *NTP_SERVER = "pool.ntp.org";
static const uint32_t SYNC_INTERVAL = 86400;

void ntp_manager(void *pvParameters) {
  ESP_LOGI(TAG, "%s task started", TAG);

  while (1) {
    ESP_LOGI(TAG, "Waiting to make sure ESP is connected to the internet");
    system_wait_for_bits(SYS_BIT_GOT_IP, pdTRUE, portMAX_DELAY);
    ESP_LOGI(TAG, "Will now attempt to sync time, since ESP has been connected "
                  "to the internet");

    ESP_LOGI(TAG, "Cleared SYS_BIT_NTP_SYNCED before resyncing time");
    system_clear_bits(SYS_BIT_NTP_SYNCED);

    ESP_LOGI(TAG, "Attempting to resync time with \"%s\"", NTP_SERVER);
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(NTP_SERVER);
    ESP_ERROR_CHECK(esp_netif_sntp_init(&config));
    if (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(10000)) != ESP_OK) {
      ESP_LOGE(TAG, "Failed to sync time in 10 seconds");
    }
    esp_netif_sntp_deinit();
    ESP_LOGI(TAG, "Time synced!");

    time_t now;
    char strftime_buf[64];
    struct tm timeinfo;

    time(&now);
    // Set timezone to China Standard Time
    setenv("TZ", "UTC", 1);
    tzset();

    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time UTC is: %s", strftime_buf);

    system_set_bits(SYS_BIT_NTP_SYNCED);
    ESP_LOGI(TAG, "Set SYS_BIT_NTP_SYNCED again.");
    ESP_LOGI(TAG, "Next NTP sync in: %d seconds", SYNC_INTERVAL);

    vTaskDelay(pdMS_TO_TICKS(SYNC_INTERVAL * 1000));
  }
}