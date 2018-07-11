#include "acutest.h"
#include <string.h>

#define __IO volatile
#define FLASH_BASE ((uintptr_t) 0)

#include "../Src/init.c"

err_t ui_authenticate_user() {
  return ERR_UNKNOWN;
}

err_t ui_display_mnemonic(const uint16_t* mnemonic, int mnlen) {
  return ERR_UNKNOWN;
}

err_t ui_read_mnemonic(uint16_t* mnemonic, int mnlen) {
  return ERR_UNKNOWN;
}

err_t ui_display_retry(void) {
  return ERR_UNKNOWN;
}

int wallet_new(const uint8_t* seed, int seed_len) {
  return -1;
}

int wallet_created() {
  return -1;
}

int wallet_master_address(uint8_t out_addr[WALLET_ADDR_LEN]) {
  return -1;
}

int bip39_generate_mnemonic(int cslen, uint16_t* mnemonic) {
  return -1;
}

int bip39_verify(int cslen, const uint16_t* mnemonic) {
  return -1;
}

void bip39_render_mnemonic(const uint16_t* mnemonic, int len, uint8_t* mnstr, int* mnlen) {

}

int bip39_generate_seed(const uint8_t* mnstr, int mnlen, const uint8_t* passphrase, int pplen, uint8_t seed[BIP39_SEED_LEN]) {
  return -1;
}

int fs_init() {
  return -1;
}

int fs_initialized() {
  return -1;
}

int pin_set(uint8_t* pin) {
  return -1;
}

int pin_is_set() {
  return -1;
}

int rng(uint8_t* dst, unsigned int size) {
  return 0;
}

void test_init_boot(void) {

}

void test_init_ready(void) {

}

void test_init_phase_0(void) {

}

void test_init_phase_1(void) {

}

void test_init_phase_2a(void) {

}

void test_init_phase_2b(void) {

}

void test_init_phase_4(void) {

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
