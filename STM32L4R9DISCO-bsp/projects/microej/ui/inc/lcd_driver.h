/*
 * Copyright 2019-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#ifndef __LCD_DRIVER_H
#define __LCD_DRIVER_H

/*
 * XXX
 */

/*
 * Update too "display.properties" file in MicroEJ platform configuration project
 * and rebuild the platform before using the new BPP definition.
 *
 * Linker file should be updated according BPP: a smaller frame buffer is used for
 * 16 and 24 BPP.
 */
#define LCD_BPP 16

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>

#ifdef USE_GFXMMU
#if LCD_BPP == 16
#define BSP_LCD_COLOR_DEPTH_16_BPP
#elif LCD_BPP == 24
#define BSP_LCD_COLOR_DEPTH_24_BPP
#endif
#include "stm32l4r9i_discovery_gfxmmu_lut.h"
#endif

/* Defines -------------------------------------------------------------------*/

#define LCD_WIDTH 390
#define LCD_HEIGHT 390

#ifndef USE_GFXMMU
#define BSP_LCD_IMAGE_WIDTH LCD_WIDTH
#endif

#define LCD_OK 0x00
#define LCD_ERROR 0x01
#define LCD_TIMEOUT 0x02

/* API -----------------------------------------------------------------------*/

uint8_t LCD_DRIVER_initialize(uint8_t* frame_buffer_address);

/*
 * Set backlight given a percentage between 0 and 100% (inclusive)
 */
void LCD_DRIVER_set_backlight(uint32_t percent);

/*
 * Get backlight percentage
 */
uint32_t LCD_DRIVER_get_backlight(void);

/*
 * Ask a DSI refresh
 */
void LCD_DRIVER_refresh(void);

/* EOF -----------------------------------------------------------------------*/

#endif // __LCD_DRIVER_H
