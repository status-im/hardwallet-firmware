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

#include "util/bignum256.h"
#include "uecc/uECC_vli.h"

struct bignum256 {
  uECC_word_t _v[BIGNUM_WORDS];
};

void bignum256_from_bytes(const uint8_t* bytes, bignum256_t* bignum) {
  uECC_vli_bytesToNative(bignum->_v, bytes, BIGNUM_BYTES);
}

void bignum256_to_bytes(const bignum256_t* bignum, uint8_t* bytes) {
  uECC_vli_nativeToBytes(bytes, BIGNUM_BYTES, bignum->_v);
}

void bignum256_mod_add(const bignum256_t* a, const bignum256_t* b, const bignum256_t* mod, bignum256_t* c) {
  uECC_vli_modAdd(c->_v, a->_v, b->_v, mod->_v, BIGNUM_WORDS);
}

int bignum256_is_zero(const bignum256_t* n) {
  return uECC_vli_isZero(n->_v, BIGNUM_WORDS);
}

int bignum256_read_bit(const bignum256_t* n, int bit) {
  return !!uECC_vli_testBit(n->_v, bit);
}

int bignum256_cmp(const bignum256_t* a, const bignum256_t* b) {
  return uECC_vli_cmp(a->_v, b->_v, BIGNUM_WORDS);
}

bignum256_t* bignum256_secp256k1_n() {
  uECC_Curve curve = uECC_secp256k1();
  return (bignum256_t *) uECC_curve_n(curve);
}

void bignum256_secp256k1_publickey(const bignum256_t* priv_key, bignum256_t out_pub[2]) {
  uECC_Curve curve = uECC_secp256k1();
  uECC_point_mult((uECC_word_t *)out_pub, uECC_curve_G(curve), priv_key->_v, curve);
}

