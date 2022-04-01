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
static DMA2D_CLUTCfgTypeDef hdma2d_lut;
static DMA2D_LayerDestinationTypeDef DMA2D_LayerDestinationInitStruct;
static DMA2D_LayerForegroundTypeDef DMA2D_LayerForegroundInitStruct;

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

static void lcd_dma2d_wakeup(void)
{
	LLUI_DISPLAY_notifyAsynchronousDrawingEnd(true);
}

/*
 * Fill a buffer area whose size is equal to LCD buffer size.
 */
static void lcd_fillRect(uint32_t bufferAddress, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t bufferWidth, uint32_t color)
{
	// adjust start address
	bufferAddress += (y * bufferWidth + x) * LCD_BPP/8;

	hdma2d_eval.Init.Mode         = DMA2D_R2M;
	hdma2d_eval.Init.OutputOffset = bufferWidth - width;
	HAL_DMA2D_Init(&hdma2d_eval) ;
	HAL_DMA2D_Start_IT(&hdma2d_eval, color, bufferAddress, width, height);
}

/*
 * destination is always a buffer whose size is equal to LCD buffer size.
 */
static void lcd_blend(void)
{
	uint32_t backgroundAddress = DMA2D_LayerDestinationInitStruct.address;
	uint32_t foregroundAddr = DMA2D_LayerForegroundInitStruct.address;

	// adjust start addresses
	backgroundAddress += (DMA2D_LayerDestinationInitStruct.y * DMA2D_LayerDestinationInitStruct.width + DMA2D_LayerDestinationInitStruct.x) * LCD_BPP/8;
	foregroundAddr += (DMA2D_LayerForegroundInitStruct.area_y * DMA2D_LayerForegroundInitStruct.width + DMA2D_LayerForegroundInitStruct.area_x) * DMA2D_LayerForegroundInitStruct.bpp/8;

	// destination
	hdma2d_eval.Init.Mode         = DMA2D_M2M_BLEND;
	hdma2d_eval.Init.OutputOffset = DMA2D_LayerDestinationInitStruct.width - DMA2D_LayerForegroundInitStruct.area_width;

	// background
	hdma2d_eval.LayerCfg[0].InputOffset = DMA2D_LayerDestinationInitStruct.width - DMA2D_LayerForegroundInitStruct.area_width;
	hdma2d_eval.LayerCfg[0].InputColorMode = LCD_BPP_DMA2D_COLOR_MODE;
	hdma2d_eval.LayerCfg[0].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	hdma2d_eval.LayerCfg[0].InputAlpha = 255;
	HAL_DMA2D_ConfigLayer(&hdma2d_eval, 0);

	// foreground
	HAL_DMA2D_DisableCLUT(&hdma2d_eval, 1);
	hdma2d_eval.LayerCfg[1].InputOffset = DMA2D_LayerForegroundInitStruct.width - DMA2D_LayerForegroundInitStruct.area_width;
	hdma2d_eval.LayerCfg[1].InputColorMode = DMA2D_LayerForegroundInitStruct.format;
	hdma2d_eval.LayerCfg[1].AlphaMode = DMA2D_COMBINE_ALPHA;
	hdma2d_eval.LayerCfg[1].InputAlpha = DMA2D_LayerForegroundInitStruct.alphaAndColor;
	HAL_DMA2D_ConfigLayer(&hdma2d_eval, 1);
	HAL_DMA2D_Init(&hdma2d_eval) ;
	if (DMA2D_LayerForegroundInitStruct.format == CM_L4 || DMA2D_LayerForegroundInitStruct.format == CM_L8)
	{
		hdma2d_lut.pCLUT = (uint32_t*)DMA2D_LayerForegroundInitStruct.lut_address;
		hdma2d_lut.CLUTColorMode = DMA2D_LayerForegroundInitStruct.lut_color_mode;
		hdma2d_lut.Size = DMA2D_LayerForegroundInitStruct.lut_size;
		HAL_DMA2D_ConfigCLUT(&hdma2d_eval, hdma2d_lut, 1);
		HAL_DMA2D_EnableCLUT(&hdma2d_eval, 1);
	}

	HAL_DMA2D_BlendingStart_IT(&hdma2d_eval, foregroundAddr, backgroundAddress, backgroundAddress, DMA2D_LayerForegroundInitStruct.area_width,  DMA2D_LayerForegroundInitStruct.area_height);
}

static void lcd_prepare_A48_data(jint color, uint32_t* alphaAndColor)
{
	// for A4 and A8 formats, alphaAndColor is both the global alpha + wanted color
	*(alphaAndColor) <<= 24;
	*(alphaAndColor) |= (color & 0xffffff);
}

static void lcd_init_destination_struct(uint32_t address, int32_t x_dest, int32_t y_dest, uint16_t width)
{
	DMA2D_LayerDestinationInitStruct.address = address;
	DMA2D_LayerDestinationInitStruct.width = width;
	DMA2D_LayerDestinationInitStruct.x = (uint32_t)x_dest;
	DMA2D_LayerDestinationInitStruct.y = (uint32_t)y_dest;
}

