#include "acutest.h"
#include <string.h>

#define __IO volatile
#define FLASH_BASE ((uintptr_t) 0)

#include "../Src/fw.c"

int system_schedule_reboot_called = 0;
int flash_erase_called = 0;
int flash_copy_called = 0;

int test_system_valid_firmware = 0;

void system_schedule_reboot() {
  system_schedule_reboot_called = 1;
}

int system_valid_firmware(uintptr_t addr) {
  return test_system_valid_firmware;
}

int flash_erase(uint8_t page, uint8_t page_count) {
  if (flash_erase_called == -1) {
    flash_erase_called = 1;
    TEST_CHECK(page == UPGRADE_FW_FIRST_PAGE);
    TEST_CHECK(page_count == FIRMWARE_PAGE_COUNT);
  } else {
    TEST_CHECK(page == flash_erase_called++);
    TEST_CHECK(page_count == 1);
  }

  return 0;
}

int flash_copy(const uint32_t* src, __IO uint32_t* dst, uint32_t size) {
  flash_copy_called++;
  return 0;
}

void test_fw_load(void) {
  TEST_CHECK(fw_load(0, 2048, NULL) == ERR_OK);
  TEST_CHECK(flash_erase_called == 1);
  TEST_CHECK(flash_copy_called == 1);
  TEST_CHECK(fw_load(0, 2047, NULL) == ERR_INVALID_DATA);
  TEST_CHECK(flash_erase_called == 1);
  TEST_CHECK(flash_copy_called == 1);
  TEST_CHECK(fw_load(0, 2056, NULL) == ERR_INVALID_DATA);
  TEST_CHECK(flash_erase_called == 1);
  TEST_CHECK(flash_copy_called == 1);
  TEST_CHECK(fw_load(1, 2040, NULL) == ERR_OK);
  TEST_CHECK(flash_erase_called == 2);
  TEST_CHECK(flash_copy_called == 2);
}

void test_fw_upgrade(void) {
  flash_erase_called = -1;
  TEST_CHECK(fw_upgrade() == ERR_INVALID_FW);
  TEST_CHECK(flash_erase_called == 1);
  TEST_CHECK(!system_schedule_reboot_called);

  flash_erase_called = -1;
  test_system_valid_firmware = 1;
  TEST_CHECK(fw_upgrade() == ERR_OK);
  TEST_CHECK(flash_erase_called == -1);
  TEST_CHECK(system_schedule_reboot_called);
}

TEST_LIST = {
   { "fw_load", test_fw_load },
   { "fw_upgrade", test_fw_upgrade },
   { 0 }
};
