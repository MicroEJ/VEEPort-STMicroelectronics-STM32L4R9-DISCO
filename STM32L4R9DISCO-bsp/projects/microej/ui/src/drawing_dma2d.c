/*
 * C
 *
 * Copyright 2019-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
 * @file
 * @brief Use STM32 DMA2D (ChromART) for MicroEJ ui_drawing.h implementation.
 * @author MicroEJ Developer Team
 * @version 2.1.0
 * @date 20 July 2022
 */

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/

#include "drawing_dma2d.h"
#include "ui_drawing.h"
#include "ui_drawing_soft.h"
#include "bsp_util.h"

/* Defines and Macros --------------------------------------------------------*/

/*
 * @brief Defines the DMA2D format according DRAWING_DMA2D_BPP.
 */
#if DRAWING_DMA2D_BPP == 16
#define DRAWING_DMA2D_FORMAT DMA2D_RGB565
#elif DRAWING_DMA2D_BPP == 24
#define DRAWING_DMA2D_FORMAT DMA2D_RGB888
#elif DRAWING_DMA2D_BPP == 32
#define DRAWING_DMA2D_FORMAT DMA2D_ARGB8888
#else
#error "Define 'DRAWING_DMA2D_BPP' is required (16, 24 or 32)"
#endif

/* Structs and Typedefs ------------------------------------------------------*/

/*
 * @brief Function used to notify the graphical engine about the end of DMA2D work.
 * Available values are LLUI_DISPLAY_notifyAsynchronousDrawingEnd() and LLUI_DISPLAY_flushDone().
 */
typedef void (*t_drawing_notification)(bool under_isr);

/*
 * @brief Working data when blending an image with the DMA2D
 */
typedef struct {
	uint8_t* src_address; // address of the image to draw
	uint8_t* dest_address; // address of the destination
	uint32_t src_stride; // source image' stride
	uint32_t dest_stride; // destination' stride
	jchar dest_width; // destination's width
	jchar dest_height; // destination's height
	jint x_src; // source image's region X coordinate
	jint y_src; // source image's region Y coordinate
	jint width; // region's width
	jint height; // region's height
	jint x_dest; // destination X coordinate
	jint y_dest; // destination X coordinate
	jint alpha; // opacity to apply
	uint32_t src_dma2d_format; // source image's format in DMA2D format
	uint32_t src_bpp; // source image's bpp
} DRAWING_DMA2D_blending_t;

/* Private fields ------------------------------------------------------------*/

/*
 * @brief STM32 HAL DMA2D declaration.
 */
static DMA2D_HandleTypeDef g_hdma2d;

/*
 * @brief Next notification to call after DMA2D work.
 */
static t_drawing_notification g_callback_notification;

/*
 * @brief This boolean is set to true before launching a DMA2D action and back to
 * false during DMA2D interrupt. It allows to know if the DMA2D is running or not.
 */
static bool g_dma2d_running;

/*
 * @brief Binary semaphore used to lock a DMA2D user when DMA2D is already running.
 */
static void* g_dma2d_semaphore;

/* Private functions ---------------------------------------------------------*/

/*
 * @brief Ensures DMA2D previous work is done before returning.
 */
static inline void _drawing_dma2d_wait(void) {
	while(g_dma2d_running) {
		LLUI_DISPLAY_IMPL_binarySemaphoreTake(g_dma2d_semaphore);
	}
}

/*
 * @brief Notify the DMA2D has finished previous job.
 */
static inline void _drawing_dma2d_done(void) {
	g_dma2d_running = false;
	LLUI_DISPLAY_IMPL_binarySemaphoreGive(g_dma2d_semaphore, true);
}

/*
 * @brief Adjusts the given address to target the given point.
 */
static inline uint8_t* _drawing_dma2d_adjust_address(uint8_t* address, uint32_t x, uint32_t y, uint32_t stride, uint32_t bpp) {
	return address + ((((y * stride) + x) * bpp) / (uint32_t)8);
}

/*
 * @brief For A4 and A8 formats, alpha contains both the global alpha + wanted color.
 */
