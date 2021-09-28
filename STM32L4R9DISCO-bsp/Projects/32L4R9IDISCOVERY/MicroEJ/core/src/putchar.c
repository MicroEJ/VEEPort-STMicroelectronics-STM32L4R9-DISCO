/*
 * C
 *
 * Copyright 2019 MicroEJ Corp. All rights reserved.
 * For demonstration purpose only.
 * MicroEJ Corp. PROPRIETARY. Use is subject to license terms.
 */

/* Includes ------------------------------------------------------------------*/

#include <stdio.h>
#include "stm32l4xx_hal.h"

/* Defines -------------------------------------------------------------------*/

#define UART_TRANSMIT_TIMEOUT 0xFFFF

// USARTx clock resources
#define USARTx                           USART2
#define USARTx_CLK_ENABLE()              __HAL_RCC_USART2_CLK_ENABLE()
#define USARTx_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

// USARTx RX/TX Pins
#define USARTx_TX_PIN                    GPIO_PIN_2
#define USARTx_TX_GPIO_PORT              GPIOA
#define USARTx_TX_AF                     GPIO_AF7_USART2
#define USARTx_RX_PIN                    GPIO_PIN_3
#define USARTx_RX_GPIO_PORT              GPIOA
#define USARTx_RX_AF                     GPIO_AF7_USART2

/* Global --------------------------------------------------------------------*/

USART_HandleTypeDef UsartHandle;

static int putchar_initialized = 0;

/* Private API ---------------------------------------------------------------*/

static void PUTCHAR_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;

	// enable GPIO TX/RX clock
	USARTx_TX_GPIO_CLK_ENABLE();
	USARTx_RX_GPIO_CLK_ENABLE();

	// Enable USARTx clock
	USARTx_CLK_ENABLE();

	// UART TX GPIO pin configuration
	GPIO_InitStruct.Pin       = USARTx_TX_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = USARTx_TX_AF;

	HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);

	// UARTx RX GPIO pin configuration
	GPIO_InitStruct.Pin = USARTx_RX_PIN;
	GPIO_InitStruct.Alternate = USARTx_RX_AF;

	HAL_GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStruct);

	UsartHandle.Instance        = USARTx;
	UsartHandle.Init.BaudRate   = 115200;
	UsartHandle.Init.WordLength = USART_WORDLENGTH_9B;
	UsartHandle.Init.StopBits   = USART_STOPBITS_1;
	UsartHandle.Init.Parity     = USART_PARITY_NONE;
	UsartHandle.Init.Mode       = USART_MODE_TX_RX;

	if (HAL_USART_Init(&UsartHandle) != HAL_OK)
	{
		while(1); // error
	}

}

/* Public functions ----------------------------------------------------------*/

int fputc(int ch, FILE *f)
{
	if(!putchar_initialized)
	{
		PUTCHAR_init();
		putchar_initialized = 1;
	}

	HAL_USART_Transmit(&UsartHandle, (uint8_t *)&ch, 1, HAL_MAX_DELAY);

	return ch;
}
