/*
 * C
 *
 * Copyright 2019 MicroEJ Corp. All rights reserved.
 * For demonstration purpose only.
 * MicroEJ Corp. PROPRIETARY. Use is subject to license terms.
 */

#ifndef DEVICE_H
#define DEVICE_H

/**
 * STM32F4 Unique device ID register (96bits)
 * @see section 52.1 of the STM32FL4 Reference Manual
 */
#define UNIQUE_DEVICE_ID_ADDRESS 	((const char*)0x1FFF7590)
#define UNIQUE_DEVICE_ID_SIZE 		(96/8)

#endif

