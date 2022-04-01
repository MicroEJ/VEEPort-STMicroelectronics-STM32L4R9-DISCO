/*
 * Copyright 2012-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/*
 * This file is only useful when building MicroEJ binary platform from this BSP.
 *
 * When building a MicroEJ binary platform, the MicroEJ framework inserts a
 * call to platform_init function BEFORE calling main function. MicroEJ linker
 * also declares the platform_init.c global fields.
 *
 * When building MicroEJ Reference Implementation platform, the BSP compilation
 * and final link is made by a 3rd-party IDE. In this case the entry point
 * "platform_init" is never call. Furthermore, the platform_init.c global fields
 * are never available and that's why there are declared as weak symbol.
 */

/* Includes ------------------------------------------------------------------*/

#include <string.h>
#include "bsp_util.h"

/* Structs -------------------------------------------------------------------*/

/*
 * Table on intervals to be initialized.
 * [sourceStart, sourceEnd[ is copied to [destStart, destStart + (sourceEnd - sourceStart)[
 */
typedef struct
{
	char* sourceStart;
	char* sourceEnd;
	char* destStart;

} SRelocationDataInterval;

/*
 * Table on intervals to be zero initialized. See SZeroInitInterval
 * [start0, end0[ is zero initialized
 * ...
 * [startN, endN[ is zero initialized
 */
typedef struct
{
	char* start;
	char* end;

} SZeroInitInterval;

/* External functions --------------------------------------------------------*/

/*
 * Function called just after the tables initializations. The function implementation
 * just call the main function or equivalent.
 */
extern void platform_main(void);

/* Global --------------------------------------------------------------------*/

/*
 * First and last item blocks to be initialized.
 */
BSP_DECLARE_WEAK_FCNT const SRelocationDataInterval relocation_data_table_start;
BSP_DECLARE_WEAK_FCNT const SRelocationDataInterval relocation_data_table_end;

/*
 * First and last item blocks to be zero initialized.
 */
BSP_DECLARE_WEAK_FCNT const SZeroInitInterval zero_init_table_start;
BSP_DECLARE_WEAK_FCNT const SZeroInitInterval zero_init_table_end;

/* Private functions ---------------------------------------------------------*/

/*
 * Performs the copy of all items from start to end.
 */
static inline void initializeSRelocationDataInterval(const SRelocationDataInterval* start, const SRelocationDataInterval* end)
{
	for(const SRelocationDataInterval* item = start; item < end; item++)
	{
		memcpy(item->destStart, item->sourceStart, item->sourceEnd - item->sourceStart);
	}
}

/*
 * Zero-initializes all items from start to end
 */
static inline void initializeSZeroInitInterval(const SZeroInitInterval* start, const SZeroInitInterval* end)
{
	for(const SZeroInitInterval* item = start; item < end; item++)
	{
		memset(item->start, 0, item->end - item->start);
	}
}

/* Public functions ----------------------------------------------------------*/

/*
 * The function platform_init is called during startup after the SystemInit function.
 * We are in the following state :
 *  - Clocks and memory are initialized
 *  - Interrupts are disabled
 *  - OS is not running
 *  - No runtime C relocations are made
 * This function must perform the C relocations and give control to C main().
 * We assume platform_main() never returns or exits. If it does, behavior is undefined.
 */
void platform_init(void)
{
	initializeSRelocationDataInterval(&relocation_data_table_start, &relocation_data_table_end);
	initializeSZeroInitInterval(&zero_init_table_start, &zero_init_table_end);
	platform_main();
}
