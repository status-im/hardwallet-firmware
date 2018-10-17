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

#ifndef BIP32_H_
#define BIP32_H_

#include <stdint.h>

#define BIP32_KEY_COMPONENT_LEN 32

typedef struct {
  uint8_t key[BIP32_KEY_COMPONENT_LEN];
  uint8_t chain[BIP32_KEY_COMPONENT_LEN];
} bip32_priv_key_t;

// Padding is needed for alignment reasons. We store keys in compressed format for storage efficiency and because we do not need to verify signatures.
typedef struct {
  uint8_t padding[2];
  uint8_t has_full_y;
  uint8_t y_comp;
  uint8_t x[BIP32_KEY_COMPONENT_LEN];
  uint8_t y[BIP32_KEY_COMPONENT_LEN];
} bip32_pub_key_t;

/**
 * Performs the Private CKD algorithm defined in the BIP32 specs. Caching is not performed at this place and must be handled externally.
 */
int bip32_ckd_private(uint32_t i, const bip32_priv_key_t* priv_key, const bip32_pub_key_t* pub_key, bip32_priv_key_t* out_priv, bip32_pub_key_t* out_pub);

/**
 * Generates the private and public keys from the given binary seed.
 */
void bip32_master_key(const uint8_t* seed, int seed_len, bip32_priv_key_t* out_priv, bip32_pub_key_t* out_pub);

#endif /* BIP32_H_ */
