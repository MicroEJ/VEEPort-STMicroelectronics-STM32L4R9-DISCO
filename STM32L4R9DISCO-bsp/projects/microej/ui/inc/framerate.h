/*
 * Copyright 2014-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */
#ifndef _FRAMERATE_INTERN
#define _FRAMERATE_INTERN

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include "framerate_conf.h"

/* Defines -------------------------------------------------------------------*/

#define FRAMERATE_OK 0
#define FRAMERATE_ERROR -1

/* API -----------------------------------------------------------------------*/

/*
 * Update the framerate counter
 */
void framerate_increment(void);

/*
 * Return the last framerate
 */
void framerate_get(uint32_t* average, uint32_t* peak);

#endif	// _FRAMERATE_INTERN
