// SPDX-License-Identifier: MPL-2.0
// Copyright (C) 2025 Stratos Thivaios
//
// EDLAVP-ESP-FW - The ESP-IDF Version of the EDLAVP firmware
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef _DEVICE_ID_H
#define _DEVICE_ID_H

#define DEVICE_ID_STRING_LENGTH 24 // "EDLAVP-XXXXXXXXXXXX" + null terminator

/**
 * @brief Initializes the device ID thingy.
 *
 * Must be called once during startup before any get_device_id() calls.
 * Reads the MAC address and generates the device ID string.
 */
void device_id_init(void);

/**
 * @brief Gets a pointer to the device ID string.
 *
 * @return Pointer to null-terminated device ID string (e.g.,
 * "EDLAVP-AABBCCDDEEFF") Returns null if it hasnt been inited.
 */
const char *get_device_id(void);

#endif //_DEVICE_ID_H