static inline void _drawing_dma2d_configure_alpha_image_data(MICROUI_GraphicsContext* gc, jint* alphaAndColor) {
	// for A4 and A8 formats, alphaAndColor is both the global alpha + wanted color
	*(alphaAndColor) <<= 24;
	*(alphaAndColor) |= (gc->foreground_color & 0xffffff);
}

/*
 * @brief Cleans the cache.
 *
 * Before each DMA_2D transfer, the data cache must be cleaned because the
 * graphics memory is in the memory which is defined "cache enabled" in the MPU
 * configuration.
 *
 * This feature is only required on STM32 CPUs that hold a cache.
 */
static inline void _cleanDCache(void) {
#if defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
	SCB_CleanDCache();
#endif
}

/*
 * @brief Tells is the image to draw in the graphics context is compatible with the DMA2D.
 *
 * The image is compatible with the DMA2D when its format is supported by the DMA2D.
 * Note for the A4 format: the odd left and right bands are drawn in sofware.
 *
 * @param[in] gc: the destination
 * @param[in] image: the source
 * @param[in] x_src: the source's region X coordinate
 * @param[in] y_src: the source's region Y coordinate
 * @param[in] width: the width of the region to draw
 * @param[in] height: the width of the region to draw
 * @param[in] x_dest: the destination X coordinate
 * @param[in] y_dest: the destination Y coordinate
 * @param[in] alpha: a point on the opacity to apply.
 * @param[out] dma2d_blending_data: the data to configure the DMA2D registers
 *
 * @return true if the source is compatible with the DMA2D
 */
static bool _drawing_dma2d_is_image_compatible_with_dma2d(MICROUI_GraphicsContext* gc, MICROUI_Image* image, jint x_src, jint y_src, jint width, jint height, jint x_dest, jint y_dest, jint alpha, DRAWING_DMA2D_blending_t* dma2d_blending_data){

	bool is_dma2d_compatible = true;
	jint data_width = width;
	jint data_x_src = x_src;
	jint data_x_dest = x_dest;

	switch(image->format) {
	case MICROUI_IMAGE_FORMAT_RGB565:
		dma2d_blending_data->src_dma2d_format = CM_RGB565;
		dma2d_blending_data->src_bpp = 16;
		break;
	case MICROUI_IMAGE_FORMAT_ARGB8888:
		dma2d_blending_data->src_dma2d_format = CM_ARGB8888;
		dma2d_blending_data->src_bpp = 32;
		break;
	case MICROUI_IMAGE_FORMAT_RGB888:
		dma2d_blending_data->src_dma2d_format = CM_RGB888;
		dma2d_blending_data->src_bpp = 24;
		break;
	case MICROUI_IMAGE_FORMAT_ARGB1555:
		dma2d_blending_data->src_dma2d_format = CM_ARGB1555;
		dma2d_blending_data->src_bpp = 16;
		break;
	case MICROUI_IMAGE_FORMAT_ARGB4444:
		dma2d_blending_data->src_dma2d_format = CM_ARGB4444;
		dma2d_blending_data->src_bpp = 16;
		break;
	case MICROUI_IMAGE_FORMAT_A4: {
		// check DMA2D limitations: first and last vertical lines addresses must be aligned on 8 bits

		jint xAlign = data_x_src & 1;
		jint wAlign = data_width & 1;

		if (xAlign == 0) {
			if (wAlign != 0) {
				// hard cannot draw last vertical line
				--data_width;
				UI_DRAWING_SOFT_drawImage(gc, image, data_x_src + data_width, y_src, 1, height, data_x_dest + data_width, y_dest, alpha);
			}
			// else: easy case: first and last vertical lines are aligned on 8 bits
		}
		else {
			if (wAlign == 0) {
				// worst case: hard cannot draw first and last vertical lines

				// first line
				UI_DRAWING_SOFT_drawImage(gc, image, data_x_src, y_src, 1, height, data_x_dest, y_dest, alpha);

				// last line
				--data_width;
				UI_DRAWING_SOFT_drawImage(gc, image, data_x_src + data_width, y_src, 1, height, data_x_dest + data_width, y_dest, alpha);

				++data_x_src;
				++data_x_dest;
				--data_width;
			}
			else {
				// cannot draw first vertical line
				UI_DRAWING_SOFT_drawImage(gc, image, data_x_src, y_src, 1, height, data_x_dest, y_dest, alpha);
				++data_x_src;
				++data_x_dest;
				--data_width;
			}
		}

		_drawing_dma2d_configure_alpha_image_data(gc, &alpha);
		dma2d_blending_data->src_dma2d_format = CM_A4;
		dma2d_blending_data->src_bpp = 4;
		break;
	}
	case MICROUI_IMAGE_FORMAT_A8:
		_drawing_dma2d_configure_alpha_image_data(gc, &alpha);
		dma2d_blending_data->src_dma2d_format = CM_A8;
		dma2d_blending_data->src_bpp = 8;
		break;
	default:
		// unsupported image format
		is_dma2d_compatible = false;
		break;
	}

	if (is_dma2d_compatible){
		MICROUI_Image* dest = &gc->image;
		dma2d_blending_data->src_address = LLUI_DISPLAY_getBufferAddress(image);
		dma2d_blending_data->dest_address = LLUI_DISPLAY_getBufferAddress(dest);
		dma2d_blending_data->src_stride = LLUI_DISPLAY_getStrideInPixels(image);
		dma2d_blending_data->dest_stride = LLUI_DISPLAY_getStrideInPixels(dest);
		dma2d_blending_data->dest_width = dest->width;
		dma2d_blending_data->dest_height = dest->height;
		dma2d_blending_data->x_src = data_x_src;
		dma2d_blending_data->y_src = y_src;
		dma2d_blending_data->width = data_width;
		dma2d_blending_data->height = height;
		dma2d_blending_data->x_dest = data_x_dest;
		dma2d_blending_data->y_dest = y_dest;
		dma2d_blending_data->alpha = alpha;
	}

	return is_dma2d_compatible;
}

