// SPDX-License-Identifier: MPL-2.0
// Copyright (C) 2025 Stratos Thivaios
//
// EDLAVP-ESP-FW - The ESP-IDF Version of the EDLAVP firmware
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef _WIFI_MANAGER_H
#define _WIFI_MANAGER_H

/**
 * @brief Attempts to connect to Wi-Fi. Also sets up event handlers and ensures
 * the connection stays up.
 */
void wifi_connect(void);

#endif //_WIFI_MANAGER_H