/*
 * C
 *
 * Copyright 2015-2019 MicroEJ Corp. All rights reserved.
 * For demonstration purpose only.
 * MicroEJ Corp. PROPRIETARY. Use is subject to license terms.
 */
 
/* Includes ------------------------------------------------------------------*/

#include "FreeRTOS.h"
#include "task.h"
#include "interrupts.h"
#include "stm32l4xx.h"

/* Public functions ----------------------------------------------------------*/

void OS_SUPPORT_disable_context_switching()
{
  if (interrupt_is_in() == MICROEJ_TRUE)
  {
    //taskENTER_CRITICAL_FROM_ISR();
    __get_BASEPRI(); portDISABLE_INTERRUPTS();    
  }
  else
  {
    taskENTER_CRITICAL();
  }
	
}

void OS_SUPPORT_enable_context_switching()
{
  if (interrupt_is_in() == MICROEJ_TRUE)
  {
    //taskEXIT_CRITICAL_FROM_ISR(0);
    __set_BASEPRI( 0 );
  }
  else
  {
    taskEXIT_CRITICAL();
  }	
}