/*
 * @brief Configures the DMA2D to draw an image.
 *
 * This function ensures that the DMA2D is free before configuring the DMA2D.
 *
 * @param[in] dma2d_blending_data the blending configuration
 */
static void _drawing_dma2d_blending_configure(DRAWING_DMA2D_blending_t* dma2d_blending_data) {

	_drawing_dma2d_wait();

	// de-init DMA2D
	HAL_DMA2D_DeInit(&g_hdma2d);

	// configure background
	g_hdma2d.LayerCfg[0].InputOffset = dma2d_blending_data->dest_stride - dma2d_blending_data->width;
	g_hdma2d.LayerCfg[0].InputColorMode = DRAWING_DMA2D_FORMAT;
	g_hdma2d.LayerCfg[0].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	g_hdma2d.LayerCfg[0].InputAlpha = 255;
	HAL_DMA2D_ConfigLayer(&g_hdma2d, 0);

	// configure foreground
	HAL_DMA2D_DisableCLUT(&g_hdma2d, 1);
	g_hdma2d.LayerCfg[1].InputOffset = dma2d_blending_data->src_stride - dma2d_blending_data->width;
	g_hdma2d.LayerCfg[1].InputColorMode = dma2d_blending_data->src_dma2d_format;
	g_hdma2d.LayerCfg[1].AlphaMode = DMA2D_COMBINE_ALPHA;
	g_hdma2d.LayerCfg[1].InputAlpha = dma2d_blending_data->alpha;
	HAL_DMA2D_ConfigLayer(&g_hdma2d, 1);

	// configure DMA2D
	g_hdma2d.Init.Mode = DMA2D_M2M_BLEND;
	g_hdma2d.Init.OutputOffset = dma2d_blending_data->dest_stride - dma2d_blending_data->width;
	HAL_DMA2D_Init(&g_hdma2d) ;

	// configure environment (useless if false is returned)
	g_callback_notification = &LLUI_DISPLAY_notifyAsynchronousDrawingEnd;
}

