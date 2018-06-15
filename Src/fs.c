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

#include "fs.h"

#define HEADER(h, m, c) h[0] = m; h[1] = c;

int fs_init() {
  if (flash_unlock() || flash_erase(FS_FIRST_PAGE, FS_PAGE_COUNT)) return -1;

  uint32_t header[2];

  HEADER(header, FS_V1_PAGE_ID(FS_WRITE_ONCE_ID, 0), 0);
  if(flash_copy(header, FS_PAGE_ADDR(FS_WRITE_ONCE_PAGE, 0), 2)) return -1;

  HEADER(header, FS_V1_PAGE_ID(FS_PIN_DATA_ID, 0), 0);
  if(flash_copy(header, FS_PAGE_ADDR(FS_PIN_DATA_PAGE, 0), 2)) return -1;

  HEADER(header, FS_V1_PAGE_ID(FS_COUNTERS_ID, 0), 0);
  if(flash_copy(header, FS_PAGE_ADDR(FS_COUNTERS_PAGE, 0), 2)) return -1;
  HEADER(header, FS_V1_PAGE_ID(FS_COUNTERS_ID, 1), 0);
  if(flash_copy(header, FS_PAGE_ADDR(FS_COUNTERS_PAGE, 1), 2)) return -1;

  HEADER(header, FS_V1_PAGE_ID(FS_PINLESS_LIST_ID, 0), 0);
  if(flash_copy(header, FS_PAGE_ADDR(FS_PINLESS_LIST_PAGE, 0), 2)) return -1;
  HEADER(header, FS_V1_PAGE_ID(FS_PINLESS_LIST_ID, 1), 0);
  if(flash_copy(header, FS_PAGE_ADDR(FS_PINLESS_LIST_PAGE, 1), 2)) return -1;
  HEADER(header, FS_V1_PAGE_ID(FS_PINLESS_LIST_ID, 2), 0);
  if(flash_copy(header, FS_PAGE_ADDR(FS_PINLESS_LIST_PAGE, 2), 2)) return -1;

  HEADER(header, FS_V1_PAGE_ID(FS_SETTINGS_ID, 0), 0);
  if(flash_copy(header, FS_PAGE_ADDR(FS_SETTINGS_PAGE, 0), 2)) return -1;
  HEADER(header, FS_V1_PAGE_ID(FS_SETTINGS_ID, 1), 0);
  if(flash_copy(header, FS_PAGE_ADDR(FS_SETTINGS_PAGE, 1), 2)) return -1;
  HEADER(header, FS_V1_PAGE_ID(FS_SETTINGS_ID, 2), 0);
  if(flash_copy(header, FS_PAGE_ADDR(FS_SETTINGS_PAGE, 2), 2)) return -1;

  for (int i = 0; i < FS_KEY_CACHE_COUNT; i++) {
    HEADER(header, FS_V1_PAGE_ID(FS_KEY_CACHE_ID, i), 0);
    if(flash_copy(header, FS_PAGE_ADDR(FS_KEY_CACHE_PAGE, i), 2)) return -1;
  }

  return flash_lock();
}

int fs_commit() {
  for (int i = 0; i < FS_SWAP_COUNT; i++) {

  }
}

int32_t fs_page_find_free(uint32_t page, int page_count, int entry_size) {

}

int32_t fs_page_find_last(uint32_t page, int page_count, int entry_size) {

}

uint32_t fs_swap_get_free() {

}

uint32_t fs_cache_get_free(uint32_t cache_start, int page_count, int entry_size) {

}
