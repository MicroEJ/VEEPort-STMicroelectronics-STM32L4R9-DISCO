/*
 * Copyright 2019 MicroEJ Corp. All rights reserved.
 * For demonstration purpose only.
 * MicroEJ Corp. PROPRIETARY. Use is subject to license terms.
 */

/* Includes ------------------------------------------------------------------*/

#include "lcd_driver.h"
#include "stm32l4r9i_discovery_io.h"

/* Defines -------------------------------------------------------------------*/

#if LCD_BPP == 16
#define LTDC_PIXEL_FORMAT LTDC_PIXEL_FORMAT_RGB565
#elif LCD_BPP == 24
#define LTDC_PIXEL_FORMAT LTDC_PIXEL_FORMAT_RGB888
#elif LCD_BPP == 32
#define LTDC_PIXEL_FORMAT LTDC_PIXEL_FORMAT_ARGB8888
#else
#error "Invalid BPP, please set a value between 16, 24 and 32"
#endif

/* Extern fields -------------------------------------------------------------*/

/* LCD/PSRAM initialization status sharing the same power source */
extern uint32_t bsp_lcd_initialized;
extern uint32_t bsp_psram_initialized;

/* Private fields ------------------------------------------------------------*/

/* GFXMMU, LTDC and DSI handles */
static GFXMMU_HandleTypeDef hgfxmmu_discovery;
static LTDC_HandleTypeDef hltdc_discovery;
static DSI_HandleTypeDef hdsi_discovery;
static uint32_t backlight_percent;

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Initialize the BSP LCD Msp.
 * @note   Application can surcharge if needed this function implementation.
 */
