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

#ifndef ETH_H_
#define ETH_H_

#include <stdint.h>
#include <stdbool.h>

struct {
  uint8_t* buffer;
  const uint8_t* barrier;

  int list_len;
  uint8_t* list;

  uint8_t* dst_addr;

  int value_len;
  uint8_t* value;

  int data_len;
  uint8_t* data;

  uint8_t* v;
  uint8_t* r;
  uint8_t* s;

  bool is_signed;
  bool is_valid;
  int erc20_idx;
} typedef eth_tx_t;

/**
 * Takes an EthTx structure where the buffer and barrier fields must be populated and fills the other fields by parsing the content of the buffer.
 * On error, returns -1, on success returns 0.
 *
 * Only accepts unsigned transactions, where v is the chain id (for now on 1 byte only) and r and s are empty as per EIP-155
 */
int eth_parse(eth_tx_t* tx);

/**
 * Signs a previously parsed transaction. The transaction is modified in place.
 * It must be possible to decrease the buffer pointer by 1, thus prepending 1 additional byte. The reason for this is that the overall
 * length of the RLP list will increase and its encoded representation might be larger by one byte
 *
 * Additionally, it must be possible to increase the barrier pointer by 64 bytes. This will always happen because the
 * the empty r and s component are substituted by the signature.
 */
int eth_sign(eth_tx_t* tx, const uint8_t* priv_key);

#endif /* ETH_H_ */
