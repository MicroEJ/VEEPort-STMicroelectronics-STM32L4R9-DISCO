/*
 * C
 *
 * Copyright 2019 MicroEJ Corp. All rights reserved.
 * For demonstration purpose only.
 * MicroEJ Corp. PROPRIETARY. Use is subject to license terms.
 */

/*
 * This header files groups several APIs to notify to a or several events handlers an
 * event has occured on an input driver (buttons, touch etc.).
 *
 * The idea is to disconnect the management of the inputs drivers (how to initialize an
 * input driver, how to get/read an input event, how to clear it, etc.) and how to
 * send these inputs events to the MicroUI application using the MicroUI EventGenerators.
 *
 * The MicroUI framework is using an internal buffer to store the inputs events. The event
 * will not be added if the internal events buffer is full. In this case the input driver
 * has to adapt itself in order to not send a future invalid event. For instance, if a
 * PRESSED event is not sent, the input driver has not to send a REPEAT or RELEASE event.
 * So it may have a distinction between the real input state and the "software" input
 * state.
 */
#ifndef _EVENT_GENERATOR
#define _EVENT_GENERATOR

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>

/* API -----------------------------------------------------------------------*/

/*
 * Notify to the event generator a button has been pressed.
 * @param buttonId the button ID, between 0 and 255
 * @return {@link LLINPUT_OK} if all events have been added, {@link LLINPUT_NOK} otherwise
 */
int32_t EVENT_GENERATOR_button_pressed(int32_t buttonId);

/*
 * Notify to the event generator a button has been repeated.
 * @param buttonId the button ID, between 0 and 255
 * @return {@link LLINPUT_OK} if all events have been added, {@link LLINPUT_NOK} otherwise
 */
int32_t EVENT_GENERATOR_button_repeated(int32_t buttonId);

/*
 * Notify to the event generator a button has been released.
 * @param buttonId the button ID, between 0 and 255
 * @return {@link LLINPUT_OK} if all events have been added, {@link LLINPUT_NOK} otherwise
 */
int32_t EVENT_GENERATOR_button_released(int32_t buttonId);

/*
 * Notify to the event generator a touch has been pressed.
 * @param x the pointer X coordinate
 * @param y the pointer Y coordinate
 * @return {@link LLINPUT_OK} if all events have been added, {@link LLINPUT_NOK} otherwise
 */
int32_t EVENT_GENERATOR_touch_pressed(int32_t x, int32_t y);

/*
 * Notify to the event generator a touch has moved.
 * @param x the pointer X coordinate
 * @param y the pointer Y coordinate
 * @return {@link LLINPUT_OK} if all events have been added, {@link LLINPUT_NOK} otherwise
 */
int32_t EVENT_GENERATOR_touch_moved(int32_t x, int32_t y);

/*
 * Notify to the event generator a touch has been released.
 * @return {@link LLINPUT_OK} if all events have been added, {@link LLINPUT_NOK} otherwise
 */
int32_t EVENT_GENERATOR_touch_released(void);

#endif
