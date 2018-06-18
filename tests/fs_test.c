#include "acutest.h"
#include <string.h>
#include <stdint.h>

#define __IO volatile

static uint32_t _flash[0x80000/2];
static uint32_t* _flash_bank2 = &_flash[0x80000/4];
static int flash_locked = 1;

#define FLASH_BASE ((uintptr_t) _flash)

#include "../Src/fs.c"

const static int fs_start_idx = ((FLASH_BANK_SIZE + FIRMWARE_SIZE) / 4);

int flash_unlock() {
  TEST_CHECK(flash_locked);
  flash_locked = 0;
  return 0;
}

int flash_erase(uint8_t page, uint8_t page_count) {
  TEST_CHECK(!flash_locked);

  uint32_t idx = (page * FLASH_PAGE_SIZE) / 4;
  uint32_t last_idx = idx + ((page_count * FLASH_PAGE_SIZE) / 4);

  while(idx < last_idx) {
    _flash_bank2[idx++] = 0xffffffff;
  }

  return 0;
}

int flash_copy(uint32_t* src, __IO uint32_t* dst, uint32_t size) {
  TEST_CHECK(!(flash_locked || (((uintptr_t) dst) % 4) || (size % 2)));

  for(int i = 0; i < size; i += 2) {
    TEST_CHECK(dst[i] == 0xffffffff || src[i] == 0x00000000);
    TEST_CHECK(dst[i + 1] == 0xffffffff || src[i + 1] == 0x00000000);

    dst[i] = src[i];
    dst[i + 1] = src[i + 1];
  }

  return 0;
}

int flash_lock() {
  TEST_CHECK(!flash_locked);
  flash_locked = 1;
  return 0;
}

void _check_page(uint32_t page_id, uint32_t write_count, int *i) {
  TEST_CHECK(_flash[(*i)++] == page_id);
  TEST_CHECK(_flash[(*i)++] == write_count);

  for(int j = 0; j < 510; j++) {
    TEST_CHECK(_flash[(*i)++] == 0xffffffff);
  }
}

void test_fs_init(void) {
  memset(_flash, 0, 0x80000 * 2);
  TEST_CHECK(!fs_init());
  TEST_CHECK(flash_locked);

  int i = 0;

  while(i < fs_start_idx) {
    TEST_CHECK(_flash[i++] == 0);
  }

  _check_page(FS_V1_PAGE_ID(FS_WRITE_ONCE_ID, 0), 0, &i);
  _check_page(FS_V1_PAGE_ID(FS_PIN_DATA_ID, 0), 0, &i);
  _check_page(FS_V1_PAGE_ID(FS_COUNTERS_ID, 0), 0, &i);
  _check_page(FS_V1_PAGE_ID(FS_COUNTERS_ID, 1), 0, &i);
  _check_page(FS_V1_PAGE_ID(FS_PINLESS_LIST_ID, 0), 0, &i);
  _check_page(FS_V1_PAGE_ID(FS_PINLESS_LIST_ID, 1), 0, &i);
  _check_page(FS_V1_PAGE_ID(FS_PINLESS_LIST_ID, 2), 0, &i);
  _check_page(FS_V1_PAGE_ID(FS_SETTINGS_ID, 0), 0, &i);
  _check_page(FS_V1_PAGE_ID(FS_SETTINGS_ID, 1), 0, &i);
  _check_page(FS_V1_PAGE_ID(FS_SETTINGS_ID, 2), 0, &i);

  for (int j = 0; j < 30; j++) {
    _check_page(FS_V1_PAGE_ID(FS_KEY_CACHE_ID, j), j, &i);
  }

  for(int j = 0; j < 90; j++) {
    _check_page(0xffffffff, 0xffffffff, &i);
  }
}

#define _fs_swap_header(page, swap) swap[0] = page[0]; swap[1] = (page[1] + 1)

