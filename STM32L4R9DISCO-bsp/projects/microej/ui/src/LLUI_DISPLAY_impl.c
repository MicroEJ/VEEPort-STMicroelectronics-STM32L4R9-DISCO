/*
 * Copyright 2019-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/* Includes ------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "stm32l4xx_hal.h"
#include "LLUI_DISPLAY.h"
#include "LLUI_DISPLAY_impl.h"
#include <LLTRACE_impl.h>
#include "lcd_driver.h"
#include "bsp_util.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "framerate.h"
#include "drawing_dma2d.h"

/* Defines -------------------------------------------------------------------*/

// declare MicroEJ Display buffer
BSP_DECLARE_BUFFER(DISPLAY_MEM)
#define BACK_BUFFER (DISPLAY_MEM_START)

#if LCD_BPP == 16
#define LCD_BPP_DMA2D_COLOR_MODE DMA2D_RGB565
#elif LCD_BPP == 24
#define LCD_BPP_DMA2D_COLOR_MODE DMA2D_RGB888
#elif LCD_BPP == 32
#define LCD_BPP_DMA2D_COLOR_MODE DMA2D_ARGB8888
#else
#error "Invalid BPP, please set a value between 16, 24 and 32"
#endif

/* Structs -------------------------------------------------------------------*/

/*
 * destination, same memory layout and BPP than LCD frame buffer
 */
typedef struct
{
	uint32_t address;
	uint16_t width;
	uint16_t x;
	uint16_t y;
} DMA2D_LayerDestinationTypeDef;

/*
 * foreground is used for copy and blend modes
 */
typedef struct
{
	uint32_t address;
	uint32_t alphaAndColor;
	uint32_t lut_address;

	uint16_t width;
	uint16_t area_x;
	uint16_t area_y;
	uint16_t area_width;
	uint16_t area_height;

	uint8_t format;
	uint8_t bpp;
	uint8_t lut_color_mode;
	uint8_t lut_size;
} DMA2D_LayerForegroundTypeDef;

/* Private fields ------------------------------------------------------------*/

static DMA2D_HandleTypeDef hdma2d_eval;
static SemaphoreHandle_t dma2d_sem;

/* Private functions ---------------------------------------------------------*/

static void semaphore_wait(SemaphoreHandle_t sem)
{
	xSemaphoreTake(sem, portMAX_DELAY);
}

static void semaphore_notify(SemaphoreHandle_t sem, bool from_isr)
{
	if (from_isr) {
		portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR(sem, &xHigherPriorityTaskWoken);
		if (xHigherPriorityTaskWoken != pdFALSE) {
			// Force a context switch here.
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
	} else {
		xSemaphoreGive(sem);
	}
}


static void lcd_dma2d_init(void)
{
	/** @brief Enable the DMA2D clock */
	__HAL_RCC_DMA2D_CLK_ENABLE();

	/** @brief Toggle Sw reset of DMA2D IP */
	__HAL_RCC_DMA2D_FORCE_RESET();
	__HAL_RCC_DMA2D_RELEASE_RESET();

}

/* Interrupt functions -------------------------------------------------------*/

/**
 * @brief  End of Refresh DSI callback.
 */
void HAL_DSI_EndOfRefreshCallback(DSI_HandleTypeDef *hdsi)
{
	LLUI_DISPLAY_flushDone(true);
}

void DMA2D_IRQHandler(void)
{
        DRAWING_DMA2D_IRQHandler();
}

/* Public functions ----------------------------------------------------------*/

void LLUI_DISPLAY_IMPL_initialize(LLUI_DISPLAY_SInitData* init_data)
{
	lcd_dma2d_init();

	init_data->binary_semaphore_0 = (LLUI_DISPLAY_binary_semaphore*)xSemaphoreCreateBinary();
	init_data->binary_semaphore_1 = (LLUI_DISPLAY_binary_semaphore*)xSemaphoreCreateBinary();
#ifdef USE_GFXMMU
	init_data->back_buffer_address = (uint8_t*)GFXMMU_VIRTUAL_BUFFER0_BASE;
#else
	init_data->back_buffer_address = (uint8_t*)BACK_BUFFER;
#endif
	init_data->lcd_width = LCD_WIDTH;
	init_data->lcd_height = LCD_HEIGHT;
	init_data->memory_width = BSP_LCD_IMAGE_WIDTH;

	LCD_DRIVER_initialize((uint8_t*)BACK_BUFFER);
	LCD_DRIVER_set_backlight(100);
        
	dma2d_sem = xSemaphoreCreateBinary();
        DRAWING_DMA2D_initialize((void*)dma2d_sem);
}

uint8_t* LLUI_DISPLAY_IMPL_flush(MICROUI_GraphicsContext* gc, uint8_t* sourceAddr, uint32_t xmin, uint32_t ymin, uint32_t xmax, uint32_t ymax)
{
#ifdef FRAMERATE_ENABLED
	framerate_increment();
#endif
	LCD_DRIVER_refresh();
	return sourceAddr;
}

int32_t LLUI_DISPLAY_IMPL_getHeight(void)
{
	return LCD_HEIGHT;
}

int32_t LLUI_DISPLAY_IMPL_getWidth(void)
{
	return LCD_WIDTH;
}

void LLUI_DISPLAY_IMPL_binarySemaphoreTake(void* binary_semaphore)
{
	semaphore_wait(binary_semaphore);
}

void LLUI_DISPLAY_IMPL_binarySemaphoreGive(void* binary_semaphore, bool from_isr)
{
	semaphore_notify(binary_semaphore, from_isr);
}

bool LLUI_DISPLAY_IMPL_hasBacklight(void)
{
	return true;
}

void LLUI_DISPLAY_IMPL_setBacklight(uint32_t backlight)
{
	LCD_DRIVER_set_backlight(backlight);
}

uint32_t LLUI_DISPLAY_IMPL_getBacklight(void)
{
	return LCD_DRIVER_get_backlight();
}

/* EOF -----------------------------------------------------------------------*/
