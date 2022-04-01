/*
 * Copyright 2012-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#ifndef _MICROEJ_TIME_H
#define _MICROEJ_TIME_H

#include <stdint.h>

/*
 * Returns -1 on error, 0 otherwise.
 */

void microej_time_init(void);

int64_t microej_time_getcurrenttime(uint8_t isPlatformTime);

int64_t microej_time_gettimenanos(void);

void microej_time_setapplicationtime(int64_t t);

int64_t microej_time_timeToTick(int64_t time);

#endif // _MICROEJ_TIME_H
