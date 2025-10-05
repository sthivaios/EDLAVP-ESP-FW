#include "mqtt_manager.h"

#include "cJSON.h"
#include "esp_netif.h"
#include <inttypes.h>
#include <stdint.h>
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
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;
  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "Successfully connected to the MQTT broker.");
    system_set_bits(SYS_BIT_MQTT_CONNECTED);

    msg_id = esp_mqtt_client_subscribe(client, "/test", 0);

    break;

  case MQTT_EVENT_DISCONNECTED:
    system_clear_bits(SYS_BIT_MQTT_CONNECTED);
    break;

  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI(TAG, "Subscribed to an MQTT topic: msg_id=%d", event->msg_id);
    break;

  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(TAG, "Unsubscribed from an MQTT topic: msg_id=%d", event->msg_id);
    break;

  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI(TAG, "Published a message to an MQTT topic: msg_id=%d",
             event->msg_id);
    break;

  case MQTT_EVENT_DATA:
    ESP_LOGI(TAG, "Received the following data:");
    printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    printf("DATA=%.*s\r\n", event->data_len, event->data);
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

static void mqtt_publish_readout(float readout) {

  system_wait_for_bits(SYS_BIT_NTP_SYNCED, pdTRUE, portMAX_DELAY);

  // declare time variables
  static char strftime_buf[64];
  time_t now;
  struct tm timeinfo;

  // get the time
  time(&now);

  // time stuff that idk what its doing but its doing it
  localtime_r(&now, &timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);

  // create the json object
  cJSON *mqtt_message_object = cJSON_CreateObject();
  cJSON_AddStringToObject(mqtt_message_object, "utc_timestamp", strftime_buf);
  cJSON_AddNumberToObject(mqtt_message_object, "value", readout);

  char *json_string = cJSON_PrintUnformatted(mqtt_message_object);
  cJSON_Delete(mqtt_message_object);

  const int msg_id = esp_mqtt_client_publish(
      mqtt_client, CONFIG_MQTT_SENSOR_READOUT_TOPIC, json_string, 0, 0, 0);
  ESP_LOGI(TAG, "sent publish successfully, msg_id=%d", msg_id);

  free(json_string);
}

void mqtt_manager(void *pvParameters) {
  ESP_LOGI(TAG, "%s task started", TAG);

  mqtt_app_start();

  while (1) {
    float readout;

    readout_queue_receive(&readout, portMAX_DELAY);
    mqtt_publish_readout(readout);
  }
}