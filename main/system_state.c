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

void system_state_init(void) {
  if (s_event_group == NULL) {
    s_event_group = xEventGroupCreate();
    ESP_LOGI(TAG, "Event group created");
  }
}

void system_set_bits(EventBits_t bits) {
  if (s_event_group) {
    xEventGroupSetBits(s_event_group, bits);
  }
}

void system_clear_bits(EventBits_t bits) {
  if (s_event_group) {
    xEventGroupClearBits(s_event_group, bits);
  }
}

EventBits_t system_wait_for_bits(EventBits_t bits, BaseType_t wait_for_all,
                                 TickType_t ticks_to_wait) {
  if (!s_event_group)
    return 0;
  return xEventGroupWaitBits(s_event_group, bits, pdFALSE, wait_for_all,
                             ticks_to_wait);
}