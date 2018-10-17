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

#ifndef FS_H_
#define FS_H_

#include "storage/flash.h"
#include <stdint.h>

#define FS_FLASH_PAGE_SIZE (FLASH_PAGE_SIZE/4)
#define FS_HEADER_SIZE 2
#define FS_FLASH_PAGE_DATA_SIZE (FS_FLASH_PAGE_SIZE - FS_HEADER_SIZE)

#define FS_WRITE_ONCE_ID 0x574f0000
#define FS_WRITE_ONCE_PAGE 0
#define FS_WRITE_ONCE_COUNT 1

#define FS_PIN_DATA_ID 0x504e0000
#define FS_PIN_DATA_PAGE 1
#define FS_PIN_DATA_COUNT 1

#define FS_COUNTERS_ID 0x43540000
#define FS_COUNTERS_PAGE 2
#define FS_COUNTERS_COUNT 1

#define FS_PINLESS_LIST_ID 0x4e500000
#define FS_PINLESS_LIST_PAGE 4
#define FS_PINLESS_LIST_COUNT 2

#define FS_SETTINGS_ID 0x53310000
#define FS_SETTINGS_PAGE 7
#define FS_SETTINGS_COUNT 2

#define FS_KEY_CACHE_ID 0x4b430000
#define FS_KEY_CACHE_PAGE 10
#define FS_KEY_CACHE_COUNT 16

#define FS_SWAP_PAGE (FS_PAGE_COUNT - 8)
#define FS_SWAP_COUNT 8

#define FS_PAGE_ID(p, v, i) (p | (v << 8) | i)
#define FS_V1_PAGE_ID(p, i) FS_PAGE_ID(p, 1, i)

/**
 * Erases and initialize the filesystem. Return 0 on success.
 */
int fs_init();

/**
 * Returns 1 if the file system is initialized, 0 otherwise
 */
int fs_initialized();

/**
 * Commits the changes from the swap pages to the filesystem
 */
int fs_commit();

/**
 * Gets the address of the page. This function is needed for easy stubbing in unit tests.
 */
uint32_t* fs_get_page(uint32_t base_page, int index);

/**
 * Scans the requested number of consecutive pages, starting at the given page number until empty space for a new entry is found.
 * The address of the empty entry is returned. If all pages are full, NULL is returned.
 *
 * The entry size is in words, so if you have the byte length you must divide it by 4, since we target 32-bit architectures.
 */
uint32_t* fs_find_free_entry(uint32_t page_num, int page_count, int entry_size);

/**
 * Similar to fs_find_free_entry, but returns the last non-empty entry instead of the first empty one. If there are no entries NULL is returned.
 * This function is needed to get the current value of updateable entries from the filesystem, like counters, settings, etc.
 */
uint32_t* fs_find_last_entry(uint32_t page_num, int page_count, int entry_size);

/**
 * Gets the first erased swap page. If none is erased, it erases a free page and returns its address. Returns NULL if all pages contain data.
 */
uint32_t* fs_swap_get_free();

/**
 * Similar to fs_find_free_entry, but if there is no space available it erases the oldest page and return its address.
 * The magic number and write counter of the erased page is automatically initialized.
 *
 * This function only returns NULL if there is an internal error, like being unable to erase the page.
 */
uint32_t* fs_cache_get_free(uint32_t cache_start, int page_count, int entry_size);

/**
 * Writes an entry at the given address. No check is performed on the address alignment. Return -1 if the write fails.
 */
int fs_write_entry(uint32_t* addr, const uint32_t* entry, int entry_size);

/**
 * Finds the first free entry and writes the new value. If all pages are full, they are all erased and the new value is written at the beginning
 * of the first page.
 */
int fs_replace_entry(uint32_t page_num, int page_count, int entry_size, const uint32_t *entry);

/**
 * Similar to fs_replace_entry but applicable to caches. In case the cache is full, only the oldest page is erased and the new entry is placed at
 * the beginning of the erased page. Returns a pointer to the cached entry.
 */
uint32_t* fs_cache_entry(uint32_t cache_start, int page_count, int entry_size, const uint32_t *entry);

#endif /* FS_H_ */
