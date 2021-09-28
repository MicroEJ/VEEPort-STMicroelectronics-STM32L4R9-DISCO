/*
 * C
 *
 * Copyright 2019 MicroEJ Corp. All rights reserved.
 * For demonstration purpose only.
 * MicroEJ Corp. PROPRIETARY. Use is subject to license terms.
 */

/* Prevent recursive inclusion */
#ifndef __X_QB_IMPL_H
#define __X_QB_IMPL_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Set this define to run qualification bundle tests before launching MicroEJ application.
 * This define requires the qualification bundle tests are included in BSP project as
 * described in M0018_PlatformQualificationTools project
 */
//#define TEST_QUALIFICATION_BUNDLE

/*
 * Entry point of qualification bundle tests.
 */
void QB_IMPL_run_tests(void);

#ifdef __cplusplus
}
#endif

#endif
