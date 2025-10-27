// SPDX-License-Identifier: MPL-2.0
// Copyright (C) 2025 Stratos Thivaios
//
// EDLAVP-ESP-FW - The ESP-IDF Version of the EDLAVP firmware
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "timer_manager.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "system_state.h"

static const char *TAG = "timer_manager";

static void ds18b20_timer_callback(void *arg) {
  system_set_bits(SYS_BIT_DS18B20_READ_REQUESTED);
}

static void dht11_timer_callback(void *arg) {
  system_set_bits(SYS_BIT_DHT11_READ_REQUESTED);
}

static void setup_ds18b20_timer(void) {
  // DS18B20 Timer

  const esp_timer_create_args_t timer_args = {
      .callback = &ds18b20_timer_callback, // the callback
      .arg = NULL,                         // arguments passed to the callback
      .dispatch_method = ESP_TIMER_TASK,   // run in the esp_timer task
      .name = "sensor_timer_ds18b20"       // timer name for debugging
  };

  // create the timer
  esp_timer_handle_t timer;
  ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer));
  ESP_LOGI(TAG, "Created the readout timer successfully");

  // start timer
  ESP_ERROR_CHECK(
      esp_timer_start_periodic(timer, CONFIG_SOFTWARE_DS18B20_READOUT_INTERVAL *
                                          1000000ULL)); // microseconds
  ESP_LOGI(TAG, "Started the readout timer successfully");
}

static void setup_dht11_timer(void) {
  // DHT11 Timer

  const esp_timer_create_args_t timer_args = {
      .callback = &dht11_timer_callback, // the callback
      .arg = NULL,                       // arguments passed to the callback
      .dispatch_method = ESP_TIMER_TASK, // run in the esp_timer task
      .name = "sensor_timer_dht11"       // timer name for debugging
  };

  // create the timer
  esp_timer_handle_t timer;
  ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer));
  ESP_LOGI(TAG, "Created the readout timer successfully");

  // start timer
  ESP_ERROR_CHECK(
      esp_timer_start_periodic(timer, CONFIG_SOFTWARE_DHT11_READOUT_INTERVAL *
                                          1000000ULL)); // microseconds
  ESP_LOGI(TAG, "Started the readout timer successfully");
}

void setup_readout_timers(void) {
  setup_ds18b20_timer();
  setup_dht11_timer();
}