void test_fs_commit(void) {
  TEST_CHECK(!fs_init());

  uint32_t* target0 = FS_PAGE_IDX_ADDR(FS_PINLESS_LIST_PAGE, 0);
  uint32_t* target1 = FS_PAGE_IDX_ADDR(FS_PINLESS_LIST_PAGE, 1);
  uint32_t* target2 = FS_PAGE_IDX_ADDR(FS_PINLESS_LIST_PAGE, 2);

  uint32_t* swap0 = FS_PAGE_IDX_ADDR(FS_SWAP_PAGE, 0);
  memset(swap0, 0x00, FLASH_PAGE_SIZE);
  _fs_swap_header(target0, swap0);

  uint32_t* swap1 = FS_PAGE_IDX_ADDR(FS_SWAP_PAGE, 1);
  memset(swap1, 0x00, (FLASH_PAGE_SIZE - 40));
  _fs_swap_header(target1, swap1);

  uint32_t* swap2 = FS_PAGE_IDX_ADDR(FS_SWAP_PAGE, 2);
  swap2[0] = target2[0];
  swap2[1] = target2[1];
  swap2[2] = 0;
  swap2[3] = 0;

  uint32_t* swap3 = FS_PAGE_IDX_ADDR(FS_SWAP_PAGE, 3);
  swap3[0] = 0;

  TEST_CHECK(!fs_commit());
  TEST_CHECK(flash_locked);

  TEST_CHECK(target0[0] == FS_V1_PAGE_ID(FS_PINLESS_LIST_ID, 0));
  TEST_CHECK(target0[1] == 1);
  for (int i = 2; i < 512; i++) {
    TEST_CHECK(target0[i] == 0x00);
  }

  TEST_CHECK(target1[0] == FS_V1_PAGE_ID(FS_PINLESS_LIST_ID, 1));
  TEST_CHECK(target1[1] == 1);
  for (int i = 2; i < 512; i++) {
    TEST_CHECK(target1[i] == (i < 502 ? 0x00 : 0xffffffff));
  }

  TEST_CHECK(target2[0] == FS_V1_PAGE_ID(FS_PINLESS_LIST_ID, 2));
  TEST_CHECK(target2[1] == 0);
  TEST_CHECK(target2[2] == 0xffffffff);

  TEST_CHECK(swap0[0] == 0x00);
  TEST_CHECK(swap1[0] == 0x00);
  TEST_CHECK(swap2[0] == 0x00);
  TEST_CHECK(swap3[0] == 0xffffffff);
}

void test_fs_find_free_entry(void) {
  TEST_CHECK(!fs_init());

  TEST_CHECK(0);
}

void test_fs_find_last_entry(void) {
  TEST_CHECK(!fs_init());

  TEST_CHECK(0);
}

void test_fs_swap_get_free(void) {
  TEST_CHECK(!fs_init());
  uint32_t* page = FS_PAGE_IDX_ADDR(FS_SWAP_PAGE, 0);
  TEST_CHECK(fs_swap_get_free() == page);
  memset(page, 0xaa, FLASH_PAGE_SIZE);
  TEST_CHECK(fs_swap_get_free() == FS_PAGE_IDX_ADDR(FS_SWAP_PAGE, 1));
  memset(page, 0xaa, FLASH_PAGE_SIZE*8);
  TEST_CHECK(fs_swap_get_free() == NULL);

  page = FS_PAGE_IDX_ADDR(FS_SWAP_PAGE, 3);
  page[0] = 0;

  TEST_CHECK(fs_swap_get_free() == page);

  for (int i = 0; i < FLASH_PAGE_SIZE/4; i++) {
    TEST_CHECK(page[i++] == 0xffffffff);
  }
}

void test_fs_cache_get_free(void) {
  TEST_CHECK(!fs_init());

  TEST_CHECK(0);
}

TEST_LIST = {
   { "fs_init", test_fs_init },
   { "fs_commit", test_fs_commit },
   { "fs_find_free_entry", test_fs_find_free_entry },
   { "fs_find_last_entry", test_fs_find_last_entry },
   { "fs_swap_get_free", test_fs_swap_get_free },
   { "fs_cache_get_free", test_fs_cache_get_free },
   { 0 }
};
