/*
 * Copyright 2019-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include "mfx_v3.h"
#include "stm32l4xx_hal.h"
#include "stm32l4r9i_discovery_io.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "microej.h"

/* Defines -------------------------------------------------------------------*/

#define MFX_V3_TASK_PRIORITY ( 12 )
#define MFX_V3_TASK_STACK_SIZE (256) // portSTACK_TYPE is 32 bits wide

/* Global --------------------------------------------------------------------*/

xSemaphoreHandle mfx_v3_task_sem;

/* Weak functions ------------------------------------------------------------*/

/**
 * Manages the Touchscreen IRQ events
 */
__weak uint8_t MFX_V3_Touch_Callback(void)
{
	return MICROEJ_FALSE;
}

/**
 * Manages the Joystick IRQ events
 */
__weak uint8_t MFX_V3_Joystick_Callback(void)
{
	return MICROEJ_FALSE;
}

/* Private API ---------------------------------------------------------------*/
static void MFX_V3_poll(void)
{
	uint8_t poll;

	do
	{
		poll = MICROEJ_FALSE;
		poll = MFX_V3_Joystick_Callback() == MICROEJ_TRUE ? MICROEJ_TRUE : poll;
		poll = MFX_V3_Touch_Callback() == MICROEJ_TRUE ? MICROEJ_TRUE : poll;

		if (poll)
		{
			vTaskDelay(20);
		}

	} while (poll == MICROEJ_TRUE);
}

static void MFX_V3_task(void *p_arg)
{
	while(1)
	{
		// Suspend ourselves
		xSemaphoreTake(mfx_v3_task_sem, portMAX_DELAY);

		// clear MFX_V3 interrupt
		BSP_IO_ITClear(IO_PIN_ALL);

		// We have been woken up, let's work !
		MFX_V3_poll();
	}
}

/* Interrupt function --------------------------------------------------------*/

/**
 * @brief  This function handles External lines 9_5 interrupt request.
 * @param  None
 * @retval None
 */
void EXTI1_IRQHandler(void)
{
	traceISR_ENTER();

	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);

	// send an event to wake up the task
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(mfx_v3_task_sem, &xHigherPriorityTaskWoken);
	if(xHigherPriorityTaskWoken != pdFALSE )
	{
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	}

	traceISR_EXIT();
}

/* API -----------------------------------------------------------------------*/

void MFX_V3_initialize(void)
{
	// create semaphore and task
	vSemaphoreCreateBinary(mfx_v3_task_sem);
	xSemaphoreTake(mfx_v3_task_sem, 0);
	xTaskCreate(MFX_V3_task, "MFX_V3_Task", MFX_V3_TASK_STACK_SIZE, NULL, MFX_V3_TASK_PRIORITY , NULL);
}
