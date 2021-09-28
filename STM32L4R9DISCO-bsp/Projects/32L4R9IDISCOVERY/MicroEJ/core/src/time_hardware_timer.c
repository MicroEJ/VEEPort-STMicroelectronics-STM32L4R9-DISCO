/*
 * C
 *
 * Copyright 2019 MicroEJ Corp. All rights reserved.
 * For demonstration purpose only.
 * MicroEJ Corp. PROPRIETARY. Use is subject to license terms.
 */

/* Includes ------------------------------------------------------------------*/

#include "time_hardware_timer.h"
#include "stm32l4xx_hal.h"

/* Defines -------------------------------------------------------------------*/

// TIMER definition
#define TIMx  TIM2
#define TIMx_CLK_ENABLE() __HAL_RCC_TIM2_CLK_ENABLE()
#define TIMER_CHANNEL TIM_CHANNEL_1
#define MAX_TIMER_VALUE 0xFFFFFFFF
#define TC_CLOCK 1000000	// clock frequency in Hertz

/* Globals -------------------------------------------------------------------*/

// this value is updated at each interrupt
static int64_t software_counter ;

/* TIM handle declaration */
static TIM_HandleTypeDef TimHandle;

/* Private functions ---------------------------------------------------------*/

static int64_t timer_get_counter_value(void)
{
	int64_t tc = __HAL_TIM_GET_COUNTER(&TimHandle);
	return tc & 0xffffffff ;
}

static int64_t time_hardware_timer_getTicks(void)
{
	return software_counter + timer_get_counter_value();
}

/*
 * An interrupt can occur between the reading of software_counter
 * and timer_get_counter_value(). So the value of software_counter is not
 * coherent with the value of timer_get_counter_value().
 * Compute twice the ticks value. At least one will be correct.
 */
static int64_t time_hardware_timer_getAndCheckTicks(void)
{
	volatile int64_t val1 = time_hardware_timer_getTicks();
	volatile int64_t val2 = time_hardware_timer_getTicks();

	if(val2 < val1)
	{	// second computation get back in the past: val2 is not valid for sure
		return val1;
	}
	else
	{	// first computation may be invalid (to far in the past): val2 is valid for sure
		return val2;
	}
}

static void timer_set_interrupt_value(int64_t itVal)
{
	__HAL_TIM_SET_COMPARE(&TimHandle, TIMER_CHANNEL, (uint32_t)itVal);
	__HAL_TIM_ENABLE_IT(&TimHandle, TIM_IT_CC1);
}

static int64_t timer_get_max_counter_value(void)
{
	return MAX_TIMER_VALUE ;
}

static int64_t timer_get_max_value(void)
{
	// get the max value which can be used for an interrupt
	// return the max value of the timer divided by 2
	// if the interrupt is initialize to the max value of timer, an overflow will occurs
	return timer_get_max_counter_value() >> 1; // / 2
}

/*
 * Increment the soft timer value
 */
static void timer_update_software_counter(void)
{
	int64_t currentTimerValue = timer_get_counter_value();
	software_counter += currentTimerValue ;
}

/*
 * Reconfigure interrupt value
 */
static void timer_configure_next_interrupt(void)
{
	timer_set_interrupt_value(timer_get_max_value());
}

static void timer_set_counter_value(int64_t timeVal)
{
	__HAL_TIM_SET_COUNTER(&TimHandle, (uint32_t) timeVal);
}

/* Interrupt functions -------------------------------------------------------*/

void TIM2_IRQHandler(void)
{
	__HAL_TIM_CLEAR_IT(&TimHandle, TIM_IT_CC1);

	// increment the soft timer value and reset the hardware timer to 0
	timer_update_software_counter();
	timer_set_counter_value(0) ;

	timer_configure_next_interrupt();
}

/* Public functions ----------------------------------------------------------*/

void time_hardware_timer_initialize(void)
{
	TIM_OC_InitTypeDef sConfig;
	uint32_t uwPrescalerValue = 0;

	// initialize fields
	software_counter = 0;
	TimHandle.Instance = TIMx;

	// enable timer clock
	TIMx_CLK_ENABLE();

	// get APB1 prescaler
	RCC_ClkInitTypeDef clkconfig;
	uint32_t pFLatency;
	HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);
	uint32_t uwAPB1Prescaler = clkconfig.APB1CLKDivider;

	/* Compute TIM2 clock */
	uint32_t uwTimclock = HAL_RCC_GetPCLK1Freq();
	if (uwAPB1Prescaler != RCC_HCLK_DIV1)
	{
		uwTimclock *= 2;
	}

	// Compute the prescaler value to have TIMER counter clock equal to TC_CLOCK Hz
	uwPrescalerValue = (uint32_t)((uwTimclock / TC_CLOCK) - 1);

	// Initialize TIMER peripheral as follows:
	//   - Period = TC_CLOCK - 1
	//   - Prescaler = ((SystemCoreClock / 2)/TC_CLOCK) - 1
	//   - ClockDivision = 1
	//   - Counter direction = Up
	TimHandle.Init.Period            = MAX_TIMER_VALUE;
	TimHandle.Init.Prescaler         = uwPrescalerValue;
	TimHandle.Init.ClockDivision     = 0;
	TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
	TimHandle.Init.RepetitionCounter = 0;

	if (HAL_TIM_OC_Init(&TimHandle) != HAL_OK)
	{
		while(1); // error
	}

	// configure the Output Compare channel 1
	sConfig.OCMode     	= TIM_OCMODE_ACTIVE;
	sConfig.OCPolarity	= TIM_OCPOLARITY_HIGH;
	sConfig.OCNPolarity	= TIM_OCNPOLARITY_HIGH /*not used*/;
	sConfig.Pulse 		= MAX_TIMER_VALUE;
	sConfig.OCIdleState 	= TIM_OCIDLESTATE_SET /*not used*/;
	sConfig.OCNIdleState    = TIM_OCNIDLESTATE_SET /*not used*/;

	if(HAL_TIM_OC_ConfigChannel(&TimHandle, &sConfig, TIM_CHANNEL_1) != HAL_OK)
	{
		while(1); // error
	}

	// initialize the timer value to 0
	timer_set_counter_value(0);
	// initialize the interrupt
	timer_configure_next_interrupt();

	// start timer channel 1 in Output compare mode
	if(HAL_TIM_OC_Start(&TimHandle, TIM_CHANNEL_1) != HAL_OK)
	{
		while(1); // error
	}

	HAL_NVIC_SetPriority(TIM2_IRQn, 1, 0x00);
	HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

int64_t time_hardware_timer_getCurrentTime(void)
{
	return time_hardware_timer_getAndCheckTicks()/1000 ;
}

int64_t time_hardware_timer_getTimeNanos(void)
{
	return time_hardware_timer_getAndCheckTicks()*1000 ;
}

