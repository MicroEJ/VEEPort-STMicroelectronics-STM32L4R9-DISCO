/*
 * C
 *
 * Copyright 2019 MicroEJ Corp. All rights reserved.
 * For demonstration purpose only.
 * MicroEJ Corp. PROPRIETARY. Use is subject to license terms.
 */

/* Includes ------------------------------------------------------------------*/

#include "LLINPUT_impl.h"
#include "microej.h"
#include "mfx_v3.h"
#include "joystick_manager.h"
#include "touch_manager.h"
#include "os_support.h"

/* API -----------------------------------------------------------------------*/

void LLINPUT_IMPL_initialize(void)
{
	MFX_V3_initialize();
	JOYSTICK_MANAGER_initialize();
	TOUCH_MANAGER_initialize();
}

int32_t LLINPUT_IMPL_getInitialStateValue(int32_t stateMachinesID, int32_t stateID)
{
	// no state on this BSP
	return 0;
}

void LLINPUT_IMPL_enterCriticalSection()
{
	OS_SUPPORT_disable_context_switching();
	JOYSTICK_MANAGER_disable_interrupt();
}

void LLINPUT_IMPL_leaveCriticalSection()
{
	JOYSTICK_MANAGER_enable_interrupt();
	OS_SUPPORT_enable_context_switching();
}
