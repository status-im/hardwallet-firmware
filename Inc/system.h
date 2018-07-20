/**
 * This file is part of the Status project, https://status.im/
 *
 * Copyright (c) 2018 Status Research & Development GmbH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <stdint.h>

/**
 * Returns 1 if the system detects low battery, 0 otherwise.
 */
int system_low_battery();

/**
 * Schedules a reboot. This will happen after processing all incoming commands and sending out responses.
 */
void system_schedule_reboot();

/**
 * Returns 1 if the firmware at the given address is valid, 0 otherwise. This functions invokes the bootloader to perform the actual check.
 */
int system_valid_firmware(uintptr_t addr);

/**
 * Called by the reset handler, initializes the clock and peripherals as well as storing the bootloader service table.
 */
void system_init(const void* bl_service_table[]);

#endif /* SYSTEM_H_ */
