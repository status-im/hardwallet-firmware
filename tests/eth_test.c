#include "acutest.h"
#include <string.h>

#include "../Src/rlp.c"
#include "../Src/eth.c"

#define TX1_LEN 44
uint8_t TX1_BUF[] = { 0x00,0xeb,0x02,0x85,0x02,0xef,0x04,0xbe,0x00,0x82,0x52,0x08,0x94,0xd2,0xc5,0xde,0x69,0xd9,0x65,0xcb,0xfd,0x87,0x1e,0x4f,0x1c,0x21,0x78,0x82,0x19,0x10,0x0f,0x7c,0xfc,0x87,0x81,0x57,0x4f,0xbe,0x28,0x00,0x00,0x80,0x01,0x80,0x80, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t *TX1 = &TX1_BUF[1];
uint8_t TX1_HASH[] = { 0x31,0x59,0xa1,0xbf,0x36,0x8a,0x87,0x88,0x80,0xc7,0xb2,0xee,0x22,0x11,0x36,0x36,0x43,0xb7,0x5a,0x4d,0xc4,0xf5,0x78,0xea,0x47,0x27,0x2d,0x57,0x8e,0xba,0x13,0x6d };

#define TX2_LEN 374
uint8_t TX2[] = { 0xf9,0x01,0x73,0x01,0x85,0x03,0xe9,0x5b,0xa8,0x00,0x83,0x01,0x24,0xf8,0x94,0x09,0x67,0x87,0x41,0xbd,0x50,0xc3,0xe7,0x43,0x01,0xf3,0x8f,0xbd,0x01,0x36,0x30,0x70,0x99,0xae,0x5d,0x88,0x03,0x72,0xa2,0xdd,0x1b,0xff,0x80,0x00,0xb9,0x01,0x44,0xf1,0xe0,0x3d,0x67,0x1e,0xe9,0xe0,0xd6,0x60,0x12,0x4b,0xc6,0xab,0x21,0x7a,0x8e,0x41,0x8a,0xa4,0xa8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xd2,0xc5,0xde,0x69,0xd9,0x65,0xcb,0xfd,0x87,0x1e,0x4f,0x1c,0x21,0x78,0x82,0x19,0x10,0x0f,0x7c,0xfc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x4f,0xe8,0x71,0xb5,0xcb,0x1b,0x56,0x82,0x99,0xd7,0xc6,0x06,0xa1,0x0f,0x37,0xf8,0x67,0x1f,0x5d,0x51,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x72,0xa2,0xdd,0x1b,0xff,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x64,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1c,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x5a,0xf5,0x80,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1b,0xa7,0x6b,0xcd,0xee,0x47,0xf0,0x24,0x0f,0x38,0x82,0x92,0xb0,0xc0,0xe9,0xde,0xc3,0xc4,0xd4,0x19,0xcc,0xc5,0x24,0x2d,0x29,0xee,0x56,0xd3,0x15,0x4a,0x31,0x34,0x5c,0x4f,0x8b,0x73,0xf3,0x74,0xd1,0x75,0xb1,0x0d,0xa6,0xf2,0x07,0x3f,0x0a,0xb6,0x6d,0x69,0xc6,0x1a,0xe5,0x21,0x97,0x23,0xec,0x0d,0x50,0xdc,0xb6,0xc8,0xe5,0x8c,0xaa,0x01,0x80,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
uint8_t TX2_HASH[] = { 0xa7,0x6d,0x81,0x81,0x24,0x3f,0xb3,0x35,0x28,0xc9,0x34,0x32,0xa0,0x7b,0x43,0x09,0xfc,0xd6,0x59,0x71,0x8c,0x47,0x8c,0x51,0x97,0xe5,0x47,0xac,0x5d,0xa7,0x90,0x67 };

uint8_t UNSUPPORTED_LEN[] = { 0xfc,0x00,0x00,0x00,0x00,0x00 };

uint8_t TRUNCATED[] = { 0xc1,0x05 };

uint8_t OUT_OF_RANGE[] = { 0xe2,0xa1,0x73,0x01,0x85,0x03,0xe9,0x5b,0xa8,0x00,0x83,0x01,0x24,0xf8,0x94,0x09,0x67,0x87,0x41,0xbd,0x50,0xc3,0xe7,0x43,0x01,0xf3,0x8f,0xbd,0x01,0x36,0x30,0x70,0x99,0xae,0x5d };

uint8_t TX_SIGNATURE[] = { 0x34,0xcb,0xcc,0x74,0xe1,0x74,0xb8,0x08,0x18,0xc5,0xd6,0xb1,0xa9,0xf5,0xd9,0x07,0x91,0x5a,0xf3,0x93,0xcb,0x56,0xde,0x1d,0xd4,0x8c,0x44,0x58,0xb9,0x41,0x7a,0x73,0x5e,0x76,0xfc,0x3c,0x26,0x98,0xe7,0xe8,0x4d,0x9e,0x2f,0xd0,0xea,0xc9,0xa9,0xca,0x3b,0xc7,0x89,0x5b,0x60,0xd0,0x07,0xd1,0xb1,0x2d,0xf3,0x30,0x84,0xb7,0x0b,0xb0 };

void kekkac256(const uint8_t* data, int len, uint8_t hash[KEKKAC_256_LEN]) {
  if (len == TX1_LEN) {
    memcpy(hash, TX1_HASH, KEKKAC_256_LEN);
  } else if (len == TX2_LEN) {
    memcpy(hash, TX2_HASH, KEKKAC_256_LEN);
  }
}

int ecdsa(const uint8_t* privkey, const uint8_t hash[KEKKAC_256_LEN], uint8_t *recid, uint8_t signature[ECDSA_256_SIG_LEN]) {
  if (hash[0] == 0xa7) {
    *recid = 0;
  } else if (hash[0] == 0x31) {
    *recid = 1;
  }

  memcpy(signature, TX_SIGNATURE, ECDSA_256_SIG_LEN);
  return 1;
}

void test_rlp_parse(void) {
  uint8_t *value;
  uint8_t *next;
  int len;

  // List with length on same byte
  len = rlp_parse(TX1, &value, &next, &TX1[TX1_LEN]);

  TEST_CHECK(len == (TX1_LEN - 1));
  TEST_CHECK(value == &TX1[1]);
  TEST_CHECK(next == NULL);

  // nonce (string on one byte)
  len = rlp_parse(value, &value, &next, &TX1[TX1_LEN]);

  TEST_CHECK(len == 1);
  TEST_CHECK(value == &TX1[1]);
  TEST_CHECK(next == &TX1[2]);

  // gas price (length on first byte)
  len = rlp_parse(next, &value, &next, &TX1[TX1_LEN]);

  TEST_CHECK(len == 5);
  TEST_CHECK(value == &TX1[3]);
  TEST_CHECK(next == &TX1[8]);

  // skip fields with same structure as previous ones
  rlp_parse(next, &value, &next, &TX1[TX1_LEN]); // gas limit
  rlp_parse(next, &value, &next, &TX1[TX1_LEN]); // address
  rlp_parse(next, &value, &next, &TX1[TX1_LEN]); // value

  // empty field (data)
  len = rlp_parse(next, &value, &next, &TX1[TX1_LEN]);

  TEST_CHECK(len == 0);
  TEST_CHECK(value == next);
  TEST_CHECK(next == &TX1[TX1_LEN - 3]);

  // skip to the end
  rlp_parse(next, &value, &next, &TX1[TX1_LEN]); // v
  rlp_parse(next, &value, &next, &TX1[TX1_LEN]); // r
  rlp_parse(next, &value, &next, &TX1[TX1_LEN]); // s

  TEST_CHECK(value == &TX1[TX1_LEN]);
  TEST_CHECK(next == NULL);

  // List with length of length
  len = rlp_parse(TX2, &value, &next, &TX2[TX2_LEN]);

  TEST_CHECK(len == (TX2_LEN - 3));
  TEST_CHECK(value == &TX2[3]);
  TEST_CHECK(next == NULL);

  // skip to data
  rlp_parse(value, &value, &next, &TX2[TX2_LEN]); // nonce
  rlp_parse(next, &value, &next, &TX2[TX2_LEN]); // gas price
  rlp_parse(next, &value, &next, &TX2[TX2_LEN]); // gas limit
  rlp_parse(next, &value, &next, &TX2[TX2_LEN]); // gas address
  rlp_parse(next, &value, &next, &TX2[TX2_LEN]); // value

  // data with length of length
  len = rlp_parse(next, &value, &next, &TX2[TX2_LEN]);

  TEST_CHECK(len == 324);
  TEST_CHECK(value == &TX2[47]);
  TEST_CHECK(next == &TX2[371]);

  // Check boundaries
  len = rlp_parse(TX1, &value, &next, &TX1[TX1_LEN-1]);
  TEST_CHECK(len == -1);
  TEST_CHECK(value == NULL);
  TEST_CHECK(next == NULL);

  // Reject length on more than 4 bytes
  len = rlp_parse(UNSUPPORTED_LEN, &value, &next, &UNSUPPORTED_LEN[6]);
  TEST_CHECK(len == -1);
  TEST_CHECK(value == NULL);
  TEST_CHECK(next == NULL);
}

void test_eth_parse(void) {
  eth_tx_t tx;
  tx.buffer = TX2;
  tx.barrier = &TX2[TX2_LEN];

  TEST_CHECK(eth_parse(&tx) == 0);

  TEST_CHECK(tx.list_len == 0x173);
  TEST_CHECK(tx.list == &TX2[3]);

  TEST_CHECK(tx.dst_addr == &TX2[15]);

  TEST_CHECK(tx.value_len == 8);
  TEST_CHECK(tx.value == &TX2[36]);

  TEST_CHECK(tx.data_len == 324);
  TEST_CHECK(tx.data == &TX2[47]);

  TEST_CHECK(tx.v == &TX2[371]);
  TEST_CHECK(tx.r == &TX2[373]);
  TEST_CHECK(tx.s == &TX2[374]);

  TEST_CHECK(tx.is_valid == true);
  TEST_CHECK(tx.is_signed == false);

  // Invalid RLP encoding
  memset(&tx, 0, sizeof(tx));
  tx.buffer = UNSUPPORTED_LEN;
  tx.barrier = UNSUPPORTED_LEN + 6;

  TEST_CHECK(eth_parse(&tx) == -1);

  TEST_CHECK(tx.is_valid == false);

  // TX only contains nonce
  memset(&tx, 0, sizeof(tx));
  tx.buffer = TRUNCATED;
  tx.barrier = TRUNCATED + 2;

  TEST_CHECK(eth_parse(&tx) == -1);

  TEST_CHECK(tx.is_valid == false);

  // The nonce is 33 bytes (maximum is 32)
  memset(&tx, 0, sizeof(tx));
  tx.buffer = OUT_OF_RANGE;
  tx.barrier = OUT_OF_RANGE + 35;

  eth_parse(&tx);

  TEST_CHECK(tx.is_valid == false);
}

void test_eth_sign(void) {
  eth_tx_t tx;
  tx.buffer = TX1;
  tx.barrier = &TX1[TX1_LEN];

  eth_parse(&tx);
  TEST_CHECK(eth_sign(&tx, TX1_HASH) == 0); // the private key is not used so anything will do

  TEST_CHECK(tx.buffer == TX1_BUF);
  TEST_CHECK(tx.barrier == TX1 + TX1_LEN + 64);
  TEST_CHECK(tx.list_len == TX1_LEN + 63);
  TEST_CHECK(tx.v[0] == 38);
  TEST_CHECK(tx.r[-1] == 0xa0);
  TEST_CHECK(tx.r[0] == 0x34);
  TEST_CHECK(tx.s[-1] == 0xa0);
  TEST_CHECK(tx.s[0] == 0x5e);
  TEST_CHECK(tx.is_signed == true);

  memset(&tx, 0, sizeof(tx));
  tx.buffer = TX2;
  tx.barrier = &TX2[TX2_LEN];

  eth_parse(&tx);
  TEST_CHECK(eth_sign(&tx, TX2_HASH) == 0); // the private key is not used so anything will do

  TEST_CHECK(tx.buffer == TX2);
  TEST_CHECK(tx.barrier == TX2 + TX2_LEN + 64);
  TEST_CHECK(tx.list_len == TX2_LEN + 61);
  TEST_CHECK(tx.v[0] == 37);
  TEST_CHECK(tx.r[-1] == 0xa0);
  TEST_CHECK(tx.r[0] == 0x34);
  TEST_CHECK(tx.s[-1] == 0xa0);
  TEST_CHECK(tx.s[0] == 0x5e);
  TEST_CHECK(tx.is_signed == true);
}

TEST_LIST = {
   { "rlp_parse", test_rlp_parse },
   { "eth_parse", test_eth_parse },
   { "eth_sign", test_eth_sign },
   { 0 }
};