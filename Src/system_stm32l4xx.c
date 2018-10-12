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

#include "stm32l4xx.h"
#include "system.h"
#include "ble.h"

#define BL_CHECK_FIRMWARE_SERVICE 0

static const void** system_bl_service_table;
static int system_reboot_scheduled;

int system_low_battery() {
  return -1;
}

void system_schedule_reboot() {
  system_reboot_scheduled = 1;
}

int system_valid_firmware(uintptr_t addr) {
  int (*check_firmware)(uintptr_t) = system_bl_service_table[BL_CHECK_FIRMWARE_SERVICE];
  return !check_firmware(addr);
}

void system_init(const void* bl_service_table[]) {
  system_bl_service_table = bl_service_table;
  system_reboot_scheduled = 0;

  /* Run at 16mhz */
  RCC->CR = RCC_CR_MSIRGSEL | RCC_CR_MSIRANGE_9 | RCC_CR_MSION;

  /* Reset CFGR */
  RCC->CFGR = 0x00000000U;

  /* Reset PLLCFGR register */
  RCC->PLLCFGR = 0x00001000U;

  /* Disable all interrupts */
  RCC->CIER = 0x00000000U;

  /* Enter run range 2 */
  PWR->CR1 = PWR_CR1_VOS_1;
}


void EXTI0_IRQHandler(void) {
  ble_isr();
}
