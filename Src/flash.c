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

#include "flash.h"

// Waits for the BSY flag in the FLASH_SR registry to be cleared
static inline void _flash_wait() {
  while(READ_BIT(FLASH->SR, FLASH_SR_BSY));
}

// Clears all flash programming error flags
static inline void _flash_clear_errors() {
  SET_BIT(FLASH->ECCR, FLASH_ECCR_ECCD);
  WRITE_REG(FLASH->SR, (FLASH_SR_OPERR | FLASH_SR_PROGERR | FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_SIZERR | FLASH_SR_PGSERR | FLASH_SR_MISERR  | FLASH_SR_FASTERR | FLASH_SR_RDERR | FLASH_SR_OPTVERR));
}

int flash_unlock() {
  FLASH->KEYR = FLASH_KEYR1;
  FLASH->KEYR = FLASH_KEYR2;

  return READ_BIT(FLASH->CR, FLASH_CR_LOCK);
}

// Erases a single page of flash memory
static int _flash_erase_page(uint8_t pg) {
  SET_BIT(FLASH->CR, FLASH_CR_BKER);

  MODIFY_REG(FLASH->CR, FLASH_CR_PNB, (pg << POSITION_VAL(FLASH_CR_PNB)));
  SET_BIT(FLASH->CR, FLASH_CR_PER);
  SET_BIT(FLASH->CR, FLASH_CR_STRT);

  _flash_wait();
  // Undocumented: the PER bit must be cleared manually after the page is erased
  CLEAR_BIT(FLASH->CR, FLASH_CR_PER);
  return READ_BIT(FLASH->SR, FLASH_SR_WRPERR);
}

int flash_erase(uint8_t page, uint8_t page_count) {
  _flash_wait();
  _flash_clear_errors();

  page_count = page + page_count;

  while(page < page_count) {
    if(_flash_erase_page(page)) {
      return -1;
    }

    page++;
  }

  return 0;
}

int flash_copy(const uint32_t *src, __IO uint32_t *dst, uint32_t size) {
  _flash_wait();
  _flash_clear_errors();
  int ret = 0;

  SET_BIT(FLASH->CR, FLASH_CR_PG);

  for(uint32_t i = 0; i < size; i += 2) {
    dst[i] = src[i];
    dst[i + 1] = src[i + 1];
    _flash_wait();

    if (READ_BIT(FLASH->SR, FLASH_SR_PROGERR)) {
      ret = -1;
      break;
    }
  }

  CLEAR_BIT(FLASH->CR, FLASH_CR_PG);
  return ret;
}

int flash_lock() {
  SET_BIT(FLASH->CR, FLASH_CR_LOCK);
  return 0;
}
