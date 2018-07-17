#include "acutest.h"
#include <string.h>

#define __IO volatile
#define FLASH_BASE ((uintptr_t) 0)

#include "../Src/init.c"

static err_t test_ui_err;
static int test_fs_initialized;
static int test_pin_set;
static int test_wallet_created;
static int test_bip39_verify;

static int fs_init_called;
static int pin_set_called;
static int wallet_new_called;

err_t ui_authenticate_user() {
  return test_ui_err;
}

err_t ui_display_mnemonic(const uint16_t* mnemonic, int mnlen) {
  return test_ui_err;
}

err_t ui_read_mnemonic(uint16_t* mnemonic, int mnlen) {
  return test_ui_err;
}

err_t ui_display_retry(void) {
  return ERR_OK;
}

int wallet_new(const uint8_t* seed, int seed_len) {
  wallet_new_called = 1;
  return 0;
}

int wallet_created() {
  return test_wallet_created;
}

int wallet_master_address(uint8_t out_addr[WALLET_ADDR_LEN]) {
  out_addr[0] = 0xaa;
  return 0;
}

int bip39_generate_mnemonic(int cslen, uint16_t* mnemonic) {
  TEST_CHECK(cslen == 8);
  for (int i = 0; i < 24; i++) {
    mnemonic[i] = i * 3;
  }

  return 24;
}

int bip39_verify(int cslen, const uint16_t* mnemonic) {
  return test_bip39_verify++;
}

void bip39_render_mnemonic(const uint16_t* mnemonic, int len, uint8_t* mnstr, int* mnlen) {
  *mnlen = 50;
}

int bip39_generate_seed(const uint8_t* mnstr, int mnlen, const uint8_t* passphrase, int pplen, uint8_t seed[BIP39_SEED_LEN]) {
  return 0;
}

int fs_init() {
  fs_init_called++;
  return 0;
}

int fs_initialized() {
  return test_fs_initialized;
}

int pin_set(uint8_t* pin) {
  TEST_CHECK(pin[0] == UI_PIN_LEN);
  TEST_CHECK(pin[1] == 0x32);
  TEST_CHECK(pin[2] == 0x35);
  TEST_CHECK(pin[3] == 0x34);
  TEST_CHECK(pin[4] == 0x39);
  TEST_CHECK(pin[5] == 0x36);

  pin_set_called++;
  return 0;
}

int pin_is_set() {
  return test_pin_set;
}

int rng(uint8_t* dst, unsigned int size) {
  TEST_CHECK(size == UI_PIN_LEN);
  dst[0] = 0xd4;
  dst[1] = 0x05;
  dst[2] = 0x22;
  dst[3] = 0x81;
  dst[4] = 0x88;

  return 1;
}

void test_init_boot(void) {
  test_fs_initialized = 0; test_pin_set = 0; test_wallet_created = 0;
  TEST_CHECK(init_boot() == -1);
  test_fs_initialized = 1; test_pin_set = 0; test_wallet_created = 0;
  TEST_CHECK(init_boot() == 0);
  test_fs_initialized = 1; test_pin_set = 1; test_wallet_created = 0;
  TEST_CHECK(init_boot() == 1);
  test_fs_initialized = 1; test_pin_set = 1; test_wallet_created = 1;
  TEST_CHECK(init_boot() == 4);
}

void test_init_ready(void) {
  test_fs_initialized = 1; test_pin_set = 1; test_wallet_created = 0;
  init_boot();
  TEST_CHECK(init_ready() == 0);
  test_fs_initialized = 1; test_pin_set = 1; test_wallet_created = 1;
  init_boot();
  TEST_CHECK(init_ready() == 1);
}

