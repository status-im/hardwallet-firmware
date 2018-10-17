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

#include <string.h>

#include "storage/pin.h"
#include "storage/fs.h"
#include "util/crypto.h"
#include "util/handy.h"

#define PIN_MAX_RETRIES 5
#define PIN_RETRY_COUNTER_MASK 0xf

#define PIN_ENTRY_SIZE 8
#define PIN_COUNTER_ENTRY_SIZE 2

#define PIN_ENC_KEY_IDX 504

static int pin_verified = 0;

static uint8_t* _pin_enc_key() {
  uint32_t* page = fs_get_page(FS_WRITE_ONCE_PAGE, 0);
  return (uint8_t*)(&page[PIN_ENC_KEY_IDX]);
}

static int _pin_encrypt(const uint8_t iv[AES_BLOCK_SIZE], const uint8_t* pin, uint8_t out[PIN_ENTRY_SIZE*4]) {
  if (pin[0] < PIN_MIN_LEN || pin[0] > PIN_MAX_LEN) return -1;
  int pin_data_len = (pin[0] + 1);
  memcpy(out, pin, pin_data_len);
  memset(&out[pin_data_len], 0, (AES_BLOCK_SIZE - pin_data_len));
  aes128_cbc_enc(_pin_enc_key(), iv, out, 1, &out[AES_BLOCK_SIZE]);
  memcpy(out, iv, AES_BLOCK_SIZE);

  return 0;
}

int _pin_set(uint8_t* pin) {
  uint32_t enc_pin[PIN_ENTRY_SIZE];
  uint32_t iv[AES_BLOCK_SIZE/4];
  if (!rng((uint8_t*) iv, AES_BLOCK_SIZE)) return -1;
  if (_pin_encrypt((uint8_t*) iv, pin, (uint8_t*) enc_pin)) return -1;
  return fs_replace_entry(FS_PIN_DATA_PAGE, FS_PIN_DATA_COUNT, PIN_ENTRY_SIZE, enc_pin);
}

int pin_set(uint8_t* pin) {
  if (pin_is_set()) return -1;

  if (fs_find_last_entry(FS_COUNTERS_PAGE, FS_COUNTERS_COUNT, PIN_COUNTER_ENTRY_SIZE) == NULL) {
    uint32_t counters[PIN_COUNTER_ENTRY_SIZE] = { 0, 0 };
    fs_replace_entry(FS_COUNTERS_PAGE, FS_COUNTERS_COUNT, PIN_COUNTER_ENTRY_SIZE, counters);
  }

  return _pin_set(pin);
}

inline int pin_is_set() {
  return fs_find_last_entry(FS_PIN_DATA_PAGE, FS_PIN_DATA_COUNT, PIN_ENTRY_SIZE) != NULL;
}

int pin_change(uint8_t* new_pin) {
  if (!pin_is_verified()) return -1;
  return (_pin_set(new_pin) == -1) ? -1 : 1;
}

static int _pin_remaining_tries(uint32_t *out) {
  uint32_t* counters = fs_find_last_entry(FS_COUNTERS_PAGE, FS_COUNTERS_COUNT, PIN_COUNTER_ENTRY_SIZE);

  if (counters) {
    if (out) {
      out[0] = counters[0];
      out[1] = counters[1];
    }

    return PIN_MAX_RETRIES - (counters[1] & PIN_RETRY_COUNTER_MASK);
  } else {
    return -1;
  }
}

int pin_remaining_tries() {
  return _pin_remaining_tries(NULL);
}

int pin_verify(uint8_t* pin) {
  pin_verified = 0;

  uint32_t counters[PIN_COUNTER_ENTRY_SIZE];

  uint32_t* actual_pin = fs_find_last_entry(FS_PIN_DATA_PAGE, FS_PIN_DATA_COUNT, PIN_ENTRY_SIZE);
  if (!actual_pin) return -1;

  int tries = _pin_remaining_tries(counters);
  if (tries <= 0) return 0;

  uint32_t* enc_pin[PIN_ENTRY_SIZE];
  _pin_encrypt((uint8_t*) actual_pin, pin, (uint8_t*) enc_pin);

  if (mem_eq(actual_pin, enc_pin, PIN_ENTRY_SIZE*4)) {
    if (counters[1] & PIN_RETRY_COUNTER_MASK) {
      counters[1] &= ~PIN_RETRY_COUNTER_MASK;
      fs_replace_entry(FS_COUNTERS_PAGE, FS_COUNTERS_COUNT, PIN_COUNTER_ENTRY_SIZE, counters);
    }

    pin_verified = 1;

    return 1;
  } else {
    counters[1]++;
    fs_replace_entry(FS_COUNTERS_PAGE, FS_COUNTERS_COUNT, PIN_COUNTER_ENTRY_SIZE, counters);
    return 0;
  }
}

int pin_is_verified() {
  return pin_verified;
}

void pin_unverify() {
  pin_verified = 0;
}

