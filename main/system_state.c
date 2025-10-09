// SPDX-License-Identifier: MPL-2.0
// Copyright (C) 2025 Stratos Thivaios
//
// EDLAVP-ESP-FW - The ESP-IDF Version of the EDLAVP firmware
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "system_state.h"
#include "esp_log.h"
#include "freertos/event_groups.h"

static const char *TAG = "SYSTEM_STATE";
static EventGroupHandle_t s_event_group = NULL;

static QueueHandle_t readout_queue;

/**
 * @brief Initializes the sensor readout queue.
 *
 * Creates a FreeRTOS queue that holds floating-point sensor readings.
 * Must be called before any send/receive operations.
 */
void readout_queue_init(void) {
  readout_queue = xQueueCreate(CONFIG_SOFTWARE_DS18B20_READOUT_QUEUE_SIZE,
                               sizeof(FullReadout));
  if (readout_queue == NULL) {
    ESP_LOGE(TAG, "Queue creation failed!");
  }
}

/**
 * @brief Sends a sensor reading to the readout queue.
 *
 * If the queue is full, the function blocks for up to @p ticks_to_wait
 * before returning. Use 0 for a non-blocking send.
 *
 * @param full_readout Struct of the full readout to enqueue.
 * @param ticks_to_wait Maximum number of ticks to wait if the queue is full.
 * @return pdPASS if the value was successfully enqueued, pdFAIL otherwise.
 */
BaseType_t readout_queue_send(const FullReadout full_readout,
                              const TickType_t ticks_to_wait) {
  if (readout_queue == NULL)
    return pdFAIL;
  return xQueueSend(readout_queue, &full_readout, ticks_to_wait);
}

/**
 * @brief Receives a sensor reading from the readout queue.
 *
 * If the queue is empty, the function blocks for up to @p ticks_to_wait
 * before returning. Use 0 for a non-blocking receive.
 *
 * @param full_readout Pointer to a variable to store the received struct of
 * the full readout.
 * @param ticks_to_wait Maximum number of ticks to wait if the queue is empty.
 * @return pdPASS if a value was successfully received, pdFAIL otherwise.
 */
BaseType_t readout_queue_receive(FullReadout *full_readout,
                                 const TickType_t ticks_to_wait) {
  if (readout_queue == NULL || full_readout == NULL)
    return pdFAIL;
  return xQueueReceive(readout_queue, full_readout, ticks_to_wait);
}

void system_state_init(void) {
  if (s_event_group == NULL) {
    s_event_group = xEventGroupCreate();
    ESP_LOGI(TAG, "Event group created");
  }
}

void system_set_bits(const EventBits_t bits) {
  if (s_event_group) {
    xEventGroupSetBits(s_event_group, bits);
  }
}

void system_clear_bits(const EventBits_t bits) {
  if (s_event_group) {
    xEventGroupClearBits(s_event_group, bits);
  }
}

EventBits_t system_wait_for_bits(const EventBits_t bits,
                                 const BaseType_t wait_for_all,
                                 const TickType_t ticks_to_wait) {
  if (!s_event_group)
    return 0;
  return xEventGroupWaitBits(s_event_group, bits, pdFALSE, wait_for_all,
                             ticks_to_wait);
}