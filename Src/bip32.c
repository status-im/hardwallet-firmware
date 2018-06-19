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

#include "bip32.h"
#include "crypto.h"
#include "bignum256.h"
#include <string.h>

int bip32_ckd_private(uint32_t i, const bip32_priv_key_t* priv_key, const bip32_pub_key_t* pub_key, bip32_priv_key_t* out_priv, bip32_pub_key_t* out_pub) {
  uint32_t _tmp[(KEY_COMPONENT_LEN * 3) / 4]; // declare for word access
  uint8_t *tmp = (uint8_t *) _tmp;

  if ((i & 0x80000000) != 0) {
    // hardened key
    tmp[0] = 0;
    memcpy(&tmp[1], priv_key->key, KEY_COMPONENT_LEN);
  } else {
    tmp[0] = pub_key->y;
    memcpy(&tmp[1], pub_key->x, KEY_COMPONENT_LEN);
  }

  tmp[KEY_COMPONENT_LEN + 1] = i >> 24;
  tmp[KEY_COMPONENT_LEN + 2] = (i >> 16) & 0xff;
  tmp[KEY_COMPONENT_LEN + 3] = (i >> 8) & 0xff;
  tmp[KEY_COMPONENT_LEN + 4] = i & 0xff;

  hmac_sha512(priv_key->chain, KEY_COMPONENT_LEN, tmp, (1 + KEY_COMPONENT_LEN + 4), (uint8_t *) out_priv);

  bignum256_t *priv_num = (bignum256_t *) tmp;
  bignum256_t *out_num = (bignum256_t *) &tmp[KEY_COMPONENT_LEN];
  bignum256_t *res_num = (bignum256_t *) &tmp[KEY_COMPONENT_LEN * 2];
  bignum256_t *n = bignum256_secp256k1_n();

  bignum256_from_bytes(priv_key->key, priv_num);
  bignum256_from_bytes(out_priv->key, out_num);

  if (bignum256_cmp(out_num, n) >= 0) {
    return -1;
  }

  bignum256_mod_add(priv_num, out_num, n, res_num);

  if (bignum256_is_zero(res_num)) {
    return -1;
  }

  bignum256_to_bytes(res_num, out_priv->key);

  if (out_pub != NULL) {
    // a bit hacky, but priv_num and out_num are sequential in memory so this works
    bignum256_secp256k1_publickey(res_num, priv_num);
    bignum256_to_bytes(priv_num, out_pub->x);
    out_pub->y = 2 + bignum256_read_bit(out_num, 0);
  }

  return 0;
}

