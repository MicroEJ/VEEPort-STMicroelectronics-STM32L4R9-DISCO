/**
  ******************************************************************************
  * @file    stm32l4r9i_discovery_lcd.h
  * @author  MCD Application Team
  * @brief   This file contains the common defines and functions prototypes for
  *          the stm32l4r9i_discovery_lcd.c driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32L4R9I_DISCOVERY_LCD_H
#define __STM32L4R9I_DISCOVERY_LCD_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32l4r9i_discovery.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32L4R9I_DISCOVERY
  * @{
  */

/** @addtogroup STM32L4R9I_DISCOVERY_LCD STM32L4R9I_DISCOVERY LCD
  * @{
  */

/** @defgroup STM32L4R9I_DISCOVERY_LCD_Exported_Constants LCD Exported Constants
  * @{
  */

/* LTDC layer configuration structure */
#define LCD_LayerCfgTypeDef               LTDC_LayerCfgTypeDef

/* Maximum number of LTDC layers */
#define LTDC_MAX_LAYER_NUMBER             ((uint32_t) 2)

/* LTDC Background layer index */
#define LTDC_ACTIVE_LAYER_BACKGROUND      LTDC_LAYER_1

/* LTDC Foreground layer index : Not used on STM32L4R9I_DISCOVERY, only one layer used */
#define LTDC_ACTIVE_LAYER_FOREGROUND      LTDC_LAYER_2

/* Number of LTDC layers */
#define LTDC_NB_OF_LAYERS                 ((uint32_t) 2)

/* LTDC Default used layer index */
#define LTDC_DEFAULT_ACTIVE_LAYER         LTDC_ACTIVE_LAYER_BACKGROUND

/* LCD status */
#define   LCD_OK         0x00
#define   LCD_ERROR      0x01
#define   LCD_TIMEOUT    0x02

/**
  * @}
  */

/** @defgroup STM32L4R9I_DISCOVERY_LCD_Exported_Types LCD Exported Types
  * @{
  */

/* LCD_OrientationTypeDef : Possible values of Display Orientation */
typedef enum
{
  LCD_ORIENTATION_PORTRAIT  = 0x00, /*!< Portrait orientation choice of LCD screen  */
  LCD_ORIENTATION_LANDSCAPE = 0x01, /*!< Landscape orientation choice of LCD screen */
  LCD_ORIENTATION_INVALID   = 0x02  /*!< Invalid orientation choice of LCD screen   */
} LCD_OrientationTypeDef;

/**
  * @}
  */

/** @addtogroup STM32L4R9I_DISCOVERY_LCD_Exported_Functions
  * @{
  */

uint8_t  BSP_LCD_Init(void);
uint8_t  BSP_LCD_DeInit(void);

uint32_t BSP_LCD_GetXSize(void);
uint32_t BSP_LCD_GetYSize(void);

uint8_t  BSP_LCD_SetTransparency(uint32_t LayerIndex, uint8_t Transparency);
uint8_t  BSP_LCD_SetColorKeying(uint32_t LayerIndex, uint32_t RGBValue);
uint8_t  BSP_LCD_ResetColorKeying(uint32_t LayerIndex);

uint8_t  BSP_LCD_SelectLayer(uint32_t LayerIndex);
uint8_t  BSP_LCD_SetLayerVisible(uint32_t LayerIndex, FunctionalState State);

void     BSP_LCD_DisplayOff(void);
void     BSP_LCD_DisplayOn(void);

void     BSP_LCD_Refresh(void);
uint8_t  BSP_LCD_IsFrameBufferAvailable(void);

void     BSP_LCD_SetBrightness(uint8_t BrightnessValue);

/* These __weak functions can be surcharged by application code for specific application needs */
void     BSP_LCD_MspInit(void);
void     BSP_LCD_MspDeInit(void);

void     BSP_LCD_DMA2D_IRQHandler(void);
void     BSP_LCD_DSI_IRQHandler(void);
void     BSP_LCD_LTDC_IRQHandler(void);
void     BSP_LCD_LTDC_ER_IRQHandler(void);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __STM32L4R9I_DISCOVERY_LCD_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
