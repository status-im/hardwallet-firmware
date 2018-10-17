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

/* FLASH_START=0x26000
FLASH_SIZE=0xda000*/

#ifndef FLASH_NRF52_H_
#define FLASH_NRF52_H_

#define FLASH_PAGE_SIZE 0x1000

#define FIRMWARE_PAGE_COUNT 80
#define FIRMWARE_SIZE (FIRMWARE_PAGE_COUNT * FLASH_PAGE_SIZE)

#define UPGRADE_FW_FIRST_PAGE 118
#define UPGRADE_FW_START (FLASH_PAGE_SIZE * UPGRADE_FW_FIRST_PAGE)

#define FLASH_USER_AREA_START UPGRADE_FW_START
#define FLASH_USER_AREA_SIZE (FLASH_PAGE_SIZE * 130)

#define FLASH_PAGE_ADDR(p) ((uint32_t*) (p * FLASH_PAGE_SIZE))

void _nrf52_flash_init();

#endif /* FLASH_NRF52_H_ */