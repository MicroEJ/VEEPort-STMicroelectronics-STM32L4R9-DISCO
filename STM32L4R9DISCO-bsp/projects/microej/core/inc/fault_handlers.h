/*
 * Copyright 2014-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */
#ifndef FAULT_HANDLERS_H
#define FAULT_HANDLERS_H

void HardFault_Handler(void);
void MemFault_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);

#endif // FAULT_HANDLERS_H
