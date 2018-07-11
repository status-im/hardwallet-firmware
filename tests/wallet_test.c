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

uint8_t seed[64];
uint8_t xy[64];

uint32_t wo[512];
uint8_t* wo_bytes = (uint8_t*) wo;

uint32_t cache[512];
uint32_t key_entry[WALLET_KEY_SIZE];

int aes128_cbc_enc_called;
int aes128_cbc_dec_called;
int store_cache_entry_cnt;

void test_reset_calls() {
  aes128_cbc_enc_called = 0;
  aes128_cbc_dec_called = 0;
  store_cache_entry_cnt = 0;

  for (int i = 0; i < WALLET_KEY_SIZE; i++) {
    key_entry[i] = 0xffffffff;
  }
}

void test_init() {
  for(int i = 2; i < 512; i++) {
    wo[i] = 0xffffffff;
    cache[i] = 0xffffffff;
  }

  wo[0] = FS_V1_PAGE_ID(FS_WRITE_ONCE_ID, 0);
  wo[1] = 0;

  cache[0] = FS_V1_PAGE_ID(FS_KEY_CACHE_ID, 0);
  cache[1] = 0;

  for (int i = WALLET_ENC_KEY_IDX; i < (WALLET_ENC_KEY_IDX + 4); i++) {
    wo[i] = 0xAAAAAAAA;
  }

  cache[2] = 0x02;
  cache[3] = 0x2c;
  cache[4] = 0x01;

  for(int i = WALLET_PATH_LEN; i < WALLET_KEY_SIZE; i++) {
    cache[2 + i] = 0xaabbcc00 | i;
  }

  cache[2 + WALLET_KEY_SIZE] = 0x03;
  cache[3 + WALLET_KEY_SIZE] = 0x2c;
  cache[4 + WALLET_KEY_SIZE] = 0x01;
  cache[5 + WALLET_KEY_SIZE] = 0x01;

  for(int i = WALLET_PATH_LEN; i < WALLET_KEY_SIZE; i++) {
    cache[2 + WALLET_KEY_SIZE + i] = 0xddeeff00 | i;
  }

  test_reset_calls();

  for (int i = 0; i < 64; i++) {
    seed[i] = (0x80 + i);
    xy[i] = (0x10 + i);
  }
}

int bip32_ckd_private(uint32_t i, const bip32_priv_key_t* priv_key, const bip32_pub_key_t* pub_key, bip32_priv_key_t* out_priv, bip32_pub_key_t* out_pub) {
  memcpy(out_priv, priv_key, sizeof(bip32_priv_key_t));
  out_priv->key[0] = (uint8_t) i;
  out_priv->key[1] = (uint8_t) i;
  out_priv->key[2] = (uint8_t) i;
  out_priv->key[3] = (uint8_t) i;

  memcpy(out_pub, pub_key, sizeof(bip32_pub_key_t));
  return 0;
}

void bip32_master_key(const uint8_t* seed, int seed_len, bip32_priv_key_t* out_priv, bip32_pub_key_t* out_pub) {
  memcpy(out_priv->key, seed, seed_len);
  out_pub->has_full_y = 1;
  out_pub->y_comp = 0x02;
  memcpy(out_pub->x, xy, seed_len);
}

void sha256(const uint8_t* data, int len, uint8_t hash[SHA_256_LEN]) {
  TEST_CHECK(len == (BIP32_KEY_COMPONENT_LEN * 2));
  memcpy(hash, &data[16], SHA_256_LEN);
}

int rng(uint8_t* dst, unsigned int size) {
  TEST_CHECK(size == 11);

  for(int i = 0; i < size; i++) {
    dst[i] = i;
  }

  return 1;
}

void test_fake_aes(const uint8_t key[AES_128_KEYLEN], const uint8_t iv[AES_BLOCK_SIZE], const uint8_t* data, int blocks, uint8_t* out) {
  TEST_CHECK(key == (uint8_t*)(&wo[WALLET_ENC_KEY_IDX]));
  memcpy(out, data, blocks * 16);
}

void aes128_cbc_enc(const uint8_t key[AES_128_KEYLEN], const uint8_t iv[AES_BLOCK_SIZE], const uint8_t* data, int blocks, uint8_t* out) {
  aes128_cbc_enc_called = 1;
  test_fake_aes(key, iv, data, blocks, out);
}

void aes128_cbc_dec(const uint8_t key[AES_128_KEYLEN], const uint8_t iv[AES_BLOCK_SIZE], const uint8_t* data, int blocks, uint8_t* out) {
  aes128_cbc_dec_called = 1;
  test_fake_aes(key, iv, data, blocks, out);
}

uint32_t* fs_get_page(uint32_t base_page, int index) {
  if (base_page == FS_WRITE_ONCE_PAGE) {
    TEST_CHECK(index == 0);
    return wo;
  } else if (base_page == FS_KEY_CACHE_PAGE) {
    TEST_CHECK(index == 0);
    return cache;
  } else {
    TEST_CHECK(!"Unexpected page");
    return NULL;
  }
}

uint32_t* fs_cache_entry(uint32_t cache_start, int page_count, int entry_size, const uint32_t* entry) {
  store_cache_entry_cnt++;
  TEST_CHECK(entry_size == WALLET_KEY_SIZE);
  memcpy(key_entry, entry, (entry_size * 4));
  return key_entry;
}

int fs_write_entry(uint32_t* addr, const uint32_t* entry, int entry_size) {
  TEST_CHECK(addr == &wo[WALLET_MK_IDX]);
  TEST_CHECK(entry_size == WALLET_MK_SIZE);
  memcpy(addr, entry, entry_size * 4);
  return 0;
}

