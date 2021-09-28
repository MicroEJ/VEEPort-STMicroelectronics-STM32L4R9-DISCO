/*
 * C
 *
 * Copyright 2014-2019 MicroEJ Corp. All rights reserved.
 * For demonstration purpose only.
 * MicroEJ Corp. PROPRIETARY. Use is subject to license terms.
 */

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include <stdio.h>
#include "framerate_impl.h"
#include "time_hardware_timer.h"

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
	times[index] = time_hardware_timer_getCurrentTime();
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
	uint64_t min = time_hardware_timer_getCurrentTime() - 1000;
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
