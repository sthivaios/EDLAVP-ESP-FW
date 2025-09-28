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

#ifndef _SYSTEM_STATE_H
#define _SYSTEM_STATE_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <stdint.h>

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

#endif //_SYSTEM_STATE_H
