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

#include "util/crypto.h"
#include "util/handy.h"
#include "uecc/uECC.h"

#include <string.h>

#ifdef NRF52840_XXAA
  #include "sdk_alloca.h"
#endif

#define MAX_SIGN_RETRY 5

void pbkdf2_sha512(const uint8_t *pw, int pwlen, const uint8_t *salt, int saltlen, uint32_t iterations, uint8_t *out, int outlen) {
  uint8_t *asalt = alloca(saltlen + 4);
  uint8_t d1[SHA_512_LEN], d2[SHA_512_LEN], obuf[SHA_512_LEN];

  memcpy(asalt, salt, saltlen);

  for (int count = 1; outlen > 0; count++) {
    asalt[saltlen + 0] = (count >> 24) & 0xff;
    asalt[saltlen + 1] = (count >> 16) & 0xff;
    asalt[saltlen + 2] = (count >> 8) & 0xff;
    asalt[saltlen + 3] = count & 0xff;
    
    hmac_sha512(pw, pwlen, asalt, (saltlen + 4), d1);
    memcpy(obuf, d1, SHA_512_LEN);
    
    for (int i = 1; i < iterations; i++) {
      hmac_sha512(pw, pwlen, d1, SHA_512_LEN, d2);
      memcpy(d1, d2, SHA_512_LEN);
      
      for (int j = 0; j < SHA_512_LEN; j++) {
        obuf[j] ^= d1[j];
      }
    }

    int r = MIN(outlen, SHA_512_LEN);
    memcpy(out, obuf, r);
    out += r;
    outlen -= r;
  }
}

int ecdsa(const uint8_t* privkey, const uint8_t hash[KEKKAC_256_LEN], uint8_t* recid, uint8_t signature[ECDSA_256_SIG_LEN]) {
  uECC_set_rng(&rng);
  uECC_Curve curve = uECC_secp256k1();

  for (int i = 0; i < MAX_SIGN_RETRY; i++) {
    if (uECC_sign(privkey, hash, KEKKAC_256_LEN, signature, recid, curve)) {
      return 1;
    }
  }

  return 0;
}


