#include "wifi_manager.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"

#define WIFI_SSID CONFIG_WIFI_PASSWORD
#define WIFI_PASSWORD CONFIG_WIFI_MAXIMUM_RETRY

static const char *TAG = "WIFI";
static int retry_count = 0;

// function to map the possible error codes to log strings
static const char *get_disconnect_reason_string(uint8_t reason) {
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

// handler for wifi events
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
  if (event_id == WIFI_EVENT_STA_START) {
    ESP_LOGI(TAG, "WiFi started, trying to connect...");
    esp_wifi_connect();
  } else if (event_id == WIFI_EVENT_STA_CONNECTED) {
    ESP_LOGI(TAG, "Connected to WiFi");
  } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
    wifi_event_sta_disconnected_t *disconnected =
        (wifi_event_sta_disconnected_t *)event_data;

    ESP_LOGE(TAG, "WiFi disconnected! Reason: %s",
             get_disconnect_reason_string(disconnected->reason));

    retry_count++;
    if (retry_count <= CONFIG_WIFI_MAXIMUM_RETRY) {
      ESP_LOGW(TAG, "Retry %d/%d - Attempting reconnection...", retry_count,
               CONFIG_WIFI_MAXIMUM_RETRY);
      esp_wifi_connect();
    } else {
      ESP_LOGE(TAG, "Max retries reached. Something is really wrong with the "
                    "WiFi. Giving up.");
      ESP_LOGW(TAG, "You should hard-reset the MCU.");
    }
  }
}

// handler for ip events
static void ip_event_handler(void *arg, esp_event_base_t event_base,
                             int32_t event_id, void *event_data) {
  if (event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "Got IP address: " IPSTR, IP2STR(&event->ip_info.ip));
  }
}

// main function to connect
void wifi_connect(void) {
  esp_log_level_set("wifi", ESP_LOG_WARN); // Only show warnings and errors
  ESP_LOGI(TAG, "Setting up WiFi...");

  // initialize networking
  esp_netif_init();
  esp_event_loop_create_default();
  esp_netif_create_default_wifi_sta();

  // initialize wifi
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);

  // register event handlers
  esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler,
                             NULL);
  esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler,
                             NULL);

  // Set up WiFi config
  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = CONFIG_WIFI_SSID,
              .password = CONFIG_WIFI_PASSWORD,
          },
  };

  // Configure and start WiFi
  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
  esp_wifi_start();
}