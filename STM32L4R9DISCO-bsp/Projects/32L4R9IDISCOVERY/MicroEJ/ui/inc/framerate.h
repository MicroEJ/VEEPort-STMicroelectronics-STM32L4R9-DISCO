/*
 * C
 *
 * Copyright 2014-2019 MicroEJ Corp. All rights reserved.
 * For demonstration purpose only.
 * MicroEJ Corp. PROPRIETARY. Use is subject to license terms.
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
