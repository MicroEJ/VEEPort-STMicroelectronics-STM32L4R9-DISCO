/*
 * C
 *
 * Copyright 2019 MicroEJ Corp. All rights reserved.
 * For demonstration purpose only.
 * MicroEJ Corp. PROPRIETARY. Use is subject to license terms.
 */

#include "qb_impl.h"

/**
 * When qualification bundle is used (see qb_impl.h), the functions of this file are used to replace
 * qualification bundle UI weak functions.
 */

#ifdef TEST_QUALIFICATION_BUNDLE

#include "x_ui_config.h"
#include "lcd_driver.h"

uint32_t UI_CONFIG_getBPP(void)
{
	return LCD_BPP;
}

#endif // TEST_QUALIFICATION_BUNDLE