/*
 * @brief Starts the DMA2D drawing previously configured thanks _drawing_dma2d_blending_configure().
 *
 * @param[in] dma2d_blending_data the blending configuration
 */
static inline void _drawing_dma2d_blending_start(DRAWING_DMA2D_blending_t* dma2d_blending_data){
	uint8_t* srcAddr = _drawing_dma2d_adjust_address(dma2d_blending_data->src_address, dma2d_blending_data->x_src, dma2d_blending_data->y_src, dma2d_blending_data->src_stride, dma2d_blending_data->src_bpp);
	uint8_t* destAddr = _drawing_dma2d_adjust_address(dma2d_blending_data->dest_address, dma2d_blending_data->x_dest, dma2d_blending_data->y_dest, dma2d_blending_data->dest_stride, DRAWING_DMA2D_BPP);
	g_dma2d_running = true;

	// cppcheck-suppress [misra-c2012-11.4] allow cast address in u32 (see HAL_DMA2D_BlendingStart_IT())
	HAL_DMA2D_BlendingStart_IT(&g_hdma2d, (uint32_t)srcAddr, (uint32_t)destAddr, (uint32_t)destAddr, dma2d_blending_data->width, dma2d_blending_data->height);
}

/*
 * @brief Draws a region of an image at another position.
 *
 * This function draws block per block to prevent the overlapping in X.
 *
 * @param[in] dma2d_blending_data the blending configuration
 */
static void _drawing_dma2d_overlap_horizontal(DRAWING_DMA2D_blending_t* dma2d_blending_data) {

	// retrieve band's width
	jint width = dma2d_blending_data->width;
	dma2d_blending_data->width = dma2d_blending_data->x_dest - dma2d_blending_data->x_src;
	_drawing_dma2d_blending_configure(dma2d_blending_data);

	// go to x + band width
	dma2d_blending_data->x_src += width;
	dma2d_blending_data->x_dest = dma2d_blending_data->x_src + dma2d_blending_data->width;

	while (width > 0) {

		// adjust band's width
		if (width < dma2d_blending_data->width){
			dma2d_blending_data->width = width;
			_drawing_dma2d_blending_configure(dma2d_blending_data);
		}

		// adjust src & dest positions
		dma2d_blending_data->x_src -= dma2d_blending_data->width;
		dma2d_blending_data->x_dest -= dma2d_blending_data->width;

		_drawing_dma2d_wait();
		_drawing_dma2d_blending_start(dma2d_blending_data);

		width -= dma2d_blending_data->width;
	}
}

/*
 * @brief Draws a region of an image at another position.
 *
 * This function draws block per block to prevent the overlapping in Y.
 *
 * @param[in] dma2d_blending_data the blending configuration
 */
static void _drawing_dma2d_overlap_vertical(DRAWING_DMA2D_blending_t* dma2d_blending_data) {

	// retrieve band's height
	jint height = dma2d_blending_data->height;
	dma2d_blending_data->height = dma2d_blending_data->y_dest - dma2d_blending_data->y_src;
	_drawing_dma2d_blending_configure(dma2d_blending_data);

	// go to x + band height
	dma2d_blending_data->y_src += height;
	dma2d_blending_data->y_dest = dma2d_blending_data->y_src + dma2d_blending_data->height;

	while (height > 0) {

		// adjust band's height
		if (height < dma2d_blending_data->height){
			dma2d_blending_data->height = height;
			// no need to configure again the DMA2D
		}

		// adjust src & dest positions
		dma2d_blending_data->y_src -= dma2d_blending_data->height;
		dma2d_blending_data->y_dest -= dma2d_blending_data->height;

		_drawing_dma2d_wait();
		_drawing_dma2d_blending_start(dma2d_blending_data);

		height -= dma2d_blending_data->height;
	}
}

/*
 * @brief Draws a region of an image using the DMA2D.
 *
 * @param[in] dma2d_blending_data the blending configuration
 */
