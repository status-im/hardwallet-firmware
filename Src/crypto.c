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

#include "crypto.h"
#include "cifra/sha3.h"
#include "uecc/uECC.h"
//#include "stm32l4xx.h"

#define MAX_SIGN_RETRY 5

void kekkac256(uint8_t* data, int len, uint8_t hash[KEKKAC_256_LEN]) {
  cf_sha3_context ctx;
  cf_sha3_256_init(&ctx);
  cf_sha3_256_update(&ctx, data, len);
  cf_sha3_256_digest_final(&ctx, hash);
}

int ecdsa(uint8_t* privkey, uint8_t hash[KEKKAC_256_LEN], uint8_t *recid, uint8_t signature[ECDSA_256_SIG_LEN]) {
  uECC_set_rng(&rng);
  uECC_Curve curve = uECC_secp256k1();

  for (int i = 0; i < MAX_SIGN_RETRY; i++) {
    if (uECC_sign(privkey, hash, KEKKAC_256_LEN, signature, recid, curve)) {
      return 1;
    }
  }

  return 0;
}

int rng(uint8_t *dst, unsigned int size) {
  // TODO: implement this
  return 0;
}
