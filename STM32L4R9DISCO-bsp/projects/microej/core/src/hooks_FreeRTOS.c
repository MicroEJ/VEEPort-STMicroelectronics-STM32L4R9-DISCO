/*
 * Copyright 2013-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/*
 * This file implements the application hooks functions needed by FreeRTOS.
 * These functions trace problems that occurs during execution.
 * Their names are defined by FreeRTOS
 * and must be implemented (here) or desactivated (in ConfigFreeRTOS.h).
 *
 * See "Hook Functions [More Advanced]" on FreeRTOS Website.
 * http://www.freertos.org/a00016.hmtl
 */

/* Includes ------------------------------------------------------------------*/

#include <stdio.h>
#include <stdint.h>

/* Public functions ----------------------------------------------------------*/

void vApplicationStackOverflowHook(void)
{
	printf("%s\n", __FUNCTION__);
	while(1)
	{} // Trap when a stack overflow occurs
}

void vApplicationMallocFailedHook(void)
{
	printf("%s\n", __FUNCTION__);
	while(1)
	{} // Trap when a call to malloc fails
}