static void _drawing_dma2d(DRAWING_DMA2D_blending_t* dma2d_blending_data){

	_cleanDCache();

	if (dma2d_blending_data->src_address == dma2d_blending_data->dest_address) {
		// source and destination target same buffer: have to check the overlapping

		if ((dma2d_blending_data->y_dest == dma2d_blending_data->y_src) && (dma2d_blending_data->x_dest > dma2d_blending_data->x_src) && (dma2d_blending_data->x_dest < (dma2d_blending_data->x_src + dma2d_blending_data->width))){
			_drawing_dma2d_overlap_horizontal(dma2d_blending_data);
		}
		else if ((dma2d_blending_data->y_dest > dma2d_blending_data->y_src) && (dma2d_blending_data->y_dest < (dma2d_blending_data->y_src + dma2d_blending_data->height))){
			_drawing_dma2d_overlap_vertical(dma2d_blending_data);
		}
		else {
			_drawing_dma2d_blending_configure(dma2d_blending_data);
			_drawing_dma2d_blending_start(dma2d_blending_data);
		}
	}
	else {
		_drawing_dma2d_blending_configure(dma2d_blending_data);
		_drawing_dma2d_blending_start(dma2d_blending_data);
	}

	LLUI_DISPLAY_setDrawingLimits(dma2d_blending_data->x_dest, dma2d_blending_data->y_dest, dma2d_blending_data->x_dest + dma2d_blending_data->width - 1, dma2d_blending_data->y_dest + dma2d_blending_data->height - 1);
}


#if DRAWING_DMA2D_BPP == 16

/**
 * Copies 16-bit elements of an array into an other array.
 * The elements are copied from the first one to the last one ("left to right").
 */
static void memcpy_16_ltr(uint16_t* destination, const uint16_t* source, uint32_t num) {
	for (int i = 0; i < (int)num; ++i) {
		destination[i] = source[i];
	}
}

/**
 * Copies 16-bit elements of an array into an other array.
 * The elements are copied from the last one to the first one ("right to left").
 */
static void memcpy_16_rtl(uint16_t* destination, const uint16_t* source, uint32_t num) {
	for (int i = (int)num-1; i >= 0; --i) {
		destination[i] = source[i];
	}
}

/**
 * Copies consecutive pixels of a buffer from a position to an other position.
 * The pixels are copied in the given order ("right to left" or "left to right").
 */
static void _drawing_soft_copy_pixels(uint8_t* src8, uint32_t src_stride, uint8_t* dst8, uint32_t dst_stride, uint32_t src_x, uint32_t src_y,
		uint32_t width, uint32_t dst_x, uint32_t dst_y, bool rtl) {

	// cppcheck-suppress [misra-c2012-11.3] cast 8 to 16 is ensured by the multiplication with stride
	uint16_t* dst = ((uint16_t*) dst8) + ((dst_y * dst_stride) + dst_x);
	// cppcheck-suppress [misra-c2012-11.3] cast 8 to 16 is ensured by the multiplication with stride
	uint16_t* src = ((uint16_t*) src8) + ((src_y * src_stride) + src_x);

	if (rtl) {
		memcpy_16_rtl(dst, src, width);
	} else {
		memcpy_16_ltr(dst, src, width);
	}
}

/**
 * Copies a region of an image to an other image with same format. Manages the overlapping when source == destination.
 */
static void _drawing_soft_copy_region(MICROUI_Image* dest, MICROUI_Image* source, jint x_src, jint y_src, jint width, jint height, jint x_dest, jint y_dest) {

	uint8_t* src = LLUI_DISPLAY_getBufferAddress(source);
	uint8_t* dst = LLUI_DISPLAY_getBufferAddress(dest);
	uint32_t src_stride = LLUI_DISPLAY_getStrideInPixels(source);
	uint32_t dst_stride = LLUI_DISPLAY_getStrideInPixels(dest);

	if (y_dest <= y_src) {
		bool rtl = ((y_dest == y_src) && (x_dest > x_src)); // true when copying to the same rows but to the right
		for (int dy = 0; dy < height; dy++) { // top to bottom
			_drawing_soft_copy_pixels(src, src_stride, dst, dst_stride, x_src, y_src+dy, width, x_dest, y_dest+dy, rtl);
		}
	} else {
		for (int dy = height-1; dy >= 0; dy--) { // bottom to top
			_drawing_soft_copy_pixels(src, src_stride, dst, dst_stride, x_src, y_src+dy, width, x_dest, y_dest+dy, false);
		}
	}

	LLUI_DISPLAY_setDrawingLimits(x_dest, y_dest, x_dest + width - 1, y_dest + height - 1);
}

