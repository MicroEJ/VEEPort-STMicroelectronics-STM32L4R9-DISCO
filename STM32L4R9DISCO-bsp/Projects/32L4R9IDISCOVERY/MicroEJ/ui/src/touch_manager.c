/*
 * C
 *
 * Copyright 2019 MicroEJ Corp. All rights reserved.
 * For demonstration purpose only.
 * MicroEJ Corp. PROPRIETARY. Use is subject to license terms.
 */

/* Includes ------------------------------------------------------------------*/

#include "touch_manager.h"
#include "stm32l4r9i_discovery_ts.h"
#include "microej.h"
#include "touch_helper.h"
#include "lcd_driver.h"

/* Global --------------------------------------------------------------------*/

static uint8_t touch_initialized = MICROEJ_FALSE;

/* MFX_V3 weak function ------------------------------------------------------*/

/**
 * Manages the Touch IRQ events
 */
uint8_t MFX_V3_Touch_Callback(void)
{
	if (touch_initialized == MICROEJ_FALSE)
	{
		return MICROEJ_FALSE;
	}

	BSP_TS_ITClear();

	TS_StateTypeDef TS_State;
	uint8_t poll = MICROEJ_FALSE;

	if (BSP_TS_GetState(&TS_State) == TS_OK && TS_State.touchDetected > 0)
	{
		// here, pen is down for sure
		TOUCH_HELPER_pressed(TS_State.touchX[0], TS_State.touchY[0]);
		poll = MICROEJ_TRUE;
	}
	else
	{
		// here, pen is up for sure
		TOUCH_HELPER_released();
		poll = MICROEJ_FALSE;
	}

	return poll;
}

/* API -----------------------------------------------------------------------*/

void TOUCH_MANAGER_initialize(void)
{	
	BSP_TS_Init(LCD_WIDTH, LCD_HEIGHT);
	BSP_TS_ITConfig();
	touch_initialized = MICROEJ_TRUE;
}
