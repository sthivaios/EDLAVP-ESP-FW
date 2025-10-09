#include "mqtt_manager.h"

#include "cJSON.h"
#include "esp_netif.h"
#include <inttypes.h>
#include <string.h>

#include "esp_log.h"
#include "mqtt_client.h"
#include "system_state.h"

#include <time.h>

static const char *TAG = "mqtt_manager";

static esp_mqtt_client_handle_t mqtt_client = NULL;

static void log_error_if_nonzero(const char *message, const int error_code) {
  if (error_code != 0) {
    ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
  }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               const int32_t event_id, void *event_data) {
  ESP_LOGD(TAG,
           "Event dispatched from event loop base=%s, event_id=%" PRIi32 "",
           base, event_id);
  esp_mqtt_event_handle_t event = event_data;
  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "Successfully connected to the MQTT broker.");
    system_set_bits(SYS_BIT_MQTT_CONNECTED);
    break;

  case MQTT_EVENT_DISCONNECTED:
    system_clear_bits(SYS_BIT_MQTT_CONNECTED);
    ESP_LOGW(TAG, "Disconnected from MQTT broker... Will not publish anything "
                  "until reconnection.");
    break;

  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI(TAG, "Subscribed to an MQTT topic: msg_id=%d", event->msg_id);
    break;

  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(TAG, "Unsubscribed from an MQTT topic: msg_id=%d", event->msg_id);
    break;

  case MQTT_EVENT_PUBLISHED:
    // ESP_LOGI(TAG, "Published a message to an MQTT topic: msg_id=%d",
    //          event->msg_id);
    break;

  case MQTT_EVENT_DATA:
    ESP_LOGI(TAG, "Received the following data:");
    ESP_LOGI(TAG, "TOPIC=%.*s\r\n", event->topic_len, event->topic);
    ESP_LOGI(TAG, "DATA=%.*s\r\n", event->data_len, event->data);
    break;

  case MQTT_EVENT_ERROR:
    ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
    if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
      log_error_if_nonzero("reported from esp-tls",
                           event->error_handle->esp_tls_last_esp_err);
      log_error_if_nonzero("reported from tls stack",
                           event->error_handle->esp_tls_stack_err);
      log_error_if_nonzero("captured as transport's socket errno",
                           event->error_handle->esp_transport_sock_errno);
      ESP_LOGE(TAG, "Last errno string (%s)",
               strerror(event->error_handle->esp_transport_sock_errno));
    }
    break;

  default:
    ESP_LOGI(TAG, "Unknown MQTT event id: %d", event->event_id);
    break;
  }
}

void mqtt_app_start(void) {
  const esp_mqtt_client_config_t mqtt_cfg = {
      .broker.address.uri = CONFIG_MQTT_BROKER_URL,
      .credentials.username = CONFIG_MQTT_USERNAME,
      .credentials.authentication.password = CONFIG_MQTT_PASSWORD};

  mqtt_client = esp_mqtt_client_init(&mqtt_cfg);

  ESP_ERROR_CHECK(esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID,
                                                 mqtt_event_handler, NULL));

  ESP_LOGI(TAG, "Waiting to connect to wifi first.");
  system_wait_for_bits(SYS_BIT_GOT_IP | SYS_BIT_NTP_SYNCED, pdTRUE,
                       portMAX_DELAY);
  ESP_LOGI(TAG, "Will now attempt to connect to the MQTT broker.");
  ESP_ERROR_CHECK(esp_mqtt_client_start(mqtt_client));
}

static void mqtt_publish_readout(const FullReadout full_readout) {

  system_wait_for_bits(SYS_BIT_MQTT_CONNECTED, pdTRUE, portMAX_DELAY);

  cJSON *json_array = cJSON_CreateArray();
  for (size_t i = 0; i < full_readout.readout_array_size; i++) {
    cJSON *obj = cJSON_CreateObject();
    if (!obj)
      continue;

    cJSON_AddNumberToObject(obj, "timestamp",
                            (double)full_readout.readouts[i].timestamp);
    cJSON_AddNumberToObject(obj, "value", full_readout.readouts[i].value);

    char address_str[19]; // "0x" + 16 hex chars + null
    snprintf(address_str, sizeof(address_str), "0x%016llX",
             full_readout.readouts[i].address);

    cJSON_AddStringToObject(obj, "address", address_str);

    cJSON_AddItemToArray(json_array, obj); // array takes ownership
  }

  char *json_string = cJSON_PrintUnformatted(json_array);
  cJSON_Delete(json_array);

  int msg_id;
  int retry_counter = 0;

  do {
    msg_id =
        esp_mqtt_client_publish(mqtt_client, CONFIG_MQTT_SENSOR_READOUT_TOPIC,
                                json_string, (int)strlen(json_string), 1, 0);
    retry_counter++;
  } while (msg_id == -1 && retry_counter < 3);

  if (msg_id == -1) {
    ESP_LOGE(TAG, "Failed to publish MQTT message after %d attempts",
             retry_counter);
  } else {
    ESP_LOGI(TAG, "Successfully published message (msg_id=%d)", msg_id);
  }

  free(json_string);
}

void mqtt_manager(void *pvParameters) {
  ESP_LOGI(TAG, "%s task started", TAG);

  mqtt_app_start();

  // ReSharper disable once CppDFAEndlessLoop
  while (1) {
    system_wait_for_bits(SYS_BIT_MQTT_CONNECTED, pdTRUE, portMAX_DELAY);
    FullReadout full_readout;

    while (readout_queue_receive(&full_readout, 0) == pdPASS) {
      if (system_wait_for_bits(SYS_BIT_MQTT_CONNECTED, pdTRUE, 0) == 0) {
        ESP_LOGW(TAG, "Lost MQTT connection while processing queue");
        break;
      }

      mqtt_publish_readout(full_readout);

      // small delay between publishes to avoid overwhelming the broker
      vTaskDelay(pdMS_TO_TICKS(50));
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}