#endif // DRAWING_DMA2D_BPP

/*
 * @brief Tells if given image blending is useful to be performed or not.
 *
 * @param[in] dest the destination
 * @param[in] source the image to draw
 * @param[in] x_src: the source's region X coordinate
 * @param[in] y_src: the source's region Y coordinate
 * @param[in] width: the width of the region to draw
 * @param[in] height: the width of the region to draw
 * @param[in] x_dest: the destination X coordinate
 * @param[in] y_dest: the destination Y coordinate
 * @param[in] alpha the opacity to apply
 */
static inline bool _is_useless_drawing(MICROUI_Image* dest, MICROUI_Image* source, jint x_src, jint y_src, jint x_dest, jint y_dest, jint alpha) {
	return (dest == source) && (0xff == alpha) && (x_src == x_dest) && (y_src == y_dest);
}

/* Interrupt functions -------------------------------------------------------*/

void DRAWING_DMA2D_IRQHandler(void) {
	// notify STM32 HAL
	HAL_DMA2D_IRQHandler(&g_hdma2d);

	// notify DMA2D users
	_drawing_dma2d_done();

	// notify graphical engine
	g_callback_notification(true);
}

/* Public functions ----------------------------------------------------------*/

void DRAWING_DMA2D_initialize(void* binary_semaphore_handle) {
	// configure globals
	g_dma2d_running = false;
	g_dma2d_semaphore = binary_semaphore_handle;

	// configure DMA2D IRQ handler
	HAL_NVIC_SetPriority(DMA2D_IRQn, 5, 3);
	HAL_NVIC_EnableIRQ(DMA2D_IRQn);

	// configure DMA2D
	g_hdma2d.Init.ColorMode = DRAWING_DMA2D_FORMAT;
	g_hdma2d.Instance = DMA2D;
}

void DRAWING_DMA2D_configure_memcpy(uint8_t* srcAddr, uint8_t* destAddr, uint32_t xmin, uint32_t ymin, uint32_t xmax, uint32_t ymax, uint32_t stride, DRAWING_DMA2D_memcpy* memcpy_data) {
	_drawing_dma2d_wait();

	uint32_t width = (xmax - xmin + (uint32_t)1);
	uint32_t height = (ymax - ymin + (uint32_t)1);

	// de-init DMA2D
	HAL_DMA2D_DeInit(&g_hdma2d);

	// configure foreground
	g_hdma2d.LayerCfg[1].InputOffset = stride - width;
	g_hdma2d.LayerCfg[1].InputColorMode = DRAWING_DMA2D_FORMAT;
	HAL_DMA2D_ConfigLayer(&g_hdma2d, 1);

	// configure DMA2D
	g_hdma2d.Init.Mode  = DMA2D_M2M;
	g_hdma2d.Init.OutputOffset = stride - width;
	HAL_DMA2D_Init(&g_hdma2d) ;

	// configure given structure (to give later to DRAWING_DMA2D_start_memcpy())
	memcpy_data->src_address = _drawing_dma2d_adjust_address(srcAddr, xmin, ymin, stride, DRAWING_DMA2D_BPP);
	memcpy_data->dest_address = _drawing_dma2d_adjust_address(destAddr, xmin, ymin, stride, DRAWING_DMA2D_BPP);
	memcpy_data->width = width;
	memcpy_data->height = height;

	// configure environment
	g_callback_notification = &LLUI_DISPLAY_flushDone;
	g_dma2d_running = true;
}

