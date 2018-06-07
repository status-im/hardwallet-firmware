#include "acutest.h"
#include <string.h>

#include "../Src/bip39.c"

const uint8_t rand128[] = {0x1f, 0x67, 0x25, 0xf5, 0xed, 0xf7, 0x55, 0x65, 0x58, 0x68, 0x41, 0xcb, 0xe3, 0x65, 0x92, 0x88};
const uint8_t sha256_rand128[] = {0x92, 0xd8, 0x20, 0xe0, 0x49, 0x0f, 0x78, 0x5e, 0xe9, 0x09, 0xa3, 0x9d, 0xe8, 0x0a, 0x90, 0xc3, 0xc5, 0xd3, 0x05, 0x6a, 0xa8, 0x3f, 0x22, 0xe1, 0x95, 0x4d, 0xea, 0x90, 0x53, 0x92, 0xc4, 0x8e};
const uint16_t mnemonic_12[] = { 251, 457, 1003, 1759, 938, 1429, 781, 65, 1631, 217, 805, 137 };
const uint16_t mnemonic_12_wrong[] = { 251, 457, 1003, 1759, 938, 1429, 781, 65, 1631, 217, 805, 136 };
const char *mnemonic_str_12 = "buyer decrease latin swim inspire razor giant amused slogan brass gorilla bachelor";
const char *passphrase = "a test passpharase";

#define MNEMONIC_STR_12_LEN 82
#define PASSPHRASE_LEN 18

void sha256(const uint8_t* data, int len, uint8_t hash[SHA_256_LEN]) {
  if (len == 16 && !memcmp(data, rand128, len)) {
    memcpy(hash, sha256_rand128, SHA_256_LEN);
  } else {
    memset(hash, 0xff, SHA_256_LEN);
  }
}

void pbkdf2_sha512(const uint8_t *pw, int pwlen, const uint8_t *salt, int saltlen, uint32_t iterations, uint8_t *out, int outlen) {
  TEST_CHECK(iterations == 2048);
  TEST_CHECK(outlen == BIP39_SEED_LEN);
  TEST_CHECK(pwlen == MNEMONIC_STR_12_LEN);
  TEST_CHECK(!memcmp(pw, mnemonic_str_12, MNEMONIC_STR_12_LEN));
  TEST_CHECK(!memcmp(salt, "mnemonic", 8));
  TEST_CHECK(saltlen == 8 || saltlen == 8 + PASSPHRASE_LEN);
  if (saltlen > 8) {
    TEST_CHECK(!memcmp(&salt[8], passphrase, PASSPHRASE_LEN));
  }
}

int rng(uint8_t *dst, unsigned int size) {
  if (size == 16) {
    memcpy(dst, rand128, size);
    return 1;
  }

  return 0;
}


void test_bip39_generate_mnemonic(void) {
  uint16_t mnemo[12];
  TEST_CHECK(bip39_generate_mnemonic(4, mnemo) == 12);
  TEST_CHECK(memcmp(mnemo, mnemonic_12, 24) == 0);
}

void test_bip39_find_word(void) {
  TEST_CHECK(bip39_find_word("shy") == 1596);
  TEST_CHECK(bip39_find_word("defense") == 459);
  TEST_CHECK(bip39_find_word("abandon") == 0);
  TEST_CHECK(bip39_find_word("zoo") == 2047);
  TEST_CHECK(bip39_find_word("abacus") == -1);
  TEST_CHECK(bip39_find_word("zoom") == -1);
  TEST_CHECK(bip39_find_word("noword") == -1);
}

void test_bip39_verify(void) {
  TEST_CHECK(!bip39_verify(4, mnemonic_12));
  TEST_CHECK(bip39_verify(4, mnemonic_12_wrong) == -1);
}

void test_bip39_render_mnemonic(void) {
  char str[MNEMONIC_STR_12_LEN + 1];
  int len;

  bip39_render_mnemonic(mnemonic_12, 12, str, &len);

  TEST_CHECK(!memcmp(str, mnemonic_str_12, len));
  TEST_CHECK(len == MNEMONIC_STR_12_LEN);
  TEST_CHECK(str[len] == '\0');
}

void test_bip39_generate_seed(void) {
  TEST_CHECK(bip39_generate_seed(mnemonic_str_12, MNEMONIC_STR_12_LEN, passphrase, 65, NULL) == -1);
  TEST_CHECK(!bip39_generate_seed(mnemonic_str_12, MNEMONIC_STR_12_LEN, passphrase, PASSPHRASE_LEN, NULL));
  TEST_CHECK(!bip39_generate_seed(mnemonic_str_12, MNEMONIC_STR_12_LEN, NULL, 0, NULL));
}

TEST_LIST = {
    { "bip39_generate_mnemonic", test_bip39_generate_mnemonic },
    { "bip39_find_word", test_bip39_find_word },
    { "bip39_verify", test_bip39_verify },
    { "bip39_render_mnemonic", test_bip39_render_mnemonic },
    { "bip39_generate_seed", test_bip39_generate_seed },
    { 0 }
};
