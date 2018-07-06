#include "acutest.h"
#include <string.h>

#define __IO volatile
#define FLASH_BASE ((uintptr_t) 0)

#include "../Src/pinless_list.c"

int fs_commit() {
  return -1;
}

uint32_t* fs_get_page(uint32_t base_page, int index) {
  return NULL;
}

uint32_t* fs_swap_get_free() {
  return NULL;
}

int fs_write_entry(uint32_t* addr, const uint32_t* entry, int entry_size) {
  return -1;
}

void test_pinless_list_contains(void) {
  TEST_CHECK(0);
}

void test_pinless_list_add(void) {
  TEST_CHECK(0);
}

void test_pinless_list_remove(void) {
  TEST_CHECK(0);
}

TEST_LIST = {
   { "pinless_list_contains", test_pinless_list_contains },
   { "pinless_list_add", test_pinless_list_add },
   { "pinless_list_remove", test_pinless_list_remove },
   { 0 }
};
