/*
 * C
 *
 * Copyright 2013-2019 MicroEJ Corp. All rights reserved.
 * For demonstration purpose only.
 * MicroEJ Corp. PROPRIETARY. Use is subject to license terms.
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
