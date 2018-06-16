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

#include "flash.h"
#include <stdint.h>

#define FS_START (UPGRADE_FW_START + FIRMWARE_SIZE)
#define FS_FIRST_PAGE ((FS_START - FLASH_START_BANK2) / FLASH_PAGE_SIZE)
#define FS_PAGE_COUNT (FLASH_BANK_SIZE / FLASH_PAGE_SIZE) - FS_FIRST_PAGE

#define FS_WRITE_ONCE_ID 0x574f0000
#define FS_WRITE_ONCE_PAGE 0
#define FS_WRITE_ONCE_COUNT 1

#define FS_PIN_DATA_ID 0x504e0000
#define FS_PIN_DATA_PAGE 1
#define FS_PIN_DATA_COUNT 1

#define FS_COUNTERS_ID 0x43540000
#define FS_COUNTERS_PAGE 2
#define FS_COUNTERS_COUNT 2

#define FS_PINLESS_LIST_ID 0x4e500000
#define FS_PINLESS_LIST_PAGE 4
#define FS_PINLESS_LIST_COUNT 3

#define FS_SETTINGS_ID 0x53310000
#define FS_SETTINGS_PAGE 7
#define FS_SETTINGS_COUNT 3

#define FS_KEY_CACHE_ID 0x4b430000
#define FS_KEY_CACHE_PAGE 10
#define FS_KEY_CACHE_COUNT 30

#define FS_SWAP_PAGE (FS_PAGE_COUNT - 9)
#define FS_SWAP_COUNT 8

#define FS_PAGE_ID(p, v, i) (p | (v << 8) | i)
#define FS_V1_PAGE_ID(p, i) FS_PAGE_ID(p, 1, i)

#define FS_PAGE_IDX_ADDR(p, i) ((uint32_t *)(FS_START + ((p + i) * FLASH_PAGE_SIZE)))
#define FS_ABS_IDX_PAGE(p, i) (FS_FIRST_PAGE + p + i)

#define FS_PAGE_ADDR(p) FS_PAGE_IDX_ADDR(p, 0)
#define FS_ABS_PAGE(p) FS_ABS_IDX_PAGE(p, 0)

int fs_init();
int fs_commit();

uint32_t* fs_find_free_entry(uint32_t page_id, int page_count, int entry_size);
uint32_t* fs_find_last_entry(uint32_t page_id, int page_count, int entry_size);
uint32_t* fs_swap_get_free();
uint32_t* fs_cache_get_free(uint32_t cache_start, int page_count, int entry_size);

#endif /* FS_H_ */
