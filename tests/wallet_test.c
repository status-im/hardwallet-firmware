#include "acutest.h"
#include <string.h>

#define __IO volatile
#define FLASH_BASE ((uintptr_t) 0)

#ifndef __GNUC__
#define SWAP(x, y) \
    do { \
      void* __tmp = (x); \
      (x) = (y); \
      (y) = __tmp; \
    } while (0)
#endif

#include "../Src/wallet.c"

int bip32_ckd_private(uint32_t i, const bip32_priv_key_t* priv_key, const bip32_pub_key_t* pub_key, bip32_priv_key_t* out_priv, bip32_pub_key_t* out_pub) {
  return -1;
}

void bip32_master_key(const uint8_t* seed, int seed_len, bip32_priv_key_t* out_priv, bip32_pub_key_t* out_pub) {

}

void sha256(const uint8_t* data, int len, uint8_t hash[SHA_256_LEN]) {

}

int rng(uint8_t* dst, unsigned int size) {
  for(int i = 0; i < size; i++) {
    dst[i] = i;
  }

  return 1;
}

void aes128_cbc_enc(const uint8_t key[AES_128_KEYLEN], const uint8_t iv[AES_BLOCK_SIZE], const uint8_t* data, int blocks, uint8_t* out) {

}

void aes128_cbc_dec(const uint8_t key[AES_128_KEYLEN], const uint8_t iv[AES_BLOCK_SIZE], const uint8_t* data, int blocks, uint8_t* out) {

}

uint32_t* fs_get_page(uint32_t base_page, int index) {
  return NULL;
}

uint32_t* fs_cache_entry(uint32_t cache_start, int page_count, int entry_size, const uint32_t *entry) {
  return NULL;
}

int fs_write_entry(uint32_t* addr, const uint32_t* entry, int entry_size) {
  return -1;
}

void test_wallet_new(void) {
  TEST_CHECK(0);
}

void test_wallet_priv_key(void) {
  TEST_CHECK(0);
}

void test_wallet_address(void) {
  TEST_CHECK(0);
}

void test_wallet_master_address(void) {
  TEST_CHECK(0);
}

TEST_LIST = {
   { "wallet_new", test_wallet_new },
   { "wallet_priv_key", test_wallet_priv_key },
   { "wallet_address", test_wallet_address },
   { "wallet_master_address", test_wallet_master_address },

   { 0 }
};
