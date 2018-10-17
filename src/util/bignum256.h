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

#ifndef BIGNUM256_H_
#define BIGNUM256_H_

#define BIGNUM_BYTES 32
#define BIGNUM_WORDS BIGNUM_BYTES/sizeof(int)

#include <stdint.h>

// This structure is opaque because the underlying implementation is subject to change. The functions here are a wrapper around
// an external library and they are needed to keep a stable interface when the underlying API/implementation changes. In particular
// if EC hardware acceleration is available, we might take advantage of that to speed up calculations.

typedef struct bignum256 bignum256_t;

/**
 * Converts a byte array to a 256-bit big number. The data is interpreted as MSB first.
 */
void bignum256_from_bytes(const uint8_t* bytes, bignum256_t* bignum);

/**
 * Converts a 256-bit big number to a byte array. The output is encoded MSB first.
 */
void bignum256_to_bytes(const bignum256_t* bignum, uint8_t* bytes);

/**
 * c =  (a + b) % mod
 */
void bignum256_mod_add(const bignum256_t* a, const bignum256_t* b, const bignum256_t* mod, bignum256_t* c);

/**
 * n == 0
 */
int bignum256_is_zero(const bignum256_t* n);

/**
 * Compares a with b. Returns the sign of (a - b)
 */
int bignum256_cmp(const bignum256_t* a, const bignum256_t* b);

/**
 * Reads the bit at the given position from n. Returns 1 if the bit was set, 0 otherwise.
 */
int bignum256_read_bit(const bignum256_t* n, int bit);

/**
 * Returns the modulo of the secp256k1 curve.
 */
bignum256_t* bignum256_secp256k1_n();

/**
 * Calculates the public key for the given private key on the secp256k1 curve.
 */
void bignum256_secp256k1_publickey(const bignum256_t* priv_key, bignum256_t* out_pub);

#endif /* BIGNUM256_H_ */
