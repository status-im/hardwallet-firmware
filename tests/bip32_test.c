#include "acutest.h"
#include <string.h>

#include "../Src/bip32.c"

static int generating_master_key;

static int hmac_sha512_called = 0;
static int bignum256_cmp_called = 0;
static int bignum256_is_zero_called = 0;
static int bignum256_pubkey_called = 0;

const bip32_priv_key_t priv = {
  { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0xab, 0xbc },
  { 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xa0, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xb0, 0xbc, 0xbd, 0xbe, 0xbf, 0xcd, 0xde }
};

const bip32_pub_key_t pub = {
  { 0, 0 },
  1,
  3,
  { 0x21, 0x20, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xc0, 0xcd, 0xce, 0xcf, 0xef, 0xfa },
  { 0x21, 0x20, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xc0, 0xcd, 0xce, 0xcf, 0xef, 0xfb },
};

const uint8_t bip39_test_seed[] = { 0x75, 0x89, 0xac, 0x64, 0xc5, 0x08, 0x8d, 0x86, 0x67, 0xeb, 0xe6, 0xda, 0x96, 0xd9, 0x55, 0xe7, 0x7a, 0x8c, 0x5e, 0x0c, 0xc8, 0x87, 0x4a, 0xc0, 0x1b, 0x58, 0x0e, 0xc5, 0x5e, 0x9c, 0x32, 0xb6, 0x27, 0x6d, 0x29, 0xbc, 0x79, 0x38, 0x16, 0xea, 0x16, 0x6e, 0x4f, 0x5b, 0xf7, 0xf4, 0xec, 0x41, 0x28, 0x5e, 0x80, 0x51, 0xfc, 0x08, 0x28, 0xc9, 0x7b, 0xcd, 0xb2, 0x15, 0x94, 0x7e, 0x75, 0xec };

const uint32_t curve_n[32/4]; // empty, just need to compare address;

void bignum256_from_bytes(const uint8_t* bytes, bignum256_t* bignum) {
  uint32_t *native = (uint32_t *)bignum;
  memset(bignum, 0, 32);

  for (int i = 0; i < 32; i++) {
    unsigned b = 32 - 1 - i;
    native[b / 4] |= (uint32_t)bytes[i] << (8 * (b % 4));
  }
}

void bignum256_to_bytes(const bignum256_t* bignum, uint8_t* bytes) {
  uint32_t *native = (uint32_t *)bignum;

  for (int i = 0; i < 32; i++) {
    unsigned b = 32 - 1 - i;
    bytes[i] = native[b / 4] >> (8 * (b % 4));
  }
}

void bignum256_mod_add(const bignum256_t* a, const bignum256_t* b, const bignum256_t* mod, bignum256_t* c) {
  TEST_CHECK(((uint32_t *)a)[0] == 0x0e0fabbc);
  TEST_CHECK(((uint32_t *)b)[0] == 0xbebfcdde);
  TEST_CHECK(mod == bignum256_secp256k1_n());

  bignum256_from_bytes(pub.x, c);
}

int bignum256_is_zero(const bignum256_t* n) {
  TEST_CHECK(((uint32_t *)n)[0] == 0xcecfeffa);
  bignum256_is_zero_called = 1;
  return 0;
}

int bignum256_cmp(const bignum256_t* a, const bignum256_t* b) {
  TEST_CHECK(((uint32_t *)a)[0] == 0xbebfcdde);
  TEST_CHECK(b == bignum256_secp256k1_n());
  bignum256_cmp_called = 1;
  return -1;
}

bignum256_t* bignum256_secp256k1_n() {
  return (bignum256_t *) &curve_n;
}

void bignum256_secp256k1_publickey(const bignum256_t* priv_key, bignum256_t* out_pub) {
  bignum256_pubkey_called = 1;
  TEST_CHECK(((uint32_t *)priv_key)[0] == 0xcecfeffa);
  bignum256_from_bytes(priv.key, out_pub);
  bignum256_from_bytes(pub.y, (bignum256_t *)(((uint8_t *) out_pub) + 32));
}

