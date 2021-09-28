/*
 * C
 *
 * Copyright 2013-2019 MicroEJ Corp. All rights reserved.
 * For demonstration purpose only.
 * MicroEJ Corp. PROPRIETARY. Use is subject to license terms.
 */

/* Includes ------------------------------------------------------------------*/

#include "interrupts.h"

/* Global --------------------------------------------------------------------*/

static volatile uint8_t is_in_ISR = MICROEJ_FALSE;

/* Public functions ----------------------------------------------------------*/

uint8_t interrupt_is_in(void)
{
	return is_in_ISR;
}

uint8_t interrupt_enter(void)
{
	if (is_in_ISR == MICROEJ_TRUE)
	{
		return MICROEJ_FALSE;
	}
	else
	{
		is_in_ISR = MICROEJ_TRUE;
		return MICROEJ_TRUE;
	}
}

void interrupt_leave(uint8_t leave)
{
	if (leave == MICROEJ_TRUE)
	{
		is_in_ISR = MICROEJ_FALSE;
	}
}
