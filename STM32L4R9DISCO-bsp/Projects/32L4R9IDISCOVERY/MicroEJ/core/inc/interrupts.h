/*
 * C
 *
 * Copyright 2013-2019 MicroEJ Corp. All rights reserved.
 * For demonstration purpose only.
 * MicroEJ Corp. PROPRIETARY. Use is subject to license terms.
 */

#ifndef _INTERRUPTS
#define _INTERRUPTS

/* Includes ------------------------------------------------------------------*/

#include "microej.h"

/* API -----------------------------------------------------------------------*/

/**
 * This function must be called when entering in an interrupt.
 */
uint8_t interrupt_enter(void);


/**
 * This function must be called when leaving an interrupt.
 */
void interrupt_leave(uint8_t leave);

/**
 * This function returns MICROEJ_TRUE or MICROEJ_FALSE to indicate if an ISR is currently executed.
 */
uint8_t interrupt_is_in(void);

#endif
