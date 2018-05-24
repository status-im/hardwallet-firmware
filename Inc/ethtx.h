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

#ifndef ETHTX_H_
#define ETHTX_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

struct {
  uint8_t *buffer;
  uint8_t *barrier;

  uint8_t *dst_addr;

  int value_len;
  uint8_t *value;

  int data_len;
  uint8_t *data;

  uint8_t *v;
  uint8_t *r;
  uint8_t *s;

  bool is_signed;
  bool is_valid;
} typedef EthTx;

/**
 *  Takes an RLP item, returns its length and stores the pointer to the value and to the next item in the given parameters.
 *  The barrier parameter is the address immediately after the transaction buffer. If parsing produces any address (value or next)
 *  which is larger than the barrier, parsing will fail. This is a protection against malformed data.
 *
 *  On failure this function returns -1 and. Value and next pointer will point to NULL. If the address of next is exactly the barrier, then
 *  the function will not fail but the next pointer will point to NULL, indicating that this was the last field.
 *
 **/
int rlp_parse(uint8_t *item, uint8_t **value, uint8_t **next, uint8_t *barrier);

/**
 * Takes an EthTx structure where the buffer and barrier fields must be populated and fills the other fields by parsing the content of the buffer. On error, returns -1, on success returns 0.
 *
 * Only accepts unsigned transactions, because this is what we want to use it for.
 */
int eth_parse(EthTx *tx);

#endif /* ETHTX_H_ */
