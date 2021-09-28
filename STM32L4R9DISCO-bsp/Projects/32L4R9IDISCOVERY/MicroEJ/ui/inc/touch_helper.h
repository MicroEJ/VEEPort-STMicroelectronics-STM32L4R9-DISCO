/*
 * C
 *
 * Copyright 2015-2019 MicroEJ Corp. All rights reserved.
 * For demonstration purpose only.
 * MicroEJ Corp. PROPRIETARY. Use is subject to license terms.
 */

#ifndef _TOUCH_HELPER
#define _TOUCH_HELPER

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>

/* API -----------------------------------------------------------------------*/

/*
 * Notify to an event handler a touch has been pressed.
 * @param x the pointer X coordinate
 * @param y the pointer Y coordinate
 */
void TOUCH_HELPER_pressed(int32_t x, int32_t y);

/*
 * Notify to an event handler a touch has moved.
 * @param x the pointer X coordinate
 * @param y the pointer Y coordinate
 */
void TOUCH_HELPER_moved(int32_t x, int32_t y);

/*
 * Notify to an event handler a touch has been released.
 */
void TOUCH_HELPER_released(void);

#endif
