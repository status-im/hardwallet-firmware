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

#include <stddef.h>
#include "rlp.h"

static inline int rlp_1_byte_length(const uint8_t* item, uint8_t** value) {
  *value = (uint8_t*) &item[1];
  return item[0] & 0x3f;
}

// length can be up to 4 bytes
static inline int rlp_x_bytes_length(const uint8_t* item, uint8_t** value) {
  switch(item[0] & 0x3f) {
    case 0x38:
      *value = (uint8_t*) &item[2];
      return item[1];
    case 0x39:
      *value = (uint8_t*) &item[3];
      return (item[1] << 8) | item[2];
    case 0x3a:
      *value = (uint8_t*) &item[4];
      return (item[1] << 16) | (item[2] << 8) | item[3];
    case 0x3b:
      *value = (uint8_t*) &item[5];
      return (item[1] << 24) | (item[2] << 16) | (item[3] << 8) | item[4];
    default:
      *value = NULL;
      return -1;
  }
}

int rlp_parse(const uint8_t* item, uint8_t** value, uint8_t** next, const uint8_t* barrier) {
  int len;

  if (item[0] < 0x80) {
    *value = (uint8_t*) item;
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

int rlp_read_uint32(const uint8_t* item, uint32_t* value, uint8_t** next, const uint8_t* barrier) {
  uint8_t* v;

  int len = rlp_parse(item, &v, next, barrier);

  switch(len) {
    case -1:
      return -1;
    case 0:
      *value = 0;
      break;
    case 1:
      *value = v[0];
      break;
    case 2:
      *value = (v[0] << 8 | v[1]);
      break;
    case 3:
      *value = (v[0] << 16 | (v[1] << 8) | v[2]);
      break;
    case 4:
      *value = (v[0] << 24 | (v[1] << 16) | (v[2] << 8) | v[3]);
      break;
  }

  return 0;
}

int rlp_len_of_len(int len) {
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

void rlp_write_len(uint8_t* buf, int len, int is_list) {
  uint8_t prefix;

  if (is_list) {
    prefix = 0xc0;
  } else {
    prefix = 0x80;
  }

  if (len < 56) {
    buf[0] = prefix | len;
  } else if (len < 0x100) {
    buf[0] = (prefix | 0x38);
    buf[1] = len;
  } else if (len < 0x10000) {
    buf[0] = (prefix | 0x39);
    buf[1] = len >> 8;
    buf[2] = len & 0xff;
  } else if (len < 0x1000000) {
    buf[0] = (prefix | 0x3A);
    buf[1] = len >> 16;
    buf[2] = ((len >> 8) & 0xff);
    buf[3] = len & 0xff;
  } else if (len < 0x80000000) {
    buf[0] = (prefix | 0x3B);
    buf[1] = len >> 24;
    buf[2] = ((len >> 16) & 0xff);
    buf[3] = ((len >> 8) & 0xff);
    buf[4] = len & 0xff;
  }
}
