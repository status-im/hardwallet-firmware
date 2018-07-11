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

int flash_copy(const uint32_t* src, __IO uint32_t* dst, uint32_t size) {
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

void test_fs_initialized(void) {
  flash_unlock();
  flash_erase(FS_FIRST_PAGE, FS_PAGE_COUNT);
  flash_lock();

  TEST_CHECK(!fs_initialized());
  TEST_CHECK(!fs_init());
  TEST_CHECK(fs_initialized() == 1);
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

void test_fs_get_page(void) {
  TEST_CHECK(!fs_init());
  uint32_t* page = fs_get_page(FS_WRITE_ONCE_PAGE, 0);
  TEST_CHECK(page[0] == FS_V1_PAGE_ID(FS_WRITE_ONCE_ID, 0));
  page = fs_get_page(FS_PINLESS_LIST_PAGE, 1);
  TEST_CHECK(page[0] == FS_V1_PAGE_ID(FS_PINLESS_LIST_ID, 1));
}

void test_fs_find_free_entry(void) {
  TEST_CHECK(!fs_init());

  uint32_t* page = FS_PAGE_IDX_ADDR(FS_SETTINGS_PAGE, 0);
  TEST_CHECK(fs_find_free_entry(FS_SETTINGS_PAGE, FS_SETTINGS_COUNT, 10) == &page[2]);
  page[2] = 0;

  TEST_CHECK(fs_find_free_entry(FS_SETTINGS_PAGE, FS_SETTINGS_COUNT, 10) == &page[12]);

  for (int i = 2; i < FLASH_PAGE_SIZE/4; i++) {
    page[i] = 0;
  }

  page = FS_PAGE_IDX_ADDR(FS_SETTINGS_PAGE, 1);
  TEST_CHECK(fs_find_free_entry(FS_SETTINGS_PAGE, FS_SETTINGS_COUNT, 10) == &page[2]);

  for (int i = 2; i < FLASH_PAGE_SIZE/4; i++) {
    page[i] = 0;
  }

  page = FS_PAGE_IDX_ADDR(FS_SETTINGS_PAGE, 2);

  for (int i = 2; i < ((FLASH_PAGE_SIZE/4) - 2); i++) {
    page[i] = 0;
  }

  TEST_CHECK(fs_find_free_entry(FS_SETTINGS_PAGE, FS_SETTINGS_COUNT, 2) == &page[510]);
  TEST_CHECK(fs_find_free_entry(FS_SETTINGS_PAGE, FS_SETTINGS_COUNT, 10) == NULL);
}

void test_fs_find_last_entry(void) {
  TEST_CHECK(!fs_init());
  TEST_CHECK(fs_find_last_entry(FS_SETTINGS_PAGE, FS_SETTINGS_COUNT, 8) == NULL);

  uint32_t* page = FS_PAGE_IDX_ADDR(FS_SETTINGS_PAGE, 0);
  page[2] = 0;
  TEST_CHECK(fs_find_last_entry(FS_SETTINGS_PAGE, FS_SETTINGS_COUNT, 8) == &page[2]);

  for (int i = 2; i < FLASH_PAGE_SIZE/4; i++) {
    page[i] = 0;
  }

  TEST_CHECK(fs_find_last_entry(FS_SETTINGS_PAGE, FS_SETTINGS_COUNT, 8) == &page[498]);
  page = FS_PAGE_IDX_ADDR(FS_SETTINGS_PAGE, 1);
  page[2] = 0;
  page[10] = 0;
  TEST_CHECK(fs_find_last_entry(FS_SETTINGS_PAGE, FS_SETTINGS_COUNT, 8) == &page[10]);

  for (int i = 2; i < FLASH_PAGE_SIZE/4; i++) {
    page[i] = 0;
  }

  page = FS_PAGE_IDX_ADDR(FS_SETTINGS_PAGE, 2);

  for (int i = 2; i < FLASH_PAGE_SIZE/4; i++) {
    page[i] = 0;
  }

  TEST_CHECK(fs_find_last_entry(FS_SETTINGS_PAGE, FS_SETTINGS_COUNT, 8) == &page[498]);
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
  uint32_t* page;

  for(int i = 0; i < FS_KEY_CACHE_COUNT; i++) {
    page = FS_PAGE_IDX_ADDR(FS_KEY_CACHE_PAGE, i);
    for (int j = 2; j < FLASH_PAGE_SIZE/4; j++) {
      page[j] = 0;
    }
  }

  page = FS_PAGE_IDX_ADDR(FS_KEY_CACHE_PAGE, 0);
  TEST_CHECK(fs_cache_get_free(FS_KEY_CACHE_PAGE, FS_KEY_CACHE_COUNT, 34) == &page[2]);
  TEST_CHECK(page[0] == FS_V1_PAGE_ID(FS_KEY_CACHE_ID, 0));
  TEST_CHECK(page[1] == FS_KEY_CACHE_COUNT);

  for (int i = 2; i < FLASH_PAGE_SIZE/4; i++) {
    page[i] = 0;
  }

  page = FS_PAGE_IDX_ADDR(FS_KEY_CACHE_PAGE, 1);
  TEST_CHECK(fs_cache_get_free(FS_KEY_CACHE_PAGE, FS_KEY_CACHE_COUNT, 34) == &page[2]);
  TEST_CHECK(page[0] == FS_V1_PAGE_ID(FS_KEY_CACHE_ID, 1));
  TEST_CHECK(page[1] == (FS_KEY_CACHE_COUNT + 1));
}

void test_fs_write_entry(void) {
  TEST_CHECK(!fs_init());

  uint32_t entry[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10 };
  uint32_t* page = FS_PAGE_IDX_ADDR(FS_COUNTERS_PAGE, 0);

  TEST_CHECK(!fs_write_entry(&page[2], entry, 4));
  TEST_CHECK(!memcmp(entry, &page[2], 16));
}

void test_fs_replace_entry(void) {
  TEST_CHECK(!fs_init());

  uint32_t entry[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10 };
  uint32_t* page = FS_PAGE_IDX_ADDR(FS_COUNTERS_PAGE, 0);

  TEST_CHECK(!fs_replace_entry(FS_COUNTERS_PAGE, FS_COUNTERS_PAGE, 4, entry));
  TEST_CHECK(!memcmp(entry, &page[2], 16));

  for (int i = 0; i < FS_KEY_CACHE_COUNT; i++) {
    page = FS_PAGE_IDX_ADDR(FS_COUNTERS_PAGE, i);

    for (int j = 2; j < FLASH_PAGE_SIZE/4; j++) {
      page[j] = 0xeeeeeeee;
    }
  }

  page = FS_PAGE_IDX_ADDR(FS_COUNTERS_PAGE, 0);

  TEST_CHECK(!fs_replace_entry(FS_COUNTERS_PAGE, FS_COUNTERS_PAGE, 4, entry));
  TEST_CHECK(!memcmp(entry, &page[2], 16));
  TEST_CHECK(page[0] == FS_V1_PAGE_ID(FS_COUNTERS_ID, 0));
  TEST_CHECK(page[1] == 1);

  for (int j = 6; j < FLASH_PAGE_SIZE/4; j++) {
    TEST_CHECK(page[j] == 0xffffffff);
  }

  for (int i = 1; i < FS_COUNTERS_PAGE; i++) {
    page = FS_PAGE_IDX_ADDR(FS_COUNTERS_PAGE, i);
    TEST_CHECK(page[0] == FS_V1_PAGE_ID(FS_COUNTERS_ID, i));
    TEST_CHECK(page[1] == 1);

    for (int j = 2; j < FLASH_PAGE_SIZE/4; j++) {
      TEST_CHECK(page[j] == 0xffffffff);
    }
  }
}

void test_fs_cache_entry(void) {
  TEST_CHECK(!fs_init());

  uint32_t entry[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0X10 };
  uint32_t* page = FS_PAGE_IDX_ADDR(FS_KEY_CACHE_PAGE, 0);

  TEST_CHECK(fs_cache_entry(FS_KEY_CACHE_PAGE, FS_KEY_CACHE_COUNT, 4, entry) == &page[2]);
  TEST_CHECK(!memcmp(entry, &page[2], 16));

  for (int i = 0; i < FS_KEY_CACHE_COUNT; i++) {
    page = FS_PAGE_IDX_ADDR(FS_KEY_CACHE_PAGE, i);

    for (int j = 2; j < FLASH_PAGE_SIZE/4; j++) {
      page[j] = 0xeeeeeeee;
    }
  }

  page = FS_PAGE_IDX_ADDR(FS_KEY_CACHE_PAGE, 0);

  TEST_CHECK(fs_cache_entry(FS_KEY_CACHE_PAGE, FS_KEY_CACHE_COUNT, 4, entry) == &page[2]);
  TEST_CHECK(!memcmp(entry, &page[2], 16));

  for (int j = 6; j < FLASH_PAGE_SIZE/4; j++) {
    TEST_CHECK(page[j] == 0xffffffff);
  }

  for (int i = 1; i < FS_KEY_CACHE_COUNT; i++) {
    page = FS_PAGE_IDX_ADDR(FS_KEY_CACHE_PAGE, i);

    for (int j = 2; j < FLASH_PAGE_SIZE/4; j++) {
      TEST_CHECK(page[j] == 0xeeeeeeee);
    }
  }
}

TEST_LIST = {
   { "fs_init", test_fs_init },
   { "fs_initialized", test_fs_initialized },
   { "fs_commit", test_fs_commit },
   { "fs_get_page", test_fs_get_page },
   { "fs_find_free_entry", test_fs_find_free_entry },
   { "fs_find_last_entry", test_fs_find_last_entry },
   { "fs_swap_get_free", test_fs_swap_get_free },
   { "fs_cache_get_free", test_fs_cache_get_free },
   { "fs_write_entry", test_fs_write_entry },
   { "fs_replace_entry", test_fs_replace_entry },
   { "fs_cache_entry", test_fs_cache_entry },
   { 0 }
};
