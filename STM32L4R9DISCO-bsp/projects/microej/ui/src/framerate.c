/*
 * Copyright 2014-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include <stdio.h>
#include "framerate_impl.h"
#include "microej.h"
#include "microej_time.h"

/* Defines -------------------------------------------------------------------*/

#ifdef FRAMERATE_ENABLED
#define NB_EVENTS 60
#define DIV(val, by) ((by) > 0 ? (val) / (by) : 0)
#define MS2FPS(ms) DIV(1000, (ms))
#endif

/* Globals -------------------------------------------------------------------*/

#ifdef FRAMERATE_ENABLED
static uint64_t times[NB_EVENTS];
static uint32_t delta[NB_EVENTS];
static uint64_t last_time = 0;
static uint32_t index = 0;
#endif

/* API -----------------------------------------------------------------------*/

void framerate_increment(void)
{
#ifdef FRAMERATE_ENABLED
	times[index] = microej_time_getcurrenttime(MICROEJ_TRUE);
	delta[index] = times[index] - last_time;
	last_time = times[index];
	index++;
	if (index >= NB_EVENTS)
	{
		index = 0;
	}
#endif
}

void framerate_get(uint32_t* average, uint32_t* peak)
{
#ifdef FRAMERATE_ENABLED
	uint32_t framerate_counter = 0;
	uint64_t min = microej_time_getcurrenttime(MICROEJ_TRUE) - 1000;
	uint32_t time_counter = 0;
	uint32_t _min_fps_ms = 1000;

	for(uint32_t i=0; i<NB_EVENTS; i++)
	{
		if (times[i] >= min)
		{
			++framerate_counter;
			time_counter += delta[i] ;
			if (delta[i] < _min_fps_ms)
			{
				_min_fps_ms = delta[i];
			}
		}
	}
	*average = MS2FPS(DIV(time_counter + (framerate_counter/2), framerate_counter));
	*peak = MS2FPS(_min_fps_ms);
#endif
}

/* EOF -----------------------------------------------------------------------*/
