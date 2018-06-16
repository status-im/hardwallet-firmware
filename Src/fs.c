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
#include <stddef.h>

#define FS_HEADER(h, m, c) h[0] = m; h[1] = c;

#define FS_FREE -1
#define FS_ERASED -2
#define FS_UNKNOWN -3

#define FS_ZEROED_ENTRY 0x00000000
#define FS_CLEAR_ENTRY 0xffffffff

int fs_init() {
  int res = -1;

  if (flash_unlock() || flash_erase(FS_FIRST_PAGE, FS_PAGE_COUNT)) goto ret;

  uint32_t header[2];

  FS_HEADER(header, FS_V1_PAGE_ID(FS_WRITE_ONCE_ID, 0), 0);
  if(flash_copy(header, FS_PAGE_IDX_ADDR(FS_WRITE_ONCE_PAGE, 0), 2)) goto ret;

  FS_HEADER(header, FS_V1_PAGE_ID(FS_PIN_DATA_ID, 0), 0);
  if(flash_copy(header, FS_PAGE_IDX_ADDR(FS_PIN_DATA_PAGE, 0), 2)) goto ret;

  FS_HEADER(header, FS_V1_PAGE_ID(FS_COUNTERS_ID, 0), 0);
  if(flash_copy(header, FS_PAGE_IDX_ADDR(FS_COUNTERS_PAGE, 0), 2)) goto ret;
  FS_HEADER(header, FS_V1_PAGE_ID(FS_COUNTERS_ID, 1), 0);
  if(flash_copy(header, FS_PAGE_IDX_ADDR(FS_COUNTERS_PAGE, 1), 2)) goto ret;

  FS_HEADER(header, FS_V1_PAGE_ID(FS_PINLESS_LIST_ID, 0), 0);
  if(flash_copy(header, FS_PAGE_IDX_ADDR(FS_PINLESS_LIST_PAGE, 0), 2)) goto ret;
  FS_HEADER(header, FS_V1_PAGE_ID(FS_PINLESS_LIST_ID, 1), 0);
  if(flash_copy(header, FS_PAGE_IDX_ADDR(FS_PINLESS_LIST_PAGE, 1), 2)) goto ret;
  FS_HEADER(header, FS_V1_PAGE_ID(FS_PINLESS_LIST_ID, 2), 0);
  if(flash_copy(header, FS_PAGE_IDX_ADDR(FS_PINLESS_LIST_PAGE, 2), 2)) goto ret;

  FS_HEADER(header, FS_V1_PAGE_ID(FS_SETTINGS_ID, 0), 0);
  if(flash_copy(header, FS_PAGE_IDX_ADDR(FS_SETTINGS_PAGE, 0), 2)) goto ret;
  FS_HEADER(header, FS_V1_PAGE_ID(FS_SETTINGS_ID, 1), 0);
  if(flash_copy(header, FS_PAGE_IDX_ADDR(FS_SETTINGS_PAGE, 1), 2)) goto ret;
  FS_HEADER(header, FS_V1_PAGE_ID(FS_SETTINGS_ID, 2), 0);
  if(flash_copy(header, FS_PAGE_IDX_ADDR(FS_SETTINGS_PAGE, 2), 2)) goto ret;

  for (int i = 0; i < FS_KEY_CACHE_COUNT; i++) {
    FS_HEADER(header, FS_V1_PAGE_ID(FS_KEY_CACHE_ID, i), i);
    if(flash_copy(header, FS_PAGE_IDX_ADDR(FS_KEY_CACHE_PAGE, i), 2)) goto ret;
  }

  res = 0;

ret:
  flash_lock();
  return res;
}

static int _fs_page_by_magic(uint32_t magic) {
  uint32_t idx = magic & 0xff;
  magic = magic & 0xffff0000;

  int page = FS_UNKNOWN;

  switch(magic) {
  case FS_ZEROED_ENTRY:
    page = FS_FREE;
    break;
  case FS_CLEAR_ENTRY:
    page = FS_ERASED;
    break;
  case FS_WRITE_ONCE_ID:
    page = FS_WRITE_ONCE_PAGE;
    break;
  case FS_PIN_DATA_ID:
    page = FS_PIN_DATA_PAGE;
    break;
  case FS_COUNTERS_ID:
    page = FS_COUNTERS_PAGE;
    break;
  case FS_PINLESS_LIST_ID:
    page = FS_PINLESS_LIST_PAGE;
    break;
  case FS_SETTINGS_ID:
    page = FS_SETTINGS_PAGE;
    break;
  case FS_KEY_CACHE_ID:
    page = FS_KEY_CACHE_PAGE;
    break;
  }

  if (page >= 0) {
    page += idx;
  }

  return page;
}

