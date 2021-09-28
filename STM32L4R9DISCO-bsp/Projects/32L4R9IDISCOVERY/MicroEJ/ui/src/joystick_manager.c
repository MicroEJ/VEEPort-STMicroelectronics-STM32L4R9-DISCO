/*
 * C
 *
 * Copyright 2019 MicroEJ Corp. All rights reserved.
 * For demonstration purpose only.
 * MicroEJ Corp. PROPRIETARY. Use is subject to license terms.
 */

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include "joystick_manager.h"
#include "microej.h"
#include "interrupts.h"
#include "stm32l4r9i_discovery_io.h"
#include "stm32l4r9i_discovery.h"
#include "buttons_helper.h"
#include "buttons_helper_configuration.h"
#include "FreeRTOS.h"
#include "time_hardware_timer.h"

/* Defines -------------------------------------------------------------------*/

#define JOYSTICK_THRESHOLD_MS 150

/* Global --------------------------------------------------------------------*/

#ifndef JOYSTICK_DISABLE_MFX
static uint8_t joystick_initialized = MICROEJ_FALSE;
static uint64_t last_pressed_time;
#endif

/* Private functions ---------------------------------------------------------*/

#ifndef JOYSTICK_DISABLE_MFX

static void joy_pressed(uint32_t joy_id)
{
	if (time_hardware_timer_getCurrentTime() - last_pressed_time >= JOYSTICK_THRESHOLD_MS)
	{
		BUTTONS_HELPER_pressed(joy_id);
		last_pressed_time = time_hardware_timer_getCurrentTime();
	}
}

#endif // JOYSTICK_DISABLE_MFX

/* MFX_V3 weak function ------------------------------------------------------*/

#ifndef JOYSTICK_DISABLE_MFX

/**
 * Manages the Joystick IRQ events
 */
uint8_t MFX_V3_Joystick_Callback(void)
{
	if (joystick_initialized == MICROEJ_FALSE)
	{
		return MICROEJ_FALSE;
	}

	JOYState_TypeDef state = BSP_JOY_GetState();

	switch (state)
	{
	case JOY_DOWN:
		joy_pressed(JOYSTICK_DOWN_ID);
		break;
	case JOY_LEFT:
		joy_pressed(JOYSTICK_LEFT_ID);
		break;
	case JOY_RIGHT:
		joy_pressed(JOYSTICK_RIGHT_ID);
		break;
	case JOY_UP:
		joy_pressed(JOYSTICK_UP_ID);
		break;
	case JOY_NONE:
		BUTTONS_HELPER_released(JOYSTICK_DOWN_ID);
		BUTTONS_HELPER_released(JOYSTICK_LEFT_ID);
		BUTTONS_HELPER_released(JOYSTICK_RIGHT_ID);
		BUTTONS_HELPER_released(JOYSTICK_UP_ID);
		break;
	}

	return state == JOY_NONE ? MICROEJ_FALSE : MICROEJ_TRUE;
}

#endif // JOYSTICK_DISABLE_MFX

/* Interrupt function --------------------------------------------------------*/

/**
 * @brief  This function handles External line 0 interrupt request.
 * @param  None
 * @retval None
 */
void EXTI15_10_IRQHandler(void)
{
	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_13) != RESET)
	{
		uint8_t leave = interrupt_enter();

		if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_SET)
		{
			// GPIO == 1 means "pressed"
			BUTTONS_HELPER_pressed(JOYSTICK_SEL_ID);
		}
		else
		{
			// GPIO == 0 means "released"
			BUTTONS_HELPER_released(JOYSTICK_SEL_ID);
		}

		interrupt_leave(leave);
		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_13);
	}
}

/* API -----------------------------------------------------------------------*/

void JOYSTICK_MANAGER_initialize(void)
{
	BSP_JOY_Init(JOY_MODE_EXTI);

#ifdef JOYSTICK_DISABLE_MFX
	// BSP_JOY_Init has initialized MFX buttons: disable them
	BSP_IO_ConfigPin((RIGHT_JOY_PIN | LEFT_JOY_PIN | UP_JOY_PIN | DOWN_JOY_PIN), IO_MODE_OFF);
#else
	last_pressed_time = time_hardware_timer_getCurrentTime();
	joystick_initialized = MICROEJ_TRUE;
#endif
}

void JOYSTICK_MANAGER_enable_interrupt(void)
{
	HAL_NVIC_EnableIRQ(SEL_JOY_EXTI_IRQn);
}

void JOYSTICK_MANAGER_disable_interrupt(void)
{
	HAL_NVIC_DisableIRQ(SEL_JOY_EXTI_IRQn);
}
