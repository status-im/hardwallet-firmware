/**
 * This file is part of the Status project, https://status.im/
 *
 * Copyright (c) 2018 Status Research & Development GmbH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "ethtx.h"
#include "crypto.h"
#include <string.h>

#define _RLP_WRONG_LEN(len, min, max) ((len < min || len > max))

#define RLP_ASSERT_NEXT(len, min, max, next) if (_RLP_WRONG_LEN(len, min, max) || next == NULL) return -1;
#define RLP_ASSERT_LAST(len, min, max, next) if (_RLP_WRONG_LEN(len, min, max) || next != NULL) return -1;

static inline int rlp_1_byte_length(uint8_t *item, uint8_t **value) {
  *value = &item[1];
  return item[0] & 0x3f;
}

// length can be up to 4 bytes
static inline int rlp_x_bytes_length(uint8_t *item, uint8_t **value) {
  switch(item[0] & 0x3f) {
    case 0x38:
      *value = &item[2];
      return item[1];
    case 0x39:
      *value = &item[3];
      return (item[1] << 8) | item[2];
    case 0x3a:
      *value = &item[4];
      return (item[1] << 16) | (item[2] << 8) | item[3];
    case 0x3b:
      *value = &item[5];
      return (item[1] << 24) | (item[2] << 16) | (item[3] << 8) | item[4];
    default:
      *value = NULL;
      return -1;
  }
}

int rlp_parse(uint8_t *item, uint8_t **value, uint8_t **next, uint8_t *barrier) {
  int len;

  if (item[0] < 0x80) {
    *value = item;
    len = 1;
  } else if (item[0] < 0xb8) {
    len = rlp_1_byte_length(item, value);
  } else if (item[0] < 0xc0) {
    len = rlp_x_bytes_length(item, value);
  } else if (item[0] < 0xf8) {
    len = rlp_1_byte_length(item, value);
  } else {
    len = rlp_x_bytes_length(item, value);
  }

  if (len > -1) {
    *next = *value + len;
  } else {
    *next = NULL;
  }

  if ((*next > barrier) || (*value > *next)) {
    *value = NULL;
    *next = NULL;
    len = -1;
  } else if (*next == barrier) {
    *next = NULL;
  }

  return len;
}

int eth_parse(EthTx *tx) {
  if (tx->buffer == NULL || tx->barrier <= tx->buffer) {
    return -1;
  }

  tx->is_signed = false;
  tx->is_valid = false;

  uint8_t *value;
  uint8_t *next;
  int len;

  // Enter the list
  tx->list_len = rlp_parse(tx->buffer, &tx->list, &next, tx->barrier);
  RLP_ASSERT_LAST(tx->list_len, 0, 0x7fffffff, next);

  // Skip the nonce
  len = rlp_parse(tx->list, &value, &next, tx->barrier);
  RLP_ASSERT_NEXT(len, 0, 32, next);

  // Skip the gas_price
  len = rlp_parse(next, &value, &next, tx->barrier);
  RLP_ASSERT_NEXT(len, 0, 32, next);

  // Skip the gas_limit
  len = rlp_parse(next, &value, &next, tx->barrier);
  RLP_ASSERT_NEXT(len, 0, 32, next);

  // Read the address
  len = rlp_parse(next, &tx->dst_addr, &next, tx->barrier);
  RLP_ASSERT_NEXT(len, 20, 20, next);

  // Read the value
  tx->value_len = rlp_parse(next, &tx->value, &next, tx->barrier);
  RLP_ASSERT_NEXT(tx->value_len, 0, 32, next);

  // Read the data
  tx->data_len = rlp_parse(next, &tx->data, &next, tx->barrier);
  RLP_ASSERT_NEXT(tx->data_len, 0, 0x7fffffff, next);

  // Read v
  len = rlp_parse(next, &tx->v, &next, tx->barrier);
  RLP_ASSERT_NEXT(len, 1, 1, next);

  // Read r
  len = rlp_parse(next, &tx->r, &next, tx->barrier);
  RLP_ASSERT_NEXT(len, 0, 0, next);

  // Read s
  len = rlp_parse(next, &tx->s, &next, tx->barrier);
  RLP_ASSERT_LAST(len, 0, 0, next);

  tx->is_valid = true;

  return 0;
}

static int rlp_len_of_len(int len) {
  if (len < 56) {
    return 1;
  } else if (len < 0x100) {
    return 2;
  } else if (len < 0x10000) {
    return 3;
  } else if (len < 0x1000000) {
    return 4;
  } else if (len < 0x80000000) {
    return 5;
  }

  return -1;
}

static void rlp_write_list_len(uint8_t *buf, int len) {
  if (len < 56) {
    buf[0] = 0xc0 | len;
  } else if (len < 0x100) {
    buf[0] = 0xf8;
    buf[1] = len;
  } else if (len < 0x10000) {
    buf[0] = 0xf9;
    buf[1] = len >> 8;
    buf[2] = len & 0xff;
  } else if (len < 0x1000000) {
    buf[0] = 0xfa;
    buf[1] = len >> 16;
    buf[2] = ((len >> 8) & 0xff);
    buf[3] = len & 0xff;
  } else if (len < 0x80000000) {
    buf[0] = 0xfb;
    buf[1] = len >> 24;
    buf[2] = ((len >> 16) & 0xff);
    buf[3] = ((len >> 8) & 0xff);
    buf[4] = len & 0xff;
  }
}

int eth_sign(EthTx *tx, const uint8_t *privkey) {
  if (!tx->is_valid || tx->is_signed) return -1;

  int tx_len = tx->barrier - tx->buffer;
  uint8_t hash[KEKKAC_256_LEN];
  kekkac256(tx->buffer, tx_len, hash);

  uint8_t recid;

  uint8_t sig[ECDSA_256_SIG_LEN];

  if (!ecdsa(privkey, hash, &recid, sig)) return -1;

  tx->v[0] = (tx->v[0] * 2) + 35 + recid;

  tx->r[0] = 0xa0;
  tx->r++;
  memcpy(tx->r, sig, ECDSA_256_SIG_LEN/2);

  tx->s = tx->r + (ECDSA_256_SIG_LEN/2);
  tx->s[0] = 0xa0;
  tx->s++;

  memcpy(tx->s, &sig[ECDSA_256_SIG_LEN/2], ECDSA_256_SIG_LEN/2);

  tx->barrier += ECDSA_256_SIG_LEN;
  tx->list_len += ECDSA_256_SIG_LEN;

  int oldlenlen = (tx->list - tx->buffer);
  int lenlen = rlp_len_of_len(tx->list_len + ECDSA_256_SIG_LEN);

  if (lenlen == (oldlenlen + 1)) {
    tx->buffer--;
  } else if (lenlen != oldlenlen) {
    return -1;
  }

  rlp_write_list_len(tx->buffer, tx->list_len);

  tx->is_signed = true;

  return 0;
}