static uint32_t _fs_actual_size(uint32_t *page) {
  // since we only allow programming in double words, we skip even-indexed words
  for(int i = ((FLASH_PAGE_SIZE / 4) - 1); i > 0; i -= 2) {
    if (page[i] != FS_CLEAR_ENTRY) {
      return i - 1; // the length is the index + 1, then we subtract 2 bytes of header
    }
  }

  return 0;
}

static int _fs_commit_page(uint32_t* page, int target_p) {
  uint32_t *target = FS_PAGE_ADDR(target_p);
  if (flash_erase(FS_ABS_PAGE(target_p), 1)) return -1;
  if (flash_copy(&page[2], &target[2], _fs_actual_size(page))) return -1;
  if (flash_copy(page, target, 2)) return -1;
  uint32_t header[2] = {0, 0};
  if (flash_copy(header, page, 2)) return -1;

  return 0;
}

int fs_commit() {
  int erase_next_free = 0;
  int res = -1;

  if(flash_unlock()) goto ret;

  for (int i = 0; i < FS_SWAP_COUNT; i++) {
    uint32_t *page = FS_PAGE_IDX_ADDR(FS_SWAP_PAGE, i);
    int target_p = _fs_page_by_magic(page[0]);

    switch(target_p) {
    case FS_FREE:
      if (erase_next_free) {
        if(flash_erase(FS_ABS_IDX_PAGE(FS_SWAP_PAGE, i), 1)) goto ret;
        erase_next_free = 0;
      }
      break;
    case FS_ERASED:
      break;
    case FS_UNKNOWN:
      if(flash_erase(FS_ABS_IDX_PAGE(FS_SWAP_PAGE, i), 1)) goto ret;
      break;
    default:
      if (_fs_commit_page(page, target_p)) goto ret;
      erase_next_free = 1;
      break;
    }
  }

  res = 0;

ret:
  flash_lock();
  return res;
}

uint32_t* fs_find_free_entry(uint32_t page_id, int page_count, int entry_size) {
  uint32_t* page = FS_PAGE_ADDR(page_id);
  int size = page_count * (FLASH_PAGE_SIZE/4);

  for (int i = 2; i < size; i += (entry_size/4)) {
    if (page[i] == FS_CLEAR_ENTRY) {
      return &page[i];
    }
  }

  return NULL;
}

uint32_t* fs_find_last_entry(uint32_t page_id, int page_count, int entry_size) {
  uint32_t* free_addr = fs_find_free_entry(page_id, page_count, entry_size);

  if (!free_addr) {
    free_addr = FS_PAGE_ADDR(page_id) + (page_count * FLASH_PAGE_SIZE);
  }

  return free_addr - entry_size;
}

uint32_t* _fs_free_swap_page(int zeroed) {
  if (flash_unlock() || flash_erase(FS_ABS_IDX_PAGE(FS_SWAP_PAGE, zeroed), 1)) {
    return NULL;
  }

  return FS_PAGE_IDX_ADDR(FS_SWAP_PAGE, zeroed);
}

uint32_t* fs_swap_get_free() {
  int zeroed = -1;

  for (int i = 0; i < FS_SWAP_COUNT; i++) {
    uint32_t *page = FS_PAGE_IDX_ADDR(FS_SWAP_PAGE, i);

    if (page[0] == FS_CLEAR_ENTRY) {
      return page;
    } else if(page[0] == FS_ZEROED_ENTRY) {
      zeroed = i;
    }
  }

  if (zeroed == -1) {
    return NULL;
  }

  return _fs_free_swap_page(zeroed);
}

uint32_t * _fs_cache_free_oldest(uint32_t cache_start, int page_count) {
  uint32_t* addr = NULL;

  int lowwc = -1;
  int highwc = 0x7fffffff;
  int page_idx = -1;

  for (int i = 0; i < page_count; i++) {
    uint32_t *page = FS_PAGE_IDX_ADDR(cache_start, i);
    if (page[1] > highwc) {
      highwc = page[1];
    } else if (page[1] < lowwc) {
      lowwc = page[1];
      addr = page;
      page_idx = 1;
    }
  }

  uint32_t header[2];
  FS_HEADER(header, addr[0], (highwc + 1));

  if(flash_unlock()) goto ret;

  if (flash_erase(FS_ABS_IDX_PAGE(cache_start, page_idx), 1) || flash_copy(header, addr, 2)) {
    addr = NULL;
    goto ret;
  }

ret:
  flash_lock();
  return addr;
}

uint32_t* fs_cache_get_free(uint32_t cache_start, int page_count, int entry_size) {
  uint32_t* free_addr = fs_find_free_entry(cache_start, page_count, entry_size);

  if (!free_addr) {
    free_addr = _fs_cache_free_oldest(cache_start, page_count);
  }

  return free_addr;
}