void test_wallet_new(void) {
  test_init();

  wo[WALLET_MK_IDX] = 0x00000000;
  TEST_CHECK(wallet_new(seed, 64) == -1);
  wo[WALLET_MK_IDX] = 0xffffffff;

  TEST_CHECK(!wallet_new(seed, 64));
  TEST_CHECK(aes128_cbc_enc_called);

  for (int i = 0; i < 11; i++) {
    TEST_CHECK(wo_bytes[(WALLET_MK_IDX * 4) + i] == i);
  }

  TEST_CHECK(wo_bytes[(WALLET_MK_IDX * 4) + 11] == 2);
  TEST_CHECK(!memcmp(&wo[WALLET_MK_IDX + 3], xy, 32));
  TEST_CHECK(!memcmp(&wo[WALLET_MK_IDX + 11], seed, 64));
  TEST_CHECK(!memcmp(&wo[WALLET_MK_IDX + 27], &xy[16], 20));
  TEST_CHECK(wo[WALLET_MK_IDX + 32] == 0xffffffff);
}

void test_wallet_created(void) {
  test_init();

  TEST_CHECK(!wallet_created());
  TEST_CHECK(!wallet_new(seed, 64));
  TEST_CHECK(wallet_created() == 1);
}

void test_wallet_priv_key(void) {
  test_init();
  uint8_t priv[BIP32_KEY_COMPONENT_LEN];
  uint32_t path[WALLET_PATH_LEN];

  TEST_CHECK(wallet_priv_key(path, priv) == -1);

  TEST_CHECK(!wallet_new(seed, 64));
  test_reset_calls();

  path[0] = 0x00;
  TEST_CHECK(wallet_priv_key(path, priv) == -1);

  path[0] = 0x0A;
  TEST_CHECK(wallet_priv_key(path, priv) == -1);

  path[0] = 0x03;
  path[1] = 0x2c;
  path[2] = 0x01;
  path[3] = 0x01;
  TEST_CHECK(!wallet_priv_key(path, priv));
  TEST_CHECK(!memcmp(priv, &cache[2 + WALLET_KEY_SIZE + WALLET_PATH_LEN + 11], BIP32_KEY_COMPONENT_LEN));
  TEST_CHECK(store_cache_entry_cnt == 0);
  TEST_CHECK(!aes128_cbc_enc_called);
  TEST_CHECK(aes128_cbc_dec_called);
  test_reset_calls();

  path[0] = 0x09; path[1] = 0x2c; path[2] = 0x01; path[3] = 0x03; path[4] = 0x04; path[5] = 0x05; path[6] = 0x06; path[7] = 0x07; path[8] = 0x08; path[9] = 0x09;
  TEST_CHECK(!wallet_priv_key(path, priv));
  TEST_CHECK(!memcmp(priv, &key_entry[WALLET_PATH_LEN + 11], BIP32_KEY_COMPONENT_LEN));
  TEST_CHECK(!memcmp(path, key_entry, (WALLET_PATH_LEN * 4)));
  TEST_CHECK(key_entry[WALLET_PATH_LEN + 11] == 0x09090909);
  TEST_CHECK(store_cache_entry_cnt == 7);
  TEST_CHECK(aes128_cbc_enc_called);
  TEST_CHECK(aes128_cbc_dec_called);
  test_reset_calls();

  path[0] = 0x02; path[1] = 0x01; path[2] = 0x02; path[3] = 0xffffffff; path[4] = 0xffffffff; path[5] = 0xffffffff; path[6] = 0xffffffff; path[7] = 0xffffffff; path[8] = 0xffffffff; path[9] = 0xffffffff;
  TEST_CHECK(!wallet_priv_key(path, priv));
  TEST_CHECK(!memcmp(priv, &key_entry[WALLET_PATH_LEN + 11], BIP32_KEY_COMPONENT_LEN));
  TEST_CHECK(!memcmp(path, key_entry, (WALLET_PATH_LEN * 4)));
  TEST_CHECK(key_entry[WALLET_PATH_LEN + 11] == 0x02020202);
  TEST_CHECK(store_cache_entry_cnt == 2);
  TEST_CHECK(aes128_cbc_enc_called);
  TEST_CHECK(aes128_cbc_dec_called);
}

void test_wallet_address(void) {
  test_init();

  uint8_t addr[WALLET_ADDR_LEN];
  uint32_t path[WALLET_PATH_LEN];

  TEST_CHECK(!wallet_new(seed, 64));
  test_reset_calls();

  path[0] = 0x02;
  path[1] = 0x2c;
  path[2] = 0x01;
  TEST_CHECK(!wallet_address(path, addr));
  TEST_CHECK(!memcmp(addr, &cache[2 + WALLET_KEY_SIZE - (WALLET_ADDR_LEN / 4)], WALLET_ADDR_LEN));
}

void test_wallet_master_address(void) {
  test_init();
  uint8_t addr[WALLET_ADDR_LEN];
  TEST_CHECK(wallet_master_address(addr) == -1);
  TEST_CHECK(!wallet_new(seed, 64));
  TEST_CHECK(!wallet_master_address(addr));
  TEST_CHECK(!memcmp(addr, &xy[16], 20));
}

TEST_LIST = {
   { "wallet_new", test_wallet_new },
   { "wallet_created", test_wallet_created },
   { "wallet_priv_key", test_wallet_priv_key },
   { "wallet_address", test_wallet_address },
   { "wallet_master_address", test_wallet_master_address },
   { 0 }
};
