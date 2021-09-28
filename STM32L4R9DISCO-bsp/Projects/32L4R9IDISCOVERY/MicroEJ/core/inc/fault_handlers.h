/*
 * C
 *
 * Copyright 2014-2019 MicroEJ Corp. All rights reserved.
 * For demonstration purpose only.
 * MicroEJ Corp. PROPRIETARY. Use is subject to license terms.
 */
#ifndef FAULT_HANDLERS_H
#define FAULT_HANDLERS_H

void HardFault_Handler(void);
void MemFault_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);

#endif // FAULT_HANDLERS_H
