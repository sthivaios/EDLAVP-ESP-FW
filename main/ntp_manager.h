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

#ifndef _NTP_MANAGER_H
#define _NTP_MANAGER_H

#define NTP_MANAGER_TASK_STACK_SIZE 2048

void ntp_manager(void *pvParameters);

#endif //_NTP_MANAGER_H
