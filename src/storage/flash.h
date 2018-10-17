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

#ifndef FLASH_H_
#define FLASH_H_

#include <stdint.h>

#ifdef NRF52840_XXAA
  #include "nrf52/flash_nrf52.h"
#else
  #error "No flash backend available"
#endif

/**
 * Unlocks the flash, allowing writing.
 */
int flash_unlock();

/**
 * Erases `page_count` pages, starting from `page`. Since this function is only called using constant values, the input parameters are not validated.
 */
int flash_erase(uint8_t page, uint8_t page_count);

/**
 * Copies `size` words from `src` to `dst`. Since this function is only called using constant values and previously checked values, the input parameters are not validated. The addresses must be aligned according
 * to the rules specified in the ST DM00083560 document.
 */
int flash_copy(const uint32_t* src, uint32_t* dst, uint32_t size);

/**
 * Locks the flash again, preventing writing.
 */
int flash_lock();

#endif /* FLASH_H_ */
