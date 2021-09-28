/*
 * Copyright 2019 MicroEJ Corp. All rights reserved.
 * For demonstration purpose only.
 * MicroEJ Corp. PROPRIETARY. Use is subject to license terms.
 */

/* Includes ------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "stm32l4xx_hal.h"
#include "LLDISPLAY_impl.h"
#include "LLDISPLAY_UTILS.h"
#include "LLDISPLAY_EXTRA_impl.h"
#include "LLDISPLAY_EXTRA_drawing.h"
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
static SemaphoreHandle_t dma2d_sem;
static uint8_t dma2d_running;
static DMA2D_LayerDestinationTypeDef DMA2D_LayerDestinationInitStruct;
static DMA2D_LayerForegroundTypeDef DMA2D_LayerForegroundInitStruct;
static SemaphoreHandle_t flush_sem;

/* Private functions ---------------------------------------------------------*/

static void semaphore_wait(SemaphoreHandle_t* sem)
{
	xSemaphoreTake(*sem, portMAX_DELAY);
}

static void semaphore_notify(SemaphoreHandle_t* sem)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(*sem, &xHigherPriorityTaskWoken);
	if(xHigherPriorityTaskWoken != pdFALSE )
	{
		// Force a context switch here.
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

static void lcd_dma2d_wait(void)
{
	while(dma2d_running == 1)
	{
		semaphore_wait(&dma2d_sem);
	}
}

static void lcd_dma2d_wakeup(void)
{
	dma2d_running = 0;
	LLDISPLAY_UTILS_drawingDone();
	semaphore_notify(&dma2d_sem);
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
	dma2d_running = 1;
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

	dma2d_running = 1;
	HAL_DMA2D_BlendingStart_IT(&hdma2d_eval, foregroundAddr, backgroundAddress, backgroundAddress, DMA2D_LayerForegroundInitStruct.area_width,  DMA2D_LayerForegroundInitStruct.area_height);
}

static void lcd_prepare_A48_data(void* drawing, uint32_t* alphaAndColor)
{
	// for A4 and A8 formats, alphaAndColor is both the global alpha + wanted color
	*(alphaAndColor) <<= 24;
	*(alphaAndColor) |= (((LLDISPLAY_SDrawImage*)drawing)->color & 0xffffff);
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
	dma2d_running = 0;
	vSemaphoreCreateBinary(dma2d_sem);
	xSemaphoreTake(dma2d_sem, 0);
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
	LLDISPLAY_UTILS_flushDone();
	semaphore_notify(&flush_sem);
}

void DMA2D_IRQHandler(void)
{
	HAL_DMA2D_IRQHandler(&hdma2d_eval);
	lcd_dma2d_wakeup();
}

/* Public functions ----------------------------------------------------------*/

void LLDISPLAY_IMPL_initialize(void)
{
	lcd_dma2d_init();

	vSemaphoreCreateBinary(flush_sem);
	xSemaphoreTake(flush_sem, 0);

	LCD_DRIVER_initialize((uint8_t*)BACK_BUFFER);
	LCD_DRIVER_set_backlight(100);
}

int32_t LLDISPLAY_IMPL_flush(int32_t srcAddr, int32_t xmin, int32_t ymin, int32_t xmax, int32_t ymax)
{
#ifdef FRAMERATE_ENABLED
	framerate_increment();
#endif
	LCD_DRIVER_refresh();
	return srcAddr;
}

int32_t LLDISPLAY_IMPL_getGraphicsBufferAddress(void)
{
#ifdef USE_GFXMMU
	return (int32_t)GFXMMU_VIRTUAL_BUFFER0_BASE;
#else
	return (int32_t)BACK_BUFFER;
#endif
}

#ifdef USE_GFXMMU
int32_t LLDISPLAY_EXTRA_IMPL_getGraphicsBufferMemoryWidth(void)
{
	return BSP_LCD_IMAGE_WIDTH;
}
#endif

int32_t LLDISPLAY_IMPL_getHeight(void)
{
	return LCD_HEIGHT;
}

int32_t LLDISPLAY_IMPL_getWidth(void)
{
	return LCD_WIDTH;
}

void LLDISPLAY_IMPL_synchronize(void)
{
	semaphore_wait(&flush_sem);
}

int32_t LLDISPLAY_EXTRA_IMPL_hasBackLight(void)
{
	return (int32_t)LLDISPLAY_EXTRA_OK;
}

void LLDISPLAY_EXTRA_IMPL_setBacklight(int32_t value)
{
	LCD_DRIVER_set_backlight(value);
}

int32_t LLDISPLAY_EXTRA_IMPL_getBacklight(void)
{
	return (int32_t)LCD_DRIVER_get_backlight();
}

void LLDISPLAY_EXTRA_IMPL_waitPreviousDrawing(void)
{
	// wait the previous DMA2D copy or blend
	lcd_dma2d_wait();
}

void LLDISPLAY_EXTRA_IMPL_error(int32_t errorCode)
{
	printf("lldisplay error: %d\n", errorCode);
	while(1);
}

int32_t LLDISPLAY_EXTRA_IMPL_fillRect(void* dest, int32_t destAddr, void* rect, int32_t color)
{
	// ensure DMA2D is free
	lcd_dma2d_wait();

	// use DMA2D to perform the drawing
	lcd_fillRect(destAddr, ((LLDISPLAY_SRectangle*)rect)->x, ((LLDISPLAY_SRectangle*)rect)->y, ((LLDISPLAY_SRectangle*)rect)->width, ((LLDISPLAY_SRectangle*)rect)->height, ((LLDISPLAY_SImage*)dest)->width, color);
	return LLDISPLAY_EXTRA_DRAWING_RUNNING;
}

int32_t LLDISPLAY_EXTRA_IMPL_drawImage(void* src, int32_t srcAddr, void* dest, int32_t destAddr, void* drawing)
{
	// ensure DMA2D is free
	lcd_dma2d_wait();

	uint8_t format, bpp;
	uint32_t alphaAndColor;

	format = ((LLDISPLAY_SImage*)src)->format;

	// for standard formats, alphaAndColor is a value between 0x00 and 0xff (only global alpha)
	alphaAndColor = ((LLDISPLAY_SDrawImage*)drawing)->alpha;

	int32_t src_width = ((LLDISPLAY_SImage*)src)->width;

	switch(format)
	{
	case LLDISPLAY_EXTRA_IMAGE_RGB565:
		format = CM_RGB565;
		bpp = 16;
		break;
	case LLDISPLAY_EXTRA_IMAGE_ARGB8888:
		format = CM_ARGB8888;
		bpp = 32;
		break;
	case LLDISPLAY_EXTRA_IMAGE_RGB888:
		format = CM_RGB888;
		bpp = 24;
		break;
	case LLDISPLAY_EXTRA_IMAGE_ARGB1555:
		format = CM_ARGB1555;
		bpp = 16;
		break;
	case LLDISPLAY_EXTRA_IMAGE_ARGB4444:
		format = CM_ARGB4444;
		bpp = 16;
		break;
	case LLDISPLAY_EXTRA_IMAGE_A4:
		lcd_prepare_A48_data(drawing, &alphaAndColor);
		if ((src_width & 1) == 1)
		{
			// even software width
			// -> retrieve hardware width
			++src_width;
		}
		format = CM_A4;
		bpp = 4;
		break;
	case LLDISPLAY_EXTRA_IMAGE_A8:
		lcd_prepare_A48_data(drawing, &alphaAndColor);
		format = CM_A8;
		bpp = 8;
		break;
	default:
		// should never occur (checked by caller)
		return LLDISPLAY_EXTRA_NOT_SUPPORTED;
	}

	lcd_init_destination_struct(destAddr, ((LLDISPLAY_SDrawImage*)drawing)->dest_x, ((LLDISPLAY_SDrawImage*)drawing)->dest_y, ((LLDISPLAY_SImage*)dest)->width);
	lcd_init_foreground_struct(srcAddr, alphaAndColor, ((LLDISPLAY_SDrawImage*)drawing)->src_x, ((LLDISPLAY_SDrawImage*)drawing)->src_y, ((LLDISPLAY_SDrawImage*)drawing)->src_width, ((LLDISPLAY_SDrawImage*)drawing)->src_height, src_width);
	DMA2D_LayerForegroundInitStruct.format = format;
	DMA2D_LayerForegroundInitStruct.bpp = bpp;

	lcd_blend();

	return LLDISPLAY_EXTRA_DRAWING_RUNNING;
}

/* EOF -----------------------------------------------------------------------*/
