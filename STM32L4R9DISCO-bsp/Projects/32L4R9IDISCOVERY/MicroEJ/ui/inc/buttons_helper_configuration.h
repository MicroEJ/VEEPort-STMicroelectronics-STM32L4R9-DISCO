/*
 * C
 *
 * Copyright 2019 MicroEJ Corp. All rights reserved.
 * For demonstration purpose only.
 * MicroEJ Corp. PROPRIETARY. Use is subject to license terms.
 */

#ifndef _BUTTONS_HELPER_CONFIGURATION
#define _BUTTONS_HELPER_CONFIGURATION

/* Defines -------------------------------------------------------------------*/

/*
 * Disable the buttons UP BOTTOM LEFT RIGHT (see task CC0185HG-91)
 */
// #define JOYSTICK_DISABLE_MFX

// Number of buttons the helper has to manage
// joystick (5)
#define JOYSTICK_SEL_ID         0
#ifdef JOYSTICK_DISABLE_MFX
#define NUMBER_OF_BUTTONS	1
#else
#define NUMBER_OF_BUTTONS	5
#define JOYSTICK_DOWN_ID        1
#define JOYSTICK_LEFT_ID        2
#define JOYSTICK_RIGHT_ID       3
#define JOYSTICK_UP_ID          4
#endif // JOYSTICK_DISABLE_MFX

#endif
