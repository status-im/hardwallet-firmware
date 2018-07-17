#include "acutest.h"
#include <string.h>
#include <stdint.h>

#define __IO volatile
#define FLASH_BASE ((uintptr_t) 0)

#include "../Src/pin.c"

uint32_t wo_page[512];


uint8_t pin_min[] = { 0x04, 0x31, 0x32, 0x33, 0x34 };
uint8_t pin_max[] = { 0x0F, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39 };
uint8_t pin_short[] = { 0x03, 0x31, 0x32, 0x33 };
uint8_t pin_long[] = { 0x10, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30 };

uint8_t pin_entry[PIN_ENTRY_SIZE*4];

int is_pin_set;
uint32_t pin[PIN_ENTRY_SIZE];

int is_counter_set;
uint32_t counters[PIN_COUNTER_ENTRY_SIZE];

void test_init() {
  pin_unverify();

  wo_page[0] = FS_V1_PAGE_ID(FS_WRITE_ONCE_ID, 0);
  wo_page[1] = 0;

  for (int i = PIN_ENC_KEY_IDX; i < (PIN_ENC_KEY_IDX + 4); i++) {
    wo_page[i] = 0xAAAAAAAA;
  }

  rng(pin_entry, AES_BLOCK_SIZE);

  for (int i = AES_BLOCK_SIZE; i < (AES_BLOCK_SIZE * 2); i++) {
    pin_entry[i] = 0;
  }

  is_pin_set = 0;
  is_counter_set = 0;
}

int rng(uint8_t* dst, unsigned int size) {
  for(int i = 0; i < size; i++) {
    dst[i] = i;
  }

  return 1;
}

uint32_t* fs_find_last_entry(uint32_t page_num, int page_count, int entry_size) {
  switch(page_num) {
  case FS_PIN_DATA_PAGE:
    TEST_CHECK(page_count == FS_PIN_DATA_COUNT);
    TEST_CHECK(entry_size == PIN_ENTRY_SIZE);
    return is_pin_set ? pin : NULL;
  case FS_COUNTERS_PAGE:
    TEST_CHECK(page_count == FS_COUNTERS_COUNT);
    TEST_CHECK(entry_size == PIN_COUNTER_ENTRY_SIZE);
    return is_counter_set ? counters : NULL;
  default:
    TEST_CHECK("Unexpected page number" == 0);
    return NULL;
  }
}

uint32_t* fs_get_page(uint32_t base_page, int index) {
  TEST_CHECK(base_page == FS_WRITE_ONCE_PAGE);
  TEST_CHECK(index == 0);

  return wo_page;
}

int fs_replace_entry(uint32_t page_num, int page_count, int entry_size, const uint32_t *entry) {
  switch(page_num) {
  case FS_PIN_DATA_PAGE:
    TEST_CHECK(page_count == FS_PIN_DATA_COUNT);
    TEST_CHECK(entry_size == PIN_ENTRY_SIZE);
    is_pin_set = 1;
    memcpy(pin, entry, (entry_size*4));
    return 0;
  case FS_COUNTERS_PAGE:
    TEST_CHECK(page_count == FS_COUNTERS_COUNT);
    TEST_CHECK(entry_size == PIN_COUNTER_ENTRY_SIZE);
    is_counter_set = 1;
    memcpy(counters, entry, (entry_size*4));
    return 0;
  default:
    TEST_CHECK("Unexpected page number" == 0);
    return -1;
  }
}

void aes128_cbc_enc(const uint8_t key[AES_128_KEYLEN], const uint8_t iv[AES_BLOCK_SIZE], const uint8_t* data, int blocks, uint8_t* out) {
  TEST_CHECK(blocks == 1);

  for (int i = 0; i < AES_128_KEYLEN; i++) {
    TEST_CHECK(key[i] == 0xAA);
  }

  for (int i = 0; i < AES_BLOCK_SIZE; i++) {
    TEST_CHECK(iv[i] == i);
  }

  memcpy(out, data, AES_BLOCK_SIZE);
}