void hmac_sha512(const uint8_t* key, int keylen, const uint8_t* data, int len, uint8_t hash[SHA_512_LEN]) {
  hmac_sha512_called = 1;

  if (generating_master_key) {
    TEST_CHECK(keylen == 12);
    TEST_CHECK(len == 64);
    TEST_CHECK(!memcmp(key, "Bitcoin Seed", keylen));
    TEST_CHECK(!memcmp(data, bip39_test_seed, len));
    memcpy(hash, pub.x, 32);
  } else {
    TEST_CHECK(keylen == 32);
    TEST_CHECK(len == 37);
    TEST_CHECK(!memcmp(key, priv.chain, keylen));

    if (data[33] == 0x84) {
      TEST_CHECK(data[0] == 0x00);
      TEST_CHECK(!memcmp(&data[1], priv.key, 32));
      TEST_CHECK(!memcmp(&data[34], "\5\6\7", 3));
    } else {
      TEST_CHECK(data[0] == 0x03);
      TEST_CHECK(!memcmp(&data[1], pub.x, 32));
      TEST_CHECK(!memcmp(&data[33], "\1\2\3\4", 4));
    }

    memcpy(hash, priv.chain, 32);
  }

  memcpy(&hash[32], pub.x, 32);
}

static void reset_calls() {
  hmac_sha512_called = 0;
  bignum256_cmp_called = 0;
  bignum256_is_zero_called = 0;
  bignum256_pubkey_called = 0;
}

static void check_calls(int pub) {
  TEST_CHECK(hmac_sha512_called);
  TEST_CHECK(bignum256_cmp_called);
  TEST_CHECK(bignum256_is_zero_called);
  TEST_CHECK(bignum256_pubkey_called == pub);
}

void test_bip32_ckd_private(void) {
  bip32_priv_key_t out_priv;
  bip32_pub_key_t out_pub;
  generating_master_key = 0;

  reset_calls();
  bip32_ckd_private(0x01020304, &priv, &pub, &out_priv, &out_pub);
  check_calls(1);

  TEST_CHECK(!memcmp(out_priv.key, pub.x, 32));
  TEST_CHECK(!memcmp(out_priv.chain, pub.x, 32));
  TEST_CHECK(!memcmp(out_pub.x, priv.key, 32));
  TEST_CHECK(!memcmp(out_pub.y, pub.y, 32));
  TEST_CHECK(out_pub.y_comp == 3);
  TEST_CHECK(out_pub.has_full_y == 1);

  reset_calls();
  bip32_ckd_private(0x84050607, &priv, &pub, &out_priv, NULL);
  check_calls(0);

  TEST_CHECK(!memcmp(out_priv.key, pub.x, 32));
  TEST_CHECK(!memcmp(out_priv.chain, pub.x, 32));
}

void test_bip32_master_key(void) {
  bip32_priv_key_t out_priv;
  bip32_pub_key_t out_pub;
  generating_master_key = 1;

  reset_calls();
  bip32_master_key(bip39_test_seed, 64, &out_priv, &out_pub);
  TEST_CHECK(hmac_sha512_called);
  TEST_CHECK(bignum256_pubkey_called);

  TEST_CHECK(!memcmp(out_priv.key, pub.x, 32));
  TEST_CHECK(!memcmp(out_priv.chain, pub.x, 32));
  TEST_CHECK(!memcmp(out_pub.x, priv.key, 32));
  TEST_CHECK(!memcmp(out_pub.y, pub.y, 32));
  TEST_CHECK(out_pub.y_comp == 3);
  TEST_CHECK(out_pub.has_full_y == 1);
}

TEST_LIST = {
    { "bip32_ckd_private", test_bip32_ckd_private },
    { "bip32_master_key", test_bip32_master_key },
    { 0 }
};
