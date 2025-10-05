// SPDX-License-Identifier: MPL-2.0
// Copyright (C) 2025 Stratos Thivaios
//
// EDLAVP-ESP-FW - The ESP-IDF Version of the EDLAVP firmware
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef _SYSTEM_STATE_H
#define _SYSTEM_STATE_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define SYS_BIT_WIFI_CONNECTED (1 << 0)
#define SYS_BIT_GOT_IP (1 << 1)
#define SYS_BIT_NTP_SYNCED (1 << 2)
#define SYS_BIT_MQTT_CONNECTED (1 << 3)

// should be called early in app_main()
void system_state_init(void);

// event group wrappers

void system_set_bits(EventBits_t bits);
void system_clear_bits(EventBits_t bits);
EventBits_t system_wait_for_bits(EventBits_t bits, BaseType_t wait_for_all,
                                 TickType_t ticks_to_wait);

// sensor readout queue wrappers

void readout_queue_init(void);
BaseType_t readout_queue_send(float value, TickType_t ticks_to_wait);
BaseType_t readout_queue_receive(float *value, TickType_t ticks_to_wait);

#endif //_SYSTEM_STATE_H
