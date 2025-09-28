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