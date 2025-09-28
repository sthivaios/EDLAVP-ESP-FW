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

#ifndef _WIFI_MANAGER_H
#define _WIFI_MANAGER_H

/**
 * @brief Attempts to connect to Wi-Fi. Also sets up event handlers and ensures
 * the connection stays up.
 */
void wifi_connect(void);

#endif //_WIFI_MANAGER_H