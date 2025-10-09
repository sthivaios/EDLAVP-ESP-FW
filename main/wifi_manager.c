// SPDX-License-Identifier: MPL-2.0
// Copyright (C) 2025 Stratos Thivaios
//
// EDLAVP-ESP-FW - The ESP-IDF Version of the EDLAVP firmware
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "wifi_manager.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "system_state.h"

// grab the config values (see Kconfig.projbuild)
#define WIFI_SSID CONFIG_WIFI_SSID
#define WIFI_PASSWORD CONFIG_WIFI_PASSWORD
#define WIFI_MAX_RETRY CONFIG_WIFI_MAXIMUM_RETRY

static const char *TAG = "WIFI";
static int retry_count = 0;

// Returns a "human-readable" error string for different Wi-Fi error
// codes/reasons.
static const char *get_disconnect_reason_string(const uint8_t reason) {
  switch (reason) {
  case WIFI_REASON_UNSPECIFIED:
    return "Unspecified";
  case WIFI_REASON_AUTH_EXPIRE:
    return "Authentication expired";
  case WIFI_REASON_AUTH_LEAVE:
    return "Authentication left";
  case WIFI_REASON_ASSOC_EXPIRE:
    return "Association expired";
  case WIFI_REASON_ASSOC_TOOMANY:
    return "Too many associations";
  case WIFI_REASON_NOT_AUTHED:
    return "Not authenticated";
  case WIFI_REASON_NOT_ASSOCED:
    return "Not associated";
  case WIFI_REASON_ASSOC_LEAVE:
    return "Association left";
  case WIFI_REASON_ASSOC_NOT_AUTHED:
    return "Association not authenticated";
  case WIFI_REASON_DISASSOC_PWRCAP_BAD:
    return "Power capability bad";
  case WIFI_REASON_DISASSOC_SUPCHAN_BAD:
    return "Supported channel bad";
  case WIFI_REASON_IE_INVALID:
    return "Invalid IE";
  case WIFI_REASON_MIC_FAILURE:
    return "MIC failure";
  case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
    return "4-way handshake timeout (possibly incorrect password)";
  case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT:
    return "Group key update timeout";
  case WIFI_REASON_IE_IN_4WAY_DIFFERS:
    return "IE in 4-way differs";
  case WIFI_REASON_GROUP_CIPHER_INVALID:
    return "Invalid group cipher";
  case WIFI_REASON_PAIRWISE_CIPHER_INVALID:
    return "Invalid pairwise cipher";
  case WIFI_REASON_AKMP_INVALID:
    return "Invalid AKMP";
  case WIFI_REASON_UNSUPP_RSN_IE_VERSION:
    return "Unsupported RSN IE version";
  case WIFI_REASON_INVALID_RSN_IE_CAP:
    return "Invalid RSN IE capability";
  case WIFI_REASON_802_1X_AUTH_FAILED:
    return "802.1X authentication failed";
  case WIFI_REASON_CIPHER_SUITE_REJECTED:
    return "Cipher suite rejected";
  case WIFI_REASON_BEACON_TIMEOUT:
    return "Beacon timeout";
  case WIFI_REASON_NO_AP_FOUND:
    return "No AP found (ssid down or perhaps misspelled?)";
  case WIFI_REASON_AUTH_FAIL:
    return "Authentication failed (incorrect password)";
  case WIFI_REASON_ASSOC_FAIL:
    return "Association failed";
  case WIFI_REASON_HANDSHAKE_TIMEOUT:
    return "Handshake timeout (possibly incorrect password)";
  case WIFI_REASON_CONNECTION_FAIL:
    return "Connection failed (205 - no specific error, perhaps the password "
           "is incorrect?)";
  default:
    ESP_LOGW(TAG, "Uncaught exception code: %d", reason);
    return "Unknown reason";
  }
}

/* Handles Wi-Fi-related events, such as a connection attempt starting, a
 * successful connection or a disconnection. It then starts specific actions,
 * logs stuff or sets event group bits to trigger other actions in other RTOS
 * tasks */
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               const int32_t event_id, void *event_data) {
  if (event_id == WIFI_EVENT_STA_START) {
    ESP_LOGI(TAG, "WiFi started, trying to connect...");
    // call function to start connecting to Wi-Fi
    esp_wifi_connect();
  } else if (event_id == WIFI_EVENT_STA_CONNECTED) {
    // set Wi-Fi connected bit
    system_set_bits(SYS_BIT_WIFI_CONNECTED);
    // reset the retry counter
    retry_count = 0;
    ESP_LOGI(TAG, "Connected to WiFi");
  } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
    // clear Wi-Fi connected and got-ip bit
    system_clear_bits(SYS_BIT_WIFI_CONNECTED);
    system_clear_bits(SYS_BIT_GOT_IP);

    // grab the disconnection reason
    wifi_event_sta_disconnected_t *disconnected = event_data;

    ESP_LOGE(TAG, "WiFi disconnected! Reason: %s",
             get_disconnect_reason_string(disconnected->reason));

    // add to the retry counter
    retry_count++;

    // decide whether to continue or stop based on the number of retries
    if (retry_count <= WIFI_MAX_RETRY) {
      ESP_LOGW(TAG, "Retry %d/%d - Attempting reconnection...", retry_count,
               WIFI_MAX_RETRY);
      esp_wifi_connect();
    } else {
      ESP_LOGE(TAG, "Max retries reached. Something is really wrong with the "
                    "WiFi. Giving up.");
      ESP_LOGW(TAG, "You should hard-reset the MCU.");
    }
  }
}

// Handles IP-related events. For now only the "got ip" event.
static void ip_event_handler(void *arg, esp_event_base_t event_base,
                             const int32_t event_id, void *event_data) {
  if (event_id == IP_EVENT_STA_GOT_IP) {
    // set the got-ip bit
    system_set_bits(SYS_BIT_GOT_IP);

    // grab the ip
    ip_event_got_ip_t *event = event_data;

    // log it
    ESP_LOGI(TAG, "Got IP address: " IPSTR, IP2STR(&event->ip_info.ip));
  }
}

// Main function to connect to Wi-Fi
void wifi_connect(void) {
  esp_log_level_set("wifi", ESP_LOG_WARN); // make the Wi-Fi component shut up
  ESP_LOGI(TAG, "Setting up WiFi...");

  // initialize networking
  ESP_ERROR_CHECK(esp_netif_init());
  esp_event_loop_create_default();
  esp_netif_create_default_wifi_sta();

  // initialize Wi-Fi
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  // register event handlers
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                             &wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                             &ip_event_handler, NULL));

  // define the Wi-Fi config
  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = WIFI_SSID,
              .password = WIFI_PASSWORD,
          },
  };

  // configure Wi-Fi and start
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
}