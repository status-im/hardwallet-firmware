#include "acutest.h"
#include <string.h>

#define __IO volatile
#define FLASH_BASE ((uintptr_t) 0)

#include "../Src/fw.c"

int system_schedule_reboot_called;

void system_schedule_reboot() {
  system_schedule_reboot_called = 1;
}

int system_valid_firmware(uintptr_t addr) {
  return 0;
}

int flash_erase(uint8_t page, uint8_t page_count) {
  return -1;
}

int flash_copy(const uint32_t* src, __IO uint32_t* dst, uint32_t size) {
  return -1;
}

void test_fw_load(void) {

}

void test_fw_upgrade(void) {

}

TEST_LIST = {
   { "fw_load", test_fw_load },
   { "fw_upgrade", test_fw_upgrade },
   { 0 }
};
