// SPDX-License-Identifier: MPL-2.0
// Copyright (C) 2025 Stratos Thivaios
//
// EDLAVP-ESP-FW - The ESP-IDF Version of the EDLAVP firmware
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef _SENSOR_MANAGER_H
#define _SENSOR_MANAGER_H
#include "ds18b20.h"
#include "time.h"

typedef struct {
  ds18b20_device_handle_t handle;
  uint64_t address;
} DS18B20Sensor;

#define ONEWIRE_MAX_RX_BYTES                                                   \
  10 // 1byte ROM command + 8byte ROM number + 1byte device command

void sensor_manager_ds18b20(void *pvParameters);

#endif //_SENSOR_MANAGER_H
