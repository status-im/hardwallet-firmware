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

#include "storage/pinless_list.h"
#include "storage/fs.h"
#include "util/handy.h"

const static uint32_t EMPTY_PATH[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };



static uint32_t* _pinless_list_compact(int first_compactable_page) {
  int swap_page = first_compactable_page;
  int swap_i = FS_HEADER_SIZE;
  uint32_t* swap = fs_swap_get_free();
  uint32_t swap_header[FS_HEADER_SIZE];

  uint32_t* res = NULL;

  for (int i = first_compactable_page; i < FS_PINLESS_LIST_COUNT; i++) {
    uint32_t* page = fs_get_page(FS_PINLESS_LIST_PAGE, i);

    for (int i = FS_HEADER_SIZE; i < FS_FLASH_PAGE_SIZE; i += WALLET_PATH_LEN) {
      if (page[i] != 0x00000000) {
        if (fs_write_entry(&swap[swap_i], &page[i], WALLET_PATH_LEN)) return NULL;
        swap_i += WALLET_PATH_LEN;

        if (swap_i >= FS_FLASH_PAGE_SIZE) {
          memcpy(swap_header, fs_get_page(FS_PINLESS_LIST_PAGE, swap_page), (FS_HEADER_SIZE * 4));
          swap_header[1]++;
          if (fs_write_entry(swap, swap_header, FS_HEADER_SIZE)) return NULL;

          swap_page++;
          swap_i = FS_HEADER_SIZE;
          swap = swap_page < FS_PINLESS_LIST_COUNT ? fs_swap_get_free() : NULL;
        }
      }
    }
  }

  res = &fs_get_page(FS_PINLESS_LIST_PAGE, swap_page)[swap_i];

  while(swap_page < FS_PINLESS_LIST_COUNT) {
    memcpy(swap_header, fs_get_page(FS_PINLESS_LIST_PAGE, swap_page), (FS_HEADER_SIZE * 4));
    swap_header[1]++;
    if (fs_write_entry(swap, swap_header, FS_HEADER_SIZE)) return NULL;
    swap = (swap_page < FS_PINLESS_LIST_COUNT - 1) ? fs_swap_get_free() : NULL;
    swap_page++;
  }

  return fs_commit() == -1 ? NULL : res;
}

static int _pinless_search(const uint32_t path[WALLET_PATH_LEN], uint32_t** out_path, uint32_t** out_free, int *first_compactable_page) {
  int found = 0;

  for (int i = 0; i < FS_PINLESS_LIST_COUNT; i++) {
    uint32_t* page = fs_get_page(FS_PINLESS_LIST_PAGE, i);

    for (int j = FS_HEADER_SIZE; j < FS_FLASH_PAGE_SIZE; j += WALLET_PATH_LEN) {
      if (page[j] == 0xffffffff) {
        if (out_free) {
          *out_free = &page[j];
        }

        return found;
      } else if (page[j] == 0x00000000) {
        if (first_compactable_page && (*first_compactable_page == -1)) {
          *first_compactable_page = i;
        }
      } else if (!found && !wordcmp(path, &page[j], WALLET_PATH_LEN)) {
        found = 1;

        if (out_path) {
          *out_path = &page[j];
        }

        if (!out_free) {
          return found;
        }
      }
    }
  }

  return found;
}

int pinless_list_contains(const uint32_t path[WALLET_PATH_LEN]) {
  return _pinless_search(path, NULL, NULL, NULL);
}

int pinless_list_add(const uint32_t path[WALLET_PATH_LEN]) {
  uint32_t* free_addr = NULL;
  int first_compactable_page = -1;

  if (!_pinless_search(path, NULL, &free_addr, &first_compactable_page)) {
    if (!free_addr) {
      free_addr = first_compactable_page == -1 ? NULL : _pinless_list_compact(first_compactable_page);
      if (!free_addr) return -2;
    }
  } else {
    return 0;
  }

  return fs_write_entry(free_addr, path, WALLET_PATH_LEN) == 0 ? 1 : -1;
}

int pinless_list_remove(const uint32_t path[WALLET_PATH_LEN]) {
  uint32_t* entry = NULL;

  if (_pinless_search(path, &entry, NULL, NULL)) {
    return fs_write_entry(entry, EMPTY_PATH, WALLET_PATH_LEN) == 0 ? 1 : -1;
  } else {
    return 0;
  }
}
