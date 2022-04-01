/*
 * Copyright 2019-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */
/*
 * This file converts the input events in MicroUI Event Generator events.
 */

/* Includes ------------------------------------------------------------------*/

#include "LLUI_INPUT.h"
#include "LLMJVM.h"
#include "buttons_helper_configuration.h"

// this h file is created by buildSystemMicroUI step
#include "microui_constants.h"

/* Private functions ---------------------------------------------------------*/

/*
 * Get MicroUI button attributes according given button ID.
 * There are two MicroUI groups of buttons in this platform. They have each an
 * unique event generator ID: MICROUI_EVENTGEN_BUTTONS and MICROUI_EVENTGEN_JOYSTICK.
 * Each group has n buttons; the buttons ID are 0 to n-1.
 */
static int32_t get_joystick_command(int32_t button_id)
{
	switch (button_id) {
	default:
	case JOYSTICK_SEL_ID:
		LLMJVM_dump();
		return LLUI_INPUT_COMMAND_SELECT;
	case JOYSTICK_DOWN_ID:
		return LLUI_INPUT_COMMAND_DOWN;
	case JOYSTICK_UP_ID:
		return LLUI_INPUT_COMMAND_UP;
	case JOYSTICK_LEFT_ID:
		return LLUI_INPUT_COMMAND_LEFT;
	case JOYSTICK_RIGHT_ID:
		return LLUI_INPUT_COMMAND_RIGHT;
	}
}

/* Buttons -------------------------------------------------------------------*/

int32_t EVENT_GENERATOR_button_pressed(int32_t buttonId)
{
	// send a MicroUI Command event
	return LLUI_INPUT_sendCommandEvent(MICROUI_EVENTGEN_COMMANDS, get_joystick_command(buttonId));
}

int32_t EVENT_GENERATOR_button_repeated(int32_t buttonId)
{
	// send a MicroUI Command event
	return LLUI_INPUT_sendCommandEvent(MICROUI_EVENTGEN_COMMANDS, get_joystick_command(buttonId));
}

int32_t EVENT_GENERATOR_button_released(int32_t buttonId)
{
	// do not send a Command or Home event on release event
	return LLUI_INPUT_OK; // the event has been managed
}

/* Touch ---------------------------------------------------------------------*/

int32_t EVENT_GENERATOR_touch_pressed(int32_t x, int32_t y)
{
	return LLUI_INPUT_sendTouchPressedEvent(MICROUI_EVENTGEN_TOUCH, x, y);
}

int32_t EVENT_GENERATOR_touch_moved(int32_t x, int32_t y)
{
	return LLUI_INPUT_sendTouchMovedEvent(MICROUI_EVENTGEN_TOUCH, x, y);
}

int32_t EVENT_GENERATOR_touch_released(void)
{
	return LLUI_INPUT_sendTouchReleasedEvent(MICROUI_EVENTGEN_TOUCH);
}