void test_pin_set(void) {
  test_init();

  TEST_CHECK(pin_set(pin_short) == -1);
  TEST_CHECK(!is_pin_set);
  TEST_CHECK(is_counter_set);
  TEST_CHECK(counters[0] == 0 && counters[1] == 0);

  TEST_CHECK(pin_set(pin_long) == -1);
  TEST_CHECK(!is_pin_set);

  memcpy(&pin_entry[AES_BLOCK_SIZE], pin_min, 5);
  TEST_CHECK(!pin_set(pin_min));
  TEST_CHECK(is_pin_set);
  TEST_CHECK(!memcmp(pin_entry, pin, (PIN_ENTRY_SIZE * 4)));

  TEST_CHECK(pin_set(pin_max) == -1);
  TEST_CHECK(!memcmp(pin_entry, pin, (PIN_ENTRY_SIZE * 4)));
}

void test_pin_is_set(void) {
  test_init();

  TEST_CHECK(!pin_is_set(pin_short));
  TEST_CHECK(!pin_set(pin_min));
  TEST_CHECK(pin_is_set(pin_short));
}

void test_pin_change(void) {
  test_init();

  TEST_CHECK(pin_change(pin_min) == -1);
  pin_set(pin_max);

  TEST_CHECK(pin_change(pin_min) == -1);

  pin_verify(pin_max);
  TEST_CHECK(pin_change(pin_short) == -1);

  TEST_CHECK(pin_change(pin_min) == 1);
}

void test_pin_verify(void) {
  test_init();
  TEST_CHECK(pin_verify(pin_max) == -1);
  TEST_CHECK(!pin_set(pin_max));

  TEST_CHECK(pin_verify(pin_short) == 0);
  TEST_CHECK(pin_remaining_tries() == PIN_MAX_RETRIES - 1);
  TEST_CHECK(!pin_is_verified());

  TEST_CHECK(pin_verify(pin_long) == 0);
  TEST_CHECK(pin_remaining_tries() == PIN_MAX_RETRIES - 2);
  TEST_CHECK(!pin_is_verified());

  TEST_CHECK(pin_verify(pin_min) == 0);
  TEST_CHECK(pin_remaining_tries() == PIN_MAX_RETRIES - 3);
  TEST_CHECK(!pin_is_verified());

  TEST_CHECK(pin_verify(pin_max) == 1);
  TEST_CHECK(pin_remaining_tries() == PIN_MAX_RETRIES);
  TEST_CHECK(pin_is_verified());

  for (int i = 1; i <= PIN_MAX_RETRIES; i++) {
    TEST_CHECK(pin_verify(pin_min) == 0);
    TEST_CHECK(pin_remaining_tries() == PIN_MAX_RETRIES - i);
    TEST_CHECK(!pin_is_verified());
  }

  TEST_CHECK(pin_verify(pin_max) == 0);
  TEST_CHECK(pin_remaining_tries() == 0);
  TEST_CHECK(!pin_is_verified());

  counters[1] = 2;
  TEST_CHECK(pin_verify(pin_max) == 1);
  TEST_CHECK(pin_remaining_tries() == PIN_MAX_RETRIES);
  TEST_CHECK(pin_is_verified());
}

void test_pin_is_verified(void) {
  test_init();
  TEST_CHECK(!pin_set(pin_max));

  TEST_CHECK(!pin_is_verified());
  TEST_CHECK(pin_verify(pin_max) == 1);
  TEST_CHECK(pin_is_verified());
}

void test_pin_unverify(void) {
  test_init();

  TEST_CHECK(!pin_set(pin_max));
  TEST_CHECK(pin_verify(pin_max) == 1);

  pin_unverify();
  TEST_CHECK(!pin_is_verified());
}

void test_pin_remaining_tries(void) {
  test_init();
  TEST_CHECK(pin_remaining_tries() == -1);
  is_counter_set = 1;

  counters[0] = 0x00; counters[1] = PIN_MAX_RETRIES - 0x02;
  TEST_CHECK(pin_remaining_tries() == 2);

  counters[0] = 0x00; counters[1] = PIN_MAX_RETRIES - 0x02;
  TEST_CHECK(pin_remaining_tries() == 2);

  counters[0] = 0xff; counters[1] = 0xf2;
  TEST_CHECK(pin_remaining_tries() ==  PIN_MAX_RETRIES - 2);

}

TEST_LIST = {
   { "pin_set", test_pin_set },
   { "pin_is_set", test_pin_is_set },
   { "pin_change", test_pin_change },
   { "pin_verify", test_pin_verify },
   { "pin_is_verified", test_pin_is_verified },
   { "pin_unverify", test_pin_unverify },
   { "pin_remaining_tries", test_pin_remaining_tries },
   { 0 }
};
