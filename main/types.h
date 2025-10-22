// SPDX-License-Identifier: MPL-2.0
// Copyright (C) 2025 Stratos Thivaios
//
// EDLAVP-ESP-FW - The ESP-IDF Version of the EDLAVP firmware
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef _TYPES_H
#define _TYPES_H
#include "time.h"

typedef struct {
  float value;
  time_t timestamp;
  const char *sensor_type;
  const char *unit;
} UniversalSingleReadout;

#endif //_TYPES_H
