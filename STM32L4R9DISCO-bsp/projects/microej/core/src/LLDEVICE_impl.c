/*
 * Copyright 2013-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/* Includes ------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "device.h"
#include "stm32l4xx_hal.h"
#include "LLDEVICE_impl.h"
#include "sni.h"

uint8_t LLDEVICE_IMPL_getArchitecture(uint8_t* buffer, int32_t length) {
	if (length >= 6) {
		buffer[0]= 'S';
		buffer[1]= 'T';
		buffer[2]= 'M';
		buffer[3]= '3';
		buffer[4]= '2';
		buffer[5]= 0;
		return 1;
	} 
	else {
		return 0;
	}
}

uint32_t LLDEVICE_IMPL_getId(uint8_t* buffer, int32_t length) {
	if(length < UNIQUE_DEVICE_ID_SIZE){
		return 0;
	}
	memcpy(buffer, UNIQUE_DEVICE_ID_ADDRESS, UNIQUE_DEVICE_ID_SIZE);
	return UNIQUE_DEVICE_ID_SIZE;
}

void LLDEVICE_IMPL_reboot(void) {
	HAL_NVIC_SystemReset();
}