static void lcd_init_foreground_struct(uint32_t address, int32_t alpha, int32_t x_src, int32_t y_src, int32_t width_src, int32_t height_src, int32_t width)
{
	DMA2D_LayerForegroundInitStruct.address = address;
	DMA2D_LayerForegroundInitStruct.alphaAndColor = alpha;
	DMA2D_LayerForegroundInitStruct.area_x = (uint32_t)x_src;
	DMA2D_LayerForegroundInitStruct.area_y = (uint32_t)y_src;
	DMA2D_LayerForegroundInitStruct.area_width = (uint32_t)width_src;
	DMA2D_LayerForegroundInitStruct.area_height = (uint32_t)height_src;
	DMA2D_LayerForegroundInitStruct.width = width;
}

static void lcd_dma2d_init(void)
{
	hdma2d_eval.Init.ColorMode = LCD_BPP_DMA2D_COLOR_MODE;
	hdma2d_eval.Instance = DMA2D;

	/** @brief Enable the DMA2D clock */
	__HAL_RCC_DMA2D_CLK_ENABLE();

	/** @brief Toggle Sw reset of DMA2D IP */
	__HAL_RCC_DMA2D_FORCE_RESET();
	__HAL_RCC_DMA2D_RELEASE_RESET();

	/** @brief NVIC configuration for DMA2D interrupt that is now enabled */
	HAL_NVIC_SetPriority(DMA2D_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(DMA2D_IRQn);
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
	HAL_DMA2D_IRQHandler(&hdma2d_eval);
	lcd_dma2d_wakeup();
}

/* Public functions ----------------------------------------------------------*/

void LLUI_DISPLAY_IMPL_initialize(LLUI_DISPLAY_SInitData* init_data)
{
	lcd_dma2d_init();

	init_data->binary_semaphore_0 = (LLUI_DISPLAY_binary_semaphore*)xSemaphoreCreateBinary();
	init_data->binary_semaphore_1 = (LLUI_DISPLAY_binary_semaphore*)xSemaphoreCreateBinary();
	init_data->back_buffer_address = (uint8_t*)GFXMMU_VIRTUAL_BUFFER0_BASE;
	init_data->lcd_width = LCD_WIDTH;
	init_data->lcd_height = LCD_HEIGHT;
	init_data->memory_width = BSP_LCD_IMAGE_WIDTH;

	LCD_DRIVER_initialize((uint8_t*)BACK_BUFFER);
	LCD_DRIVER_set_backlight(100);
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

DRAWING_Status UI_DRAWING_fillRectangle(MICROUI_GraphicsContext* gc, jint x1, jint y1, jint x2, jint y2)
{
	LLUI_DISPLAY_setDrawingLimits(x1, y1, x2, y2);

	// use DMA2D to perform the drawing
	uint32_t destAddr = (uint32_t) LLUI_DISPLAY_getBufferAddress(&gc->image);
	lcd_fillRect(destAddr, x1, y1, x2-x1+1, y2-y1+1, LLUI_DISPLAY_getStrideInPixels(&gc->image), gc->foreground_color);

	return DRAWING_RUNNING;
}

DRAWING_Status UI_DRAWING_drawImage(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint regionX, jint regionY, jint width, jint height, jint x, jint y, jint alpha)
{
	uint8_t format, bpp;
	uint32_t alphaAndColor;

	format = img->format;

	// for standard formats, alphaAndColor is a value between 0x00 and 0xff (only global alpha)
	alphaAndColor = alpha;

	switch(format)
	{
	case MICROUI_IMAGE_FORMAT_RGB565:
		format = CM_RGB565;
		bpp = 16;
		break;
	case MICROUI_IMAGE_FORMAT_ARGB8888:
		format = CM_ARGB8888;
		bpp = 32;
		break;
	case MICROUI_IMAGE_FORMAT_RGB888:
		format = CM_RGB888;
		bpp = 24;
		break;
	case MICROUI_IMAGE_FORMAT_ARGB1555:
		format = CM_ARGB1555;
		bpp = 16;
		break;
	case MICROUI_IMAGE_FORMAT_ARGB4444:
		format = CM_ARGB4444;
		bpp = 16;
		break;
	case MICROUI_IMAGE_FORMAT_A4:
		lcd_prepare_A48_data(gc->foreground_color, &alphaAndColor);
		format = CM_A4;
		bpp = 4;
		break;
	case MICROUI_IMAGE_FORMAT_A8:
		lcd_prepare_A48_data(gc->foreground_color, &alphaAndColor);
		format = CM_A8;
		bpp = 8;
		break;
	default:
		// should never occur (checked by caller)
		return DRAWING_DONE;
	}

	LLUI_DISPLAY_setDrawingLimits(x, y, x+width-1, y+height-1);

	uint32_t destAddr = (uint32_t) LLUI_DISPLAY_getBufferAddress(&gc->image);
	lcd_init_destination_struct(destAddr, x, y, LLUI_DISPLAY_getStrideInPixels(&gc->image));
	uint32_t srcAddr = (uint32_t) LLUI_DISPLAY_getBufferAddress(img);
	lcd_init_foreground_struct(srcAddr, alphaAndColor, regionX, regionY, width, height, LLUI_DISPLAY_getStrideInPixels(img));
	DMA2D_LayerForegroundInitStruct.format = format;
	DMA2D_LayerForegroundInitStruct.bpp = bpp;

	lcd_blend();

	return DRAWING_RUNNING;
}

/* EOF -----------------------------------------------------------------------*/
