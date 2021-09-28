/*
 * C
 *
 * Copyright 2019 MicroEJ Corp. All rights reserved.
 * For demonstration purpose only.
 * MicroEJ Corp. PROPRIETARY. Use is subject to license terms.
 */

#include "qb_impl.h"

#ifdef TEST_QUALIFICATION_BUNDLE

#include "t_core_main.h"
#include "t_ui_main.h"

void QB_IMPL_run_tests(void)
{
	T_CORE_main();
	T_UI_main();
}

#else

void QB_IMPL_run_tests(void)
{
	// nothing to test
}

#endif // TEST_QUALIFICATION_BUNDLE
