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
#include "LLDISPLAY.h"
#include "LLDISPLAY_UTILS.h"
#include "LLDISPLAY_EXTRA.h"
#include "lcd_driver.h"

/* Defines -------------------------------------------------------------------*/

/*
 * Retrieves a pixel address according pixel position
 */
#define ADDR16(buf,x,y,w) ((buf) + ((y) * (w) + (x)) * 2)

/*
 * Checks if given image targets the LCD back buffer
 */
#define IS_LCD_BUFFER(image_ID) (LLDISPLAY_UTILS_getBufferAddress(image_ID) == LLDISPLAY_getGraphicsBufferAddress())

/*
 * Returns the image buffer width and not the image area width.
 */
#define GET_BUFFER_WIDTH(image_ID) (IS_LCD_BUFFER(image_ID) ? LLDISPLAY_EXTRA_getGraphicsBufferMemoryWidth() : LLDISPLAY_UTILS_getWidth(image_ID))

/*
 * MicroEJ application natives
 */
#define MICROUI_ROTATION_rotate_circular_image_with_array Java_com_microej_microui_Rotate_drawCircularImageWithArray

/* Public functions ----------------------------------------------------------*/

int32_t MICROUI_ROTATION_rotate_circular_image_with_array(int32_t dest_image_ID, int32_t src_image_ID, int32_t* circular_area, int32_t angle)
{
	// sanity checks
	if (LLDISPLAY_UTILS_getFormat(dest_image_ID) != LLDISPLAY_EXTRA_IMAGE_RGB565 || LLDISPLAY_UTILS_getFormat(src_image_ID) != LLDISPLAY_EXTRA_IMAGE_RGB565)
	{
		// not supported format
		return -1;
	}
	if (LLDISPLAY_UTILS_getWidth(dest_image_ID) != LLDISPLAY_UTILS_getWidth(src_image_ID) || LLDISPLAY_UTILS_getHeight(dest_image_ID) != LLDISPLAY_UTILS_getHeight(src_image_ID))
	{
		// graphics contexts must have the same size
		return -1;
	}
	if (LLDISPLAY_UTILS_getWidth(src_image_ID) != LLDISPLAY_UTILS_getHeight(src_image_ID))
	{
		// image must be a square
		return -1;
	}

	// retrieve src & dest characteristics
	uint8_t* src_address = (uint8_t*)LLDISPLAY_UTILS_getBufferAddress(src_image_ID);
	uint8_t* dest_address = (uint8_t*)LLDISPLAY_UTILS_getBufferAddress(dest_image_ID);
	int32_t area_width = LLDISPLAY_UTILS_getWidth(dest_image_ID);
	int32_t area_height = LLDISPLAY_UTILS_getHeight(dest_image_ID);
	int32_t src_buffer_width = GET_BUFFER_WIDTH(src_image_ID);
	int32_t dest_buffer_width = GET_BUFFER_WIDTH(dest_image_ID);

	// get angle in radian
	float angleRad = ((float)angle * 3.1415927f) / (float)180.0f;
	float cosAngle = (float)cosf(angleRad);
	float sinAngle = (float)(-sinf(angleRad));

	// retrieve top-left coordinate on source (prevent to perform some multiplications in loops)
	int32_t xCosMul = -area_width / 2;
	int32_t yCosMul = -area_height / 2;
	float xSrcFromSrcOriginBase = (float)xCosMul * cosAngle + (float)yCosMul * sinAngle + (float)(area_width / 2);
	float ySrcFromSrcOriginBase = (float)(-xCosMul) * sinAngle + (float)yCosMul * cosAngle + (float)(area_height / 2);
	uint32_t circular_area_index = 0;

	for(int32_t y = 0; y < area_height; y++)
	{
		float xSrcFromSrcOrigin = xSrcFromSrcOriginBase;
		float ySrcFromSrcOrigin = ySrcFromSrcOriginBase;

		// part 1: nothing to draw
		xSrcFromSrcOrigin += (cosAngle * circular_area[circular_area_index]);
		ySrcFromSrcOrigin -= (sinAngle * circular_area[circular_area_index]);
		uint32_t x = circular_area[circular_area_index];
		++circular_area_index;

		// part 2: have to draw
		for(; x < circular_area[circular_area_index]; x++)
		{
			if (((int32_t)xSrcFromSrcOrigin >= 0) && ((int32_t)ySrcFromSrcOrigin >= 0) && ((int32_t)xSrcFromSrcOrigin < area_width) && ((int32_t)ySrcFromSrcOrigin < area_height))
			{
				// retrieve source pixel address
				uint16_t* src_pix_addr = (uint16_t*)ADDR16(src_address, (int32_t)xSrcFromSrcOrigin, (int32_t)ySrcFromSrcOrigin, src_buffer_width);

				// retrieve destination pixel address according LCD buffer width
				uint16_t* dest_pix_addr = (uint16_t*)ADDR16(dest_address, x, y, dest_buffer_width);

				// copy source into destination
				*dest_pix_addr = *src_pix_addr;
			}
			// else: source pixel is out of bounds: nothing to draw on destination

			xSrcFromSrcOrigin += cosAngle;
			ySrcFromSrcOrigin -= sinAngle;
		}
		++circular_area_index;

		// part 3: nothing to draw
		xSrcFromSrcOrigin += (cosAngle * (circular_area[circular_area_index] - x + 1));
		ySrcFromSrcOrigin -= (sinAngle * (circular_area[circular_area_index] - x + 1));
		x = circular_area[circular_area_index];
		++circular_area_index;

		// part 4: have to draw
		for(; x < circular_area[circular_area_index]; x++)
		{
			if (((int32_t)xSrcFromSrcOrigin >= 0) && ((int32_t)ySrcFromSrcOrigin >= 0) && ((int32_t)xSrcFromSrcOrigin < area_width) && ((int32_t)ySrcFromSrcOrigin < area_height))
			{
				// retrieve source pixel address
				uint16_t* src_pix_addr = (uint16_t*)ADDR16(src_address, (int32_t)xSrcFromSrcOrigin, (int32_t)ySrcFromSrcOrigin, src_buffer_width);

				// retrieve destination pixel address according LCD buffer width
				uint16_t* dest_pix_addr = (uint16_t*)ADDR16(dest_address, x, y, dest_buffer_width);

				// copy source into destination
				*dest_pix_addr = *src_pix_addr;
			}
			// else: source pixel is out of bounds: nothing to draw on destination

			xSrcFromSrcOrigin += cosAngle;
			ySrcFromSrcOrigin -= sinAngle;
		}
		++circular_area_index;

		// go to next line
		xSrcFromSrcOriginBase += sinAngle;
		ySrcFromSrcOriginBase += cosAngle;
	}

	if (IS_LCD_BUFFER(dest_image_ID))
	{
		// update flush limits: full screen has been updated
		LLDISPLAY_UTILS_setDrawingLimits(dest_image_ID, 0, 0, area_width - 1, area_height - 1);
	}

	return 0; // no error
}

/* EOF -----------------------------------------------------------------------*/