void test_init_phase_0(void) {
  fs_init_called = 0;
  test_fs_initialized = 0; test_pin_set = 0; test_wallet_created = 0;
  init_boot();
  TEST_CHECK(init_phase_0() == ERR_OK);
  TEST_CHECK(init_phase == 0);
  TEST_CHECK(fs_init_called == 1);

  test_fs_initialized = 1; test_pin_set = 1; test_wallet_created = 0;
  init_boot();
  TEST_CHECK(init_phase_0() == ERR_OK);
  TEST_CHECK(init_phase == 0);
  TEST_CHECK(fs_init_called == 2);

  test_fs_initialized = 1; test_pin_set = 1; test_wallet_created = 1;
  init_boot();
  test_ui_err = ERR_UNAUTHORIZED;
  TEST_CHECK(init_phase_0() == ERR_UNAUTHORIZED);
  TEST_CHECK(init_phase == 4);
  TEST_CHECK(fs_init_called == 2);

  test_ui_err = ERR_OK;
  TEST_CHECK(init_phase_0() == ERR_OK);
  TEST_CHECK(init_phase == 0);
  TEST_CHECK(fs_init_called == 3);
}

void test_init_phase_1(void) {
  test_fs_initialized = 0; test_pin_set = 0; test_wallet_created = 0;
  init_boot();
  TEST_CHECK(init_phase_1() == ERR_INVALID_DATA);
  TEST_CHECK(init_phase == -1);
  TEST_CHECK(init_phase_0() == ERR_OK);
  TEST_CHECK(init_phase_1() == ERR_OK);
  TEST_CHECK(init_phase == 1);
  TEST_CHECK(pin_set_called == 1);
}

void test_init_phase_2a(void) {
  test_fs_initialized = 0; test_pin_set = 0; test_wallet_created = 0;
  init_boot();
  TEST_CHECK(init_phase_2a(8) == ERR_INVALID_DATA);
  TEST_CHECK(init_phase_0() == ERR_OK);
  TEST_CHECK(init_phase_2a(8) == ERR_INVALID_DATA);
  TEST_CHECK(init_phase_1() == ERR_OK);

  test_ui_err = ERR_USER_CANCELLED;
  TEST_CHECK(init_phase_2a(8) == ERR_USER_CANCELLED);
  TEST_CHECK(init_phase == 1);
  test_ui_err = ERR_OK;
  TEST_CHECK(init_phase_2a(8) == ERR_OK);
  TEST_CHECK(init_phase == 2);
}

void test_init_phase_2b(void) {
  test_fs_initialized = 0; test_pin_set = 0; test_wallet_created = 0;
  init_boot();
  TEST_CHECK(init_phase_2b(8) == ERR_INVALID_DATA);
  TEST_CHECK(init_phase_0() == ERR_OK);
  TEST_CHECK(init_phase_2b(8) == ERR_INVALID_DATA);
  TEST_CHECK(init_phase_1() == ERR_OK);

  test_ui_err = ERR_USER_CANCELLED;
  TEST_CHECK(init_phase_2b(8) == ERR_USER_CANCELLED);
  TEST_CHECK(init_phase == 1);
  test_ui_err = ERR_OK;
  test_bip39_verify = -3;
  TEST_CHECK(init_phase_2b(8) == ERR_OK);
  TEST_CHECK(init_phase == 2);
}

void test_init_phase_4(void) {
  uint8_t addr[WALLET_ADDR_LEN];
  addr[0] = 0xbb;

  test_fs_initialized = 0; test_pin_set = 0; test_wallet_created = 0;
  init_boot();
  TEST_CHECK(init_phase_4("", 0, addr) == ERR_INVALID_DATA);
  TEST_CHECK(init_phase_0() == ERR_OK);
  TEST_CHECK(init_phase_4("", 0, addr) == ERR_INVALID_DATA);
  TEST_CHECK(init_phase_1() == ERR_OK);
  TEST_CHECK(init_phase_4("", 0, addr) == ERR_INVALID_DATA);
  TEST_CHECK(init_phase_2a(8) == ERR_OK);

  wallet_new_called = 0;
  TEST_CHECK(init_phase_4("", 0, addr) == ERR_OK);
  TEST_CHECK(addr[0] == 0xaa);
  TEST_CHECK(init_phase == 4);
  TEST_CHECK(wallet_new_called == 1);
}

TEST_LIST = {
   { "init_boot", test_init_boot },
   { "init_ready", test_init_ready },
   { "init_phase_0", test_init_phase_0 },
   { "init_phase_1", test_init_phase_1 },
   { "init_phase_2a", test_init_phase_2a },
   { "init_phase_2b", test_init_phase_2b },
   { "init_phase_4", test_init_phase_4 },
   { 0 }
};
