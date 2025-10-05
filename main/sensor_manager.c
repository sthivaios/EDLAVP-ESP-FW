#include "sensor_manager.h"

#include "ds18b20.h"
#include "esp_err.h"
#include "esp_log.h"
#include "onewire_bus_impl_rmt.h"
#include "onewire_device.h"
#include "system_state.h"
#include "time.h"

static const char *TAG = "sensor_manager";

static void send_full_readout_to_queue(FullReadout full_readout) {
  if (readout_queue_send(full_readout, pdMS_TO_TICKS(100)) != pdPASS) {
    ESP_LOGW(TAG, "Queue full, dropping readout!");
  }
}

void sensor_manager(void *pvParameters) {
  system_wait_for_bits(SYS_BIT_MQTT_CONNECTED, pdTRUE, portMAX_DELAY);

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

  int ds18b20_device_num = 0;
  ds18b20_device_handle_t ds18b20s[CONFIG_HARDWARE_DS18B20_MAX_SENSORS];
  onewire_device_iter_handle_t iter = NULL;
  onewire_device_t next_onewire_device;
  esp_err_t search_result = ESP_OK;

  // create 1-wire device iterator, which is used for device search
  ESP_ERROR_CHECK(onewire_new_device_iter(bus, &iter));
  ESP_LOGI(TAG, "Device iterator created, start searching...");
  do {
    search_result = onewire_device_iter_get_next(iter, &next_onewire_device);
    if (search_result == ESP_OK) { // found a new device, let's check if we
      // can upgrade it to a DS18B20
      ds18b20_config_t ds_cfg = {};
      onewire_device_address_t address;
      // check if the device is a DS18B20, if so, return the ds18b20 handle
      if (ds18b20_new_device_from_enumeration(&next_onewire_device, &ds_cfg,
                                              &ds18b20s[ds18b20_device_num]) ==
          ESP_OK) {
        ds18b20_get_device_address(ds18b20s[ds18b20_device_num], &address);
        ESP_LOGI(TAG, "Found a DS18B20[%d], address: %016llX",
                 ds18b20_device_num, address);
        ds18b20_device_num++;
      } else {
        ESP_LOGI(TAG, "Found an unknown device, address: %016llX",
                 next_onewire_device.address);
      }
    }
  } while (search_result != ESP_ERR_NOT_FOUND);

  if (ds18b20_device_num == 0) {
    ESP_LOGE(TAG, "No sensors found! Suspending sensor_manager task.");
    vTaskSuspend(NULL);
  }

  ESP_ERROR_CHECK(onewire_del_device_iter(iter));
  ESP_LOGI(TAG, "Searching done, %d DS18B20 device(s) found",
           ds18b20_device_num);

  while (1) {
    ReadoutArray all_readouts;
    int readout_count = 0;
    float temperature;
    ESP_ERROR_CHECK(ds18b20_trigger_temperature_conversion_for_all(bus));
    for (int i = 0; i < ds18b20_device_num; i++) {
      system_wait_for_bits(SYS_BIT_NTP_SYNCED, pdTRUE, portMAX_DELAY);
      ESP_ERROR_CHECK(ds18b20_get_temperature(ds18b20s[i], &temperature));
      // get the time
      time_t now;
      time(&now);
      ESP_LOGI(TAG, "Temperature read from DS18B20[%d]: %f", i, temperature);

      const SingleReadout readout = {
          .timestamp = now,
          .value = temperature,
      };

      all_readouts[readout_count++] = readout;
    }

    FullReadout full_readout = {0};
    full_readout.readout_array_size = readout_count;
    // TODO: Fix this
    memcpy(full_readout.readouts, all_readouts,
           readout_count * sizeof(SingleReadout));

    send_full_readout_to_queue(full_readout);

    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}