static void BSP_LCD_MspInit(void)
{
	/* Enable the GFXMMU clock */
	__HAL_RCC_GFXMMU_CLK_ENABLE();

	/* Reset of GFXMMU IP */
	__HAL_RCC_GFXMMU_FORCE_RESET();
	__HAL_RCC_GFXMMU_RELEASE_RESET();

	/* Enable the LTDC clock */
	__HAL_RCC_LTDC_CLK_ENABLE();

	/* Reset of LTDC IP */
	__HAL_RCC_LTDC_FORCE_RESET();
	__HAL_RCC_LTDC_RELEASE_RESET();

	/* Enable DSI Host and wrapper clocks */
	__HAL_RCC_DSI_CLK_ENABLE();

	/* Reset the DSI Host and wrapper */
	__HAL_RCC_DSI_FORCE_RESET();
	__HAL_RCC_DSI_RELEASE_RESET();

	/* Configure the clock for the LTDC */
	/* We want DSI PHI at 500MHz */
	/* We have only one line => 500Mbps */
	/* With 24bits per pixel, equivalent PCLK is 500/24 = 20.8MHz */
	/* We will set PCLK at 15MHz */
	/* Following values are OK with MSI = 4MHz */
	/* (4*60)/(1*4*4) = 15MHz */
	RCC_PeriphCLKInitTypeDef  PeriphClkInit;
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
	PeriphClkInit.PLLSAI2.PLLSAI2Source = RCC_PLLSOURCE_MSI;
	PeriphClkInit.PLLSAI2.PLLSAI2M = 1;
	PeriphClkInit.PLLSAI2.PLLSAI2N = 60;
	PeriphClkInit.PLLSAI2.PLLSAI2R = RCC_PLLR_DIV4;
	PeriphClkInit.LtdcClockSelection = RCC_LTDCCLKSOURCE_PLLSAI2_DIV4;
	PeriphClkInit.PLLSAI2.PLLSAI2ClockOut = RCC_PLLSAI2_LTDCCLK;
	if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	{
		while(1);
	}

	/* Enable HSE used for DSI PLL */
	RCC_OscInitTypeDef RCC_OscInitStruct;
	HAL_RCC_GetOscConfig(&RCC_OscInitStruct);
	if(RCC_OscInitStruct.HSEState == RCC_HSE_OFF)
	{
		/* Workaround for long HSE startup time (set PH0 to ouput PP low) */
		GPIO_InitTypeDef  GPIO_InitStruct;
		__HAL_RCC_GPIOH_CLK_ENABLE();
		GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull      = GPIO_NOPULL;
		GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Pin       = GPIO_PIN_0;
		HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
		HAL_GPIO_WritePin(GPIOH, GPIO_PIN_0, GPIO_PIN_RESET);

		RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
		RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
		RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_NONE;
		if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
		{
			while(1);
		}
	}

	/* NVIC configuration for LTDC interrupts that are now enabled */
	HAL_NVIC_SetPriority(LTDC_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(LTDC_IRQn);
	HAL_NVIC_SetPriority(LTDC_ER_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(LTDC_ER_IRQn);

	/* NVIC configuration for DSI interrupt that is now enabled */
	HAL_NVIC_SetPriority(DSI_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(DSI_IRQn);
}

/**
 * @brief  LCD power on
 *         Power on LCD.
 */
static void LCD_PowerOn(void)
{
	/* Configure DSI_RESET and DSI_POWER_ON only if psram is not currently used */
	if(bsp_psram_initialized == 0)
	{
		BSP_IO_Init();

#if defined(USE_STM32L4R9I_DISCO_REVA) || defined(USE_STM32L4R9I_DISCO_REVB)
		/* Set DSI_POWER_ON to input floating to avoid I2C issue during input PD configuration */
		BSP_IO_ConfigPin(IO_PIN_8, IO_MODE_INPUT);

		/* Configure the GPIO connected to DSI_RESET signal */
		BSP_IO_ConfigPin(IO_PIN_10, IO_MODE_OUTPUT);

		/* Activate DSI_RESET (active low) */
		BSP_IO_WritePin(IO_PIN_10, GPIO_PIN_RESET);

		/* Configure the GPIO connected to DSI_POWER_ON signal as input pull down */
		/* to activate 3V3_LCD. VDD_LCD is also activated if VDD = 3,3V */
		BSP_IO_ConfigPin(IO_PIN_8, IO_MODE_INPUT_PD);

		/* Wait at least 1ms before enabling 1V8_LCD */
		HAL_Delay(1);

		/* Configure the GPIO connected to DSI_POWER_ON signal as output low */
		/* to activate 1V8_LCD. VDD_LCD is also activated if VDD = 1,8V */
		BSP_IO_WritePin(IO_PIN_8, GPIO_PIN_RESET);
		BSP_IO_ConfigPin(IO_PIN_8, IO_MODE_OUTPUT);
#else /* USE_STM32L4R9I_DISCO_REVA || USE_STM32L4R9I_DISCO_REVB */
		/* Configure the GPIO connected to DSI_3V3_POWERON signal as output low */
		/* to activate 3V3_LCD. VDD_LCD is also activated if VDD = 3,3V */
		BSP_IO_WritePin(IO_PIN_8, GPIO_PIN_RESET);
		BSP_IO_ConfigPin(IO_PIN_8, IO_MODE_OUTPUT);

		/* Wait at least 1ms before enabling 1V8_LCD */
		HAL_Delay(1);

		/* Configure the GPIO connected to DSI_1V8_POWERON signal as output low */
		/* to activate 1V8_LCD. VDD_LCD is also activated if VDD = 1,8V */
		BSP_IO_WritePin(AGPIO_PIN_2, GPIO_PIN_RESET);
		BSP_IO_ConfigPin(AGPIO_PIN_2, IO_MODE_OUTPUT);
#endif /* USE_STM32L4R9I_DISCO_REVA || USE_STM32L4R9I_DISCO_REVB */

		/* Wait at least 15 ms (minimum reset low width is 10ms and add margin for 1V8_LCD ramp-up) */
		HAL_Delay(15);
	}

#if defined(USE_STM32L4R9I_DISCO_REVA) || defined(USE_STM32L4R9I_DISCO_REVB)
	/* Desactivate DSI_RESET */
	BSP_IO_WritePin(IO_PIN_10, GPIO_PIN_SET);
#else /* USE_STM32L4R9I_DISCO_REVA || USE_STM32L4R9I_DISCO_REVB */
	/* Configure the GPIO connected to DSI_RESET signal */
	BSP_IO_ConfigPin(IO_PIN_10, IO_MODE_OUTPUT);

	/* Desactivate DSI_RESET */
	BSP_IO_WritePin(IO_PIN_10, GPIO_PIN_SET);
#endif /* USE_STM32L4R9I_DISCO_REVA || USE_STM32L4R9I_DISCO_REVB */

	/* Wait reset complete time (maximum time is 5ms when LCD in sleep mode and 120ms when LCD is not in sleep mode) */
	HAL_Delay(120);
}

/* Interrupt functions -------------------------------------------------------*/

/**
 * @brief  This function handles DSI global interrupt request.
 * @param  None
 * @retval None
 */
void DSI_IRQHandler(void)
{
	HAL_DSI_IRQHandler(&(hdsi_discovery));
}

/**
 * @brief  This function handles LTDC global interrupt request.
 * @param  None
 * @retval None
 */
void LTDC_IRQHandler(void)
{
	HAL_LTDC_IRQHandler(&(hltdc_discovery));
}

/**
 * @brief  This function handles LTDC error global interrupt request.
 * @param  None
 * @retval None
 */
void LTDC_ER_IRQHandler(void)
{
	HAL_LTDC_IRQHandler(&(hltdc_discovery));
}

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Initialize the DSI LCD.
 * @note   The initialization is done as below:
 *     - GFXMMU initialization
 *     - DSI PLL initialization
 *     - DSI initialization
 *     - LTDC initialization
 *     - RM67162 LCD Display IC Driver initialization
 * @retval LCD state
 */
uint8_t LCD_DRIVER_initialize(uint8_t* frame_buffer_address)
{
	LTDC_LayerCfgTypeDef    LayerCfg;
	DSI_PLLInitTypeDef      dsiPllInit;
	DSI_PHY_TimerTypeDef    PhyTimings;
	DSI_HOST_TimeoutTypeDef HostTimeouts;
	DSI_LPCmdTypeDef        LPCmd;
	DSI_CmdCfgTypeDef       CmdCfg;

	/* Power on LCD */
	LCD_PowerOn();

	/* Call first MSP Initialize
	 * This will set IP blocks LTDC, DSI and DMA2D
	 * - out of reset
	 * - clocked
	 * - NVIC IRQ related to IP blocks enabled
	 */
	BSP_LCD_MspInit();

#ifdef USE_GFXMMU

	/************************/
	/* GFXMMU CONFIGURATION */
	/************************/
	hgfxmmu_discovery.Instance = GFXMMU;
	__HAL_GFXMMU_RESET_HANDLE_STATE(&hgfxmmu_discovery);
	hgfxmmu_discovery.Init.BlocksPerLine                     = GFXMMU_192BLOCKS;
	hgfxmmu_discovery.Init.DefaultValue                      = 0x0;
	hgfxmmu_discovery.Init.Buffers.Buf0Address               = (uint32_t) frame_buffer_address;
	hgfxmmu_discovery.Init.Buffers.Buf1Address               = 0; /* NU */
	hgfxmmu_discovery.Init.Buffers.Buf2Address               = 0; /* NU */
	hgfxmmu_discovery.Init.Buffers.Buf3Address               = 0; /* NU */
	hgfxmmu_discovery.Init.Interrupts.Activation             = DISABLE;
	hgfxmmu_discovery.Init.Interrupts.UsedInterrupts         = GFXMMU_AHB_MASTER_ERROR_IT; /* NU */
	if(HAL_OK != HAL_GFXMMU_Init(&hgfxmmu_discovery))
	{
		return(LCD_ERROR);
	}

	/* Initialize LUT */
	if(HAL_OK != HAL_GFXMMU_ConfigLut(&hgfxmmu_discovery, 0, LCD_WIDTH, (uint32_t) gfxmmu_lut_config))
	{
		return(LCD_ERROR);
	}
	/* Disable non visible lines : from line 390 to 1023 */
	if(HAL_OK != HAL_GFXMMU_DisableLutLines(&hgfxmmu_discovery, LCD_WIDTH, 634))
	{
		return(LCD_ERROR);
	}
#endif

	/**********************/
	/* LTDC CONFIGURATION */
	/**********************/

	/* LTDC initialization */
	hltdc_discovery.Instance = LTDC;
	__HAL_LTDC_RESET_HANDLE_STATE(&hltdc_discovery);
	hltdc_discovery.Init.HSPolarity         = LTDC_HSPOLARITY_AL;
	hltdc_discovery.Init.VSPolarity         = LTDC_VSPOLARITY_AL;
	hltdc_discovery.Init.DEPolarity         = LTDC_DEPOLARITY_AL;
	hltdc_discovery.Init.PCPolarity         = LTDC_PCPOLARITY_IPC;
	hltdc_discovery.Init.HorizontalSync     = 0;   /* HSYNC width - 1 */
	hltdc_discovery.Init.VerticalSync       = 0;   /* VSYNC width - 1 */
	hltdc_discovery.Init.AccumulatedHBP     = 1;   /* HSYNC width + HBP - 1 */
	hltdc_discovery.Init.AccumulatedVBP     = 1;   /* VSYNC width + VBP - 1 */
	hltdc_discovery.Init.AccumulatedActiveW = LCD_WIDTH + 1; /* HSYNC width + HBP + Active width - 1 */
	hltdc_discovery.Init.AccumulatedActiveH = LCD_HEIGHT + 1; /* VSYNC width + VBP + Active height - 1 */
	hltdc_discovery.Init.TotalWidth         = LCD_WIDTH + 1; /* HSYNC width + HBP + Active width + HFP - 1 */
	hltdc_discovery.Init.TotalHeigh         = LCD_HEIGHT + 2; /* VSYNC width + VBP + Active height + VFP - 1 */
	hltdc_discovery.Init.Backcolor.Red      = 255;
	hltdc_discovery.Init.Backcolor.Green    = 255;
	hltdc_discovery.Init.Backcolor.Blue     = 0;
	hltdc_discovery.Init.Backcolor.Reserved = 0xFF;
	if(HAL_LTDC_Init(&hltdc_discovery) != HAL_OK)
	{
		return(LCD_ERROR);
	}

	/* LTDC layers configuration */
	LayerCfg.WindowX0        = 0;
	LayerCfg.WindowX1        = LCD_WIDTH;
	LayerCfg.WindowY0        = 0;
	LayerCfg.WindowY1        = LCD_HEIGHT;
	LayerCfg.PixelFormat     = LTDC_PIXEL_FORMAT;
	LayerCfg.Alpha           = 0xFF; /* NU default value */
	LayerCfg.Alpha0          = 0; /* NU default value */
	LayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA; /* NU default value */
	LayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA; /* NU default value */
#ifdef USE_GFXMMU
	LayerCfg.FBStartAdress   = GFXMMU_VIRTUAL_BUFFER0_BASE;
	LayerCfg.ImageWidth      = BSP_LCD_IMAGE_WIDTH;
#else
	LayerCfg.FBStartAdress   = (uint32_t) frame_buffer_address;
	LayerCfg.ImageWidth      = LCD_WIDTH;
#endif
	LayerCfg.ImageHeight     = LCD_HEIGHT;
	LayerCfg.Backcolor.Red   = 0; /* NU default value */
	LayerCfg.Backcolor.Green = 0; /* NU default value */
	LayerCfg.Backcolor.Blue  = 0; /* NU default value */
	LayerCfg.Backcolor.Reserved = 0xFF;
	if(HAL_LTDC_ConfigLayer(&hltdc_discovery, &LayerCfg, 0) != HAL_OK)
	{
		return(LCD_ERROR);
	}

	/*********************/
	/* DSI CONFIGURATION */
	/*********************/

	/* DSI initialization */
	hdsi_discovery.Instance = DSI;
	__HAL_DSI_RESET_HANDLE_STATE(&hdsi_discovery);
	hdsi_discovery.Init.AutomaticClockLaneControl = DSI_AUTO_CLK_LANE_CTRL_DISABLE;
	/* We have 1 data lane at 500Mbps => lane byte clock at 500/8 = 62,5 MHZ */
	/* We want TX escape clock at arround 20MHz and under 20MHz so clock division is set to 4 */
	hdsi_discovery.Init.TXEscapeCkdiv             = 4;
	hdsi_discovery.Init.NumberOfLanes             = DSI_ONE_DATA_LANE;
	/* We have HSE value at 16 Mhz and we want data lane at 500Mbps */
	dsiPllInit.PLLNDIV = 125;
	dsiPllInit.PLLIDF  = DSI_PLL_IN_DIV4;
	dsiPllInit.PLLODF  = DSI_PLL_OUT_DIV1;
	if(HAL_DSI_Init(&hdsi_discovery, &dsiPllInit) != HAL_OK)
	{
		return(LCD_ERROR);
	}

	PhyTimings.ClockLaneHS2LPTime  = 33; /* Tclk-post + Tclk-trail + Ths-exit = [(60ns + 52xUI) + (60ns) + (300ns)]/16ns */
	PhyTimings.ClockLaneLP2HSTime  = 30; /* Tlpx + (Tclk-prepare + Tclk-zero) + Tclk-pre = [150ns + 300ns + 8xUI]/16ns */
	PhyTimings.DataLaneHS2LPTime   = 11; /* Ths-trail + Ths-exit = [(60ns + 4xUI) + 100ns]/16ns */
	PhyTimings.DataLaneLP2HSTime   = 21; /* Tlpx + (Ths-prepare + Ths-zero) + Ths-sync = [150ns + (145ns + 10xUI) + 8xUI]/16ns */
	PhyTimings.DataLaneMaxReadTime = 0;
	PhyTimings.StopWaitTime        = 7;
	if(HAL_DSI_ConfigPhyTimer(&hdsi_discovery, &PhyTimings) != HAL_OK)
	{
		return(LCD_ERROR);
	}

	HostTimeouts.TimeoutCkdiv                 = 1;
	HostTimeouts.HighSpeedTransmissionTimeout = 0;
	HostTimeouts.LowPowerReceptionTimeout     = 0;
	HostTimeouts.HighSpeedReadTimeout         = 0;
	HostTimeouts.LowPowerReadTimeout          = 0;
	HostTimeouts.HighSpeedWriteTimeout        = 0;
	HostTimeouts.HighSpeedWritePrespMode      = 0;
	HostTimeouts.LowPowerWriteTimeout         = 0;
	HostTimeouts.BTATimeout                   = 0;
	if(HAL_DSI_ConfigHostTimeouts(&hdsi_discovery, &HostTimeouts) != HAL_OK)
	{
		return(LCD_ERROR);
	}

	LPCmd.LPGenShortWriteNoP  = DSI_LP_GSW0P_ENABLE;
	LPCmd.LPGenShortWriteOneP = DSI_LP_GSW1P_ENABLE;
	LPCmd.LPGenShortWriteTwoP = DSI_LP_GSW2P_ENABLE;
	LPCmd.LPGenShortReadNoP   = DSI_LP_GSR0P_ENABLE;
	LPCmd.LPGenShortReadOneP  = DSI_LP_GSR1P_ENABLE;
	LPCmd.LPGenShortReadTwoP  = DSI_LP_GSR2P_ENABLE;
	LPCmd.LPGenLongWrite      = DSI_LP_GLW_DISABLE;
	LPCmd.LPDcsShortWriteNoP  = DSI_LP_DSW0P_ENABLE;
	LPCmd.LPDcsShortWriteOneP = DSI_LP_DSW1P_ENABLE;
	LPCmd.LPDcsShortReadNoP   = DSI_LP_DSR0P_ENABLE;
	LPCmd.LPDcsLongWrite      = DSI_LP_DLW_DISABLE;
	LPCmd.LPMaxReadPacket     = DSI_LP_MRDP_DISABLE;
	LPCmd.AcknowledgeRequest  = DSI_ACKNOWLEDGE_DISABLE;
	if(HAL_DSI_ConfigCommand(&hdsi_discovery, &LPCmd) != HAL_OK)
	{
		return(LCD_ERROR);
	}

	CmdCfg.VirtualChannelID      = 0;
	CmdCfg.ColorCoding           = DSI_RGB888;
	CmdCfg.CommandSize           = LCD_WIDTH;
	CmdCfg.TearingEffectSource   = DSI_TE_DSILINK;
	CmdCfg.TearingEffectPolarity = DSI_TE_FALLING_EDGE;
	CmdCfg.HSPolarity            = DSI_HSYNC_ACTIVE_LOW;
	CmdCfg.VSPolarity            = DSI_VSYNC_ACTIVE_LOW;
	CmdCfg.DEPolarity            = DSI_DATA_ENABLE_ACTIVE_HIGH;
	CmdCfg.VSyncPol              = DSI_VSYNC_FALLING;
	CmdCfg.AutomaticRefresh      = DSI_AR_ENABLE;
	CmdCfg.TEAcknowledgeRequest  = DSI_TE_ACKNOWLEDGE_ENABLE;
	if(HAL_DSI_ConfigAdaptedCommandMode(&hdsi_discovery, &CmdCfg) != HAL_OK)
	{
		return(LCD_ERROR);
	}

	/* Disable the Tearing Effect interrupt activated by default on previous function */
	__HAL_DSI_DISABLE_IT(&hdsi_discovery, DSI_IT_TE);

	if(HAL_DSI_ConfigFlowControl(&hdsi_discovery, DSI_FLOW_CONTROL_BTA) != HAL_OK)
	{
		return(LCD_ERROR);
	}

	/* Enable DSI */
	__HAL_DSI_ENABLE(&hdsi_discovery);

	/*************************/
	/* LCD POWER ON SEQUENCE */
	/*************************/
	/* Step 1 */
	/* Go to command 2 */
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0xFE, 0x01);
	/* IC Frame rate control, set power, sw mapping, mux swithc timing command */
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x06, 0x62);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x0E, 0x80);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x0F, 0x80);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x10, 0x71);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x13, 0x81);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x14, 0x81);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x15, 0x82);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x16, 0x82);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x18, 0x88);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x19, 0x55);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x1A, 0x10);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x1C, 0x99);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x1D, 0x03);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x1E, 0x03);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x1F, 0x03);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x20, 0x03);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x25, 0x03);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x26, 0x8D);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x2A, 0x03);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x2B, 0x8D);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x36, 0x00);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x37, 0x10);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x3A, 0x00);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x3B, 0x00);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x3D, 0x20);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x3F, 0x3A);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x40, 0x30);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x41, 0x1A);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x42, 0x33);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x43, 0x22);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x44, 0x11);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x45, 0x66);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x46, 0x55);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x47, 0x44);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x4C, 0x33);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x4D, 0x22);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x4E, 0x11);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x4F, 0x66);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x50, 0x55);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x51, 0x44);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x57, 0x33);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x6B, 0x1B);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x70, 0x55);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x74, 0x0C);

	/* Go to command 3 */
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0xFE, 0x02);
	/* Set the VGMP/VGSP coltage control */
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x9B, 0x40);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x9C, 0x00);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x9D, 0x20);

	/* Go to command 4 */
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0xFE, 0x03);
	/* Set the VGMP/VGSP coltage control */
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x9B, 0x40);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x9C, 0x00);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x9D, 0x20);


	/* Go to command 5 */
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0xFE, 0x04);
	/* VSR command */
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x5D, 0x10);
	/* VSR1 timing set */
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x00, 0x8D);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x01, 0x00);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x02, 0x01);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x03, 0x01);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x04, 0x10);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x05, 0x01);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x06, 0xA7);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x07, 0x20);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x08, 0x00);
	/* VSR2 timing set */
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x09, 0xC2);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x0A, 0x00);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x0B, 0x02);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x0C, 0x01);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x0D, 0x40);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x0E, 0x06);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x0F, 0x01);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x10, 0xA7);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x11, 0x00);
	/* VSR3 timing set */
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x12, 0xC2);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x13, 0x00);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x14, 0x02);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x15, 0x01);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x16, 0x40);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x17, 0x07);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x18, 0x01);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x19, 0xA7);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x1A, 0x00);
	/* VSR4 timing set */
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x1B, 0x82);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x1C, 0x00);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x1D, 0xFF);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x1E, 0x05);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x1F, 0x60);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x20, 0x02);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x21, 0x01);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x22, 0x7C);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x23, 0x00);
	/* VSR5 timing set */
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x24, 0xC2);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x25, 0x00);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x26, 0x04);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x27, 0x02);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x28, 0x70);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x29, 0x05);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x2A, 0x74);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x2B, 0x8D);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x2D, 0x00);
	/* VSR6 timing set */
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x2F, 0xC2);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x30, 0x00);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x31, 0x04);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x32, 0x02);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x33, 0x70);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x34, 0x07);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x35, 0x74);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x36, 0x8D);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x37, 0x00);
	/* VSR marping command */
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x5E, 0x20);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x5F, 0x31);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x60, 0x54);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x61, 0x76);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x62, 0x98);

	/* Go to command 6 */
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0xFE, 0x05);
	/* Set the ELVSS voltage */
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x05, 0x17);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x2A, 0x04);
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x91, 0x00);

	/* Go back in standard commands */
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0xFE, 0x00);

	/* Set tear off */
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, DSI_SET_TEAR_OFF, 0x0);

	/* Set DSI mode to internal timing added vs ORIGINAL for Command mode */
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0xC2, 0x0);

	/* Set memory address MODIFIED vs ORIGINAL */
	uint8_t InitParam1[4]= {0x00, 0x04, 0x01, 0x89}; // MODIF OFe: adjusted w/ real image
	HAL_DSI_LongWrite(&hdsi_discovery, 0, DSI_DCS_LONG_PKT_WRITE, 4, DSI_SET_COLUMN_ADDRESS, InitParam1);
	uint8_t InitParam2[4]= {0x00, 0x00, 0x01, 0x85};
	HAL_DSI_LongWrite(&hdsi_discovery, 0, DSI_DCS_LONG_PKT_WRITE, 4, DSI_SET_PAGE_ADDRESS, InitParam2);

	/* Sleep out */
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P0, DSI_EXIT_SLEEP_MODE, 0x0);

	HAL_Delay(120);

	/* Set display on */
	if(HAL_DSI_ShortWrite(&hdsi_discovery,
			0,
			DSI_DCS_SHORT_PKT_WRITE_P0,
			DSI_SET_DISPLAY_ON,
			0x0) != HAL_OK)
	{
		return(LCD_ERROR);
	}


	/* Enable DSI Wrapper */
	__HAL_DSI_WRAPPER_ENABLE(&hdsi_discovery);

	bsp_lcd_initialized = 1;

	return(LCD_OK);
}

/**
 * @brief  Refresh the display.
 */
void LCD_DRIVER_refresh(void)
{
	/* Set tear on */
	HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, DSI_SET_TEAR_ON, 0x0);
}

/**
 * @brief  Set the brightness value
 * @param  BrightnessValue: [0% Min (black), 100% Max]
 */
void LCD_DRIVER_set_backlight(uint32_t percent)
{
	backlight_percent = percent;

	/* Send Display on DCS command to display */
	HAL_DSI_ShortWrite(&hdsi_discovery,
			0,
			DSI_DCS_SHORT_PKT_WRITE_P1,
			0x51, (uint16_t)(percent * 255)/100);
}

uint32_t LCD_DRIVER_get_backlight(void)
{
	return backlight_percent;
}

