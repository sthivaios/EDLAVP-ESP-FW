// SPDX-License-Identifier: MPL-2.0
// Copyright (C) 2025 Stratos Thivaios
//
// EDLAVP-ESP-FW - The ESP-IDF Version of the EDLAVP firmware
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef _MQTT_MANAGER_H
#define _MQTT_MANAGER_H

void mqtt_app_start();

#define MQTT_MANAGER_TASK_STACK_SIZE 8192

void mqtt_manager(void *pvParameters);

#endif //_MQTT_MANAGER_H