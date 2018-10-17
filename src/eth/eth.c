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

#include "eth/eth.h"
#include "eth/rlp.h"
#include "util/crypto.h"
#include <string.h>

#define ETH_WRONG_LEN(len, min, max) ((len < min || len > max))

#define ETH_ASSERT_NEXT(len, min, max, next) if (ETH_WRONG_LEN(len, min, max) || next == NULL) return -1;
#define ETH_ASSERT_LAST(len, min, max, next) if (ETH_WRONG_LEN(len, min, max) || next != NULL) return -1;

void eth_erc20_token(eth_tx_t* tx) {
  //TODO: implement this, maybe a user updatable table?
  tx->erc20_idx = 0;
}

int eth_parse(eth_tx_t* tx) {
  if (tx->buffer == NULL || tx->barrier <= tx->buffer) {
    return -1;
  }

  tx->is_signed = false;
  tx->is_valid = false;
  tx->erc20_idx = -1;

  uint8_t* value;
  uint8_t* next;
  int len;

  // Enter the list
  tx->list_len = rlp_parse(tx->buffer, &tx->list, &next, tx->barrier);
  ETH_ASSERT_LAST(tx->list_len, 0, 0x7fffffff, next);

  // Skip the nonce
  len = rlp_parse(tx->list, &value, &next, tx->barrier);
  ETH_ASSERT_NEXT(len, 0, 32, next);

  // Skip the gas_price
  len = rlp_parse(next, &value, &next, tx->barrier);
  ETH_ASSERT_NEXT(len, 0, 32, next);

  // Skip the gas_limit
  len = rlp_parse(next, &value, &next, tx->barrier);
  ETH_ASSERT_NEXT(len, 0, 32, next);

  // Read the address
  len = rlp_parse(next, &tx->dst_addr, &next, tx->barrier);
  ETH_ASSERT_NEXT(len, 20, 20, next);

  // Read the value
  tx->value_len = rlp_parse(next, &tx->value, &next, tx->barrier);
  ETH_ASSERT_NEXT(tx->value_len, 0, 32, next);

  // Read the data
  tx->data_len = rlp_parse(next, &tx->data, &next, tx->barrier);
  ETH_ASSERT_NEXT(tx->data_len, 0, 0x7fffffff, next);

  // Read v
  len = rlp_parse(next, &tx->v, &next, tx->barrier);
  ETH_ASSERT_NEXT(len, 1, 1, next);

  // Read r
  len = rlp_parse(next, &tx->r, &next, tx->barrier);
  ETH_ASSERT_NEXT(len, 0, 0, next);

  // Read s
  len = rlp_parse(next, &tx->s, &next, tx->barrier);
  ETH_ASSERT_LAST(len, 0, 0, next);

  tx->is_valid = true;

  if ((tx->value_len == 0) && (tx->data_len == 68) && !memcmp(tx->data, "\xa9\x05\x9c\xbb\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16)) {
    eth_erc20_token(tx);
  }

  return 0;
}

int eth_sign(eth_tx_t* tx, const uint8_t* privkey) {
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

  rlp_write_len(tx->buffer, tx->list_len, true);

  tx->is_signed = true;

  return 0;
}
