/*
 * Copyright 2013-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#include "LLSP_impl.h"
#include "microej.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

// Mutex used to define critical sections
static xSemaphoreHandle LLSP_mutex = NULL;

void LLSP_IMPL_initialize(void)
{
	// Prevent preventive context switches only if necessary
	portENTER_CRITICAL();
	if( LLSP_mutex == NULL )
	{
		LLSP_mutex = xSemaphoreCreateMutex();
	}
	portEXIT_CRITICAL();
}

void LLSP_IMPL_syncWriteBlockEnter(int32_t databaseID, int32_t blockID)
{
	xSemaphoreTake(LLSP_mutex, portMAX_DELAY);
}

void LLSP_IMPL_syncWriteBlockExit(int32_t databaseID, int32_t blockID)
{
	xSemaphoreGive(LLSP_mutex);
}

void LLSP_IMPL_syncReadBlockEnter(int32_t databaseID, int32_t blockID)
{
	// Read synchronization not specific
	LLSP_IMPL_syncWriteBlockEnter(databaseID, blockID);
}

void LLSP_IMPL_syncReadBlockExit(int32_t databaseID, int32_t blockID)
{
	// Read synchronization not specific
	LLSP_IMPL_syncWriteBlockExit(databaseID, blockID);
}

int32_t LLSP_IMPL_wait(void)
{
	// Suspend ourselves until we are woken up
	vTaskSuspend( xTaskGetCurrentTaskHandle() );
	return LLSP_OK;
}

void LLSP_IMPL_wakeup(int32_t taskID)
{
	// TaskID is a handle.
	// See LLMJVM_IMPL_getCurrentTaskID() function in LLMJVM_FreeRTOS.c
	vTaskResume( (xTaskHandle)taskID );
}

