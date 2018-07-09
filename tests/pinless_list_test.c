#include "acutest.h"
#include <string.h>

#define __IO volatile
#define FLASH_BASE ((uintptr_t) 0)

#include "../Src/pinless_list.c"

int fs_commit_called;
int fs_write_entry_called;
int fs_swap_get_free_called;

#define TEST_AREA_SIZE FS_FLASH_PAGE_SIZE * FS_PINLESS_LIST_COUNT

uint32_t pages[TEST_AREA_SIZE];
uint32_t swap[TEST_AREA_SIZE];

uint32_t entries[] = { 6, 0x2c, 0x01, 0x02, 0x03, 0x04, 0x05, 0xffffffff, 0xffffffff, 0xffffffff, 7, 0x2c, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0xffffffff, 0xffffffff };
uint32_t new_entry[] = { 5, 0x2c, 0x01, 0x02, 0x03, 0x04, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };

void zero_page(uint32_t* addr) {
  for(int i = 2; i < FS_FLASH_PAGE_SIZE; i++) {
    addr[i] = 0;
  }
}

void test_init() {
  fs_commit_called = 0;
  fs_write_entry_called = 0;
  fs_swap_get_free_called = 0;

  for (int i = 0; i < TEST_AREA_SIZE; i++) {
    pages[i] = 0xffffffff;
    swap[i] = 0xffffffff;
  }

  for (int i = 0; i < FS_PINLESS_LIST_COUNT; i++) {
    pages[FS_FLASH_PAGE_SIZE*i] = FS_V1_PAGE_ID(FS_PINLESS_LIST_ID, i);
    pages[FS_FLASH_PAGE_SIZE*i + 1] = 0;
  }
}

int fs_commit() {
  fs_commit_called = 1;
  memcpy(pages, swap, TEST_AREA_SIZE * 4);
  return 0;
}

uint32_t* fs_get_page(uint32_t base_page, int index) {
  TEST_CHECK(base_page == FS_PINLESS_LIST_PAGE);
  return &pages[FS_FLASH_PAGE_SIZE * index];
}

uint32_t* fs_swap_get_free() {
  return &swap[FS_FLASH_PAGE_SIZE * fs_swap_get_free_called++];
}

int fs_write_entry(uint32_t* addr, const uint32_t* entry, int entry_size) {
  fs_write_entry_called++;
  memcpy(addr, entry, (entry_size * 4));
  return 0;
}

void test_pinless_list_contains(void) {
  test_init();

  memcpy(&pages[2], entries, 80);
  TEST_CHECK(pinless_list_contains(&entries[10]) == 1);
  TEST_CHECK(pinless_list_contains(&entries[0]) == 1);
  TEST_CHECK(pinless_list_contains(&entries[2]) == 0);
  zero_page(pages);
  zero_page(&pages[FS_FLASH_PAGE_SIZE]);
  memcpy(&pages[(FS_FLASH_PAGE_SIZE * 2) + 2], entries, 80);
  TEST_CHECK(pinless_list_contains(&entries[10]) == 1);
  zero_page(&pages[FS_FLASH_PAGE_SIZE * 2]);
  TEST_CHECK(pinless_list_contains(&entries[10]) == 0);
}

void test_pinless_list_add(void) {
  test_init();

  TEST_CHECK(pinless_list_add(&entries[0]) == 1);
  TEST_CHECK(fs_write_entry_called == 1);
  TEST_CHECK(pinless_list_add(&entries[10]) == 1);
  TEST_CHECK(fs_write_entry_called == 2);
  TEST_CHECK(pinless_list_add(&entries[10]) == 0);
  TEST_CHECK(fs_write_entry_called == 2);

  for (int i = 22; i < FS_FLASH_PAGE_SIZE; i++) {
    pages[i] = 0;
  }

  for(int i = 2; i < 12; i++) {
    pages[FS_FLASH_PAGE_SIZE + i] = 0;
  }

  TEST_CHECK(pinless_list_add(new_entry) == 1);
  TEST_CHECK(fs_write_entry_called == 3);
  TEST_CHECK(!memcmp(&pages[FS_FLASH_PAGE_SIZE + 12], new_entry, 40));

  for(int i = 12; i < 22; i++) {
    pages[i] = 0;
  }

  for(int i = 22; i < FS_FLASH_PAGE_SIZE; i++) {
    pages[FS_FLASH_PAGE_SIZE + i] = 0;
  }

  zero_page(&pages[FS_FLASH_PAGE_SIZE*2]);

  TEST_CHECK(pinless_list_add(&entries[10]) == 1);
  TEST_CHECK(!memcmp(&pages[2], &entries[0], 40));
  TEST_CHECK(!memcmp(&pages[12], new_entry, 40));
  TEST_CHECK(!memcmp(&pages[22], &entries[10], 40));
  TEST_CHECK(pages[1] == 1);
  TEST_CHECK(pages[FS_FLASH_PAGE_SIZE + 1] == 1);
  TEST_CHECK(pages[FS_FLASH_PAGE_SIZE*2 + 1] == 1);

  TEST_CHECK(fs_write_entry_called > 3);
  TEST_CHECK(fs_commit_called == 1);
  TEST_CHECK(fs_swap_get_free_called == 3);

  test_init();

  for(int i = 2; i < FS_FLASH_PAGE_SIZE; i++) {
    pages[i] = 1;
    pages[FS_FLASH_PAGE_SIZE + i] = 1;
    pages[FS_FLASH_PAGE_SIZE * 2 + i] = 1;
  }

  TEST_CHECK(pinless_list_add(&entries[10]) == -2);
  TEST_CHECK(fs_write_entry_called == 0);
  TEST_CHECK(fs_commit_called == 0);
  TEST_CHECK(fs_swap_get_free_called == 0);
}

void test_pinless_list_remove(void) {
  test_init();
  memcpy(&pages[2], entries, 80);

  TEST_CHECK(pinless_list_remove(&entries[10]) == 1);
  TEST_CHECK(pages[2] == 6);
  TEST_CHECK(pages[12] == 0);
  TEST_CHECK(fs_write_entry_called == 1);

  TEST_CHECK(pinless_list_remove(&entries[10]) == 0);
  TEST_CHECK(pinless_list_remove(new_entry) == 0);
  TEST_CHECK(fs_write_entry_called == 1);
  TEST_CHECK(pinless_list_remove(&entries[0]) == 1);
  TEST_CHECK(fs_write_entry_called == 2);
  TEST_CHECK(pages[2] == 0);
}

TEST_LIST = {
   { "pinless_list_contains", test_pinless_list_contains },
   { "pinless_list_add", test_pinless_list_add },
   { "pinless_list_remove", test_pinless_list_remove },
   { 0 }
};