void DRAWING_DMA2D_start_memcpy(DRAWING_DMA2D_memcpy* memcpy_data) {
	_cleanDCache();
	HAL_DMA2D_Start_IT(
			&g_hdma2d,
			(uint32_t)memcpy_data->src_address,
			(uint32_t)memcpy_data->dest_address,
			memcpy_data->width,
			memcpy_data->height
	);
}

/* ui_drawing.h functions --------------------------------------------*/

DRAWING_Status UI_DRAWING_fillRectangle(MICROUI_GraphicsContext* gc, jint x1, jint y1, jint x2, jint y2) {

	DRAWING_Status ret;

	if (LLUI_DISPLAY_isClipEnabled(gc) && !LLUI_DISPLAY_clipRectangle(gc, &x1, &y1, &x2, &y2)) {
		// drawing is out of clip: nothing to do
		ret = DRAWING_DONE;
	}
	else {
		// clip disabled (means drawing is fully in clip) or drawing fits the clip


		_drawing_dma2d_wait();

		LLUI_DISPLAY_setDrawingLimits(x1, y1, x2, y2);

		uint32_t rectangle_width = x2 - x1 + 1;
		uint32_t rectangle_height = y2 - y1 + 1;
		uint32_t stride = LLUI_DISPLAY_getStrideInPixels(&gc->image);

		// de-init DMA2D
		HAL_DMA2D_DeInit(&g_hdma2d);

		// configure DMA2D
		g_hdma2d.Init.Mode = DMA2D_R2M;
		g_hdma2d.Init.OutputOffset = stride - rectangle_width;
		HAL_DMA2D_Init(&g_hdma2d);

		// configure environment
		g_callback_notification = &LLUI_DISPLAY_notifyAsynchronousDrawingEnd;
		g_dma2d_running = true;

		// start DMA2D
		_cleanDCache();
		uint8_t* destination_address = _drawing_dma2d_adjust_address(LLUI_DISPLAY_getBufferAddress(&gc->image), x1, y1, stride, DRAWING_DMA2D_BPP);
		// cppcheck-suppress [misra-c2012-11.4] allow cast address in u32 (see HAL_DMA2D_Start_IT())
		HAL_DMA2D_Start_IT(&g_hdma2d, gc->foreground_color, (uint32_t)destination_address, rectangle_width, rectangle_height);

		ret = DRAWING_RUNNING;
	}

	return ret;
}

DRAWING_Status UI_DRAWING_drawImage(MICROUI_GraphicsContext* gc, MICROUI_Image* image, jint x_src, jint y_src, jint width, jint height, jint x_dest, jint y_dest, jint alpha) {

	DRAWING_Status ret;
	if (!_is_useless_drawing(&gc->image, image, x_src, y_src, x_dest, y_dest, alpha)){

#if DRAWING_DMA2D_BPP == 16
		if ((0xff == alpha) && (gc->image.format == image->format)) {
			// source and destination have the same format and no opacity is applied
			// -> software algorithm is faster than DMA2D
			_drawing_soft_copy_region(&gc->image, image, x_src, y_src, width, height, x_dest, y_dest);
			ret = DRAWING_DONE;
		}
		else{
#endif // DRAWING_DMA2D_BPP

			DRAWING_DMA2D_blending_t dma2d_blending_data;

			if (_drawing_dma2d_is_image_compatible_with_dma2d(gc, image, x_src, y_src, width, height, x_dest, y_dest, alpha, &dma2d_blending_data)){
				_drawing_dma2d(&dma2d_blending_data);
				ret = DRAWING_RUNNING;
			}
			else {
				UI_DRAWING_SOFT_drawImage(gc, image, x_src, y_src, width, height, x_dest, y_dest, alpha);
				ret = DRAWING_DONE;
			}
#if DRAWING_DMA2D_BPP == 16
		}
#endif // DRAWING_DMA2D_BPP
	}
	else {
		// nothing to do
		ret = DRAWING_DONE;
	}

	return ret;
}

/* EOF -----------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif


