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
#include "cifra/sha2.h"
#include "cifra/sha3.h"
#include "cifra/hmac.h"
#include "cifra/pbkdf2.h"
#include "cifra/aes.h"
#include "cifra/modes.h"
#include "uecc/uECC.h"

#if defined(__arm__)
#include "stm32l4xx.h"
#endif

#define MAX_SIGN_RETRY 5

void kekkac256(const uint8_t* data, int len, uint8_t hash[KEKKAC_256_LEN]) {
  cf_sha3_context ctx;
  cf_sha3_256_init(&ctx);
  cf_sha3_256_update(&ctx, data, len);
  cf_sha3_256_digest_final(&ctx, hash);
}

void sha256(const uint8_t* data, int len, uint8_t hash[SHA_256_LEN]) {
  cf_sha256_context ctx;
  cf_sha256_init(&ctx);
  cf_sha256_update(&ctx, data, len);
  cf_sha256_digest_final(&ctx, hash);
}

void hmac_sha512(const uint8_t* key, int keylen, const uint8_t* data, int len, uint8_t hash[SHA_512_LEN]) {
  cf_hmac(key, keylen, data, len, hash, &cf_sha512);
}

void pbkdf2_sha512(const uint8_t *pw, int pwlen, const uint8_t *salt, int saltlen, uint32_t iterations, uint8_t *out, int outlen) {
  cf_pbkdf2_hmac(pw, pwlen, salt, saltlen, iterations, out, outlen, &cf_sha512);
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

int _rng(uint32_t* out) {
  do {
    if (RNG->SR & (RNG_SR_SECS|RNG_SR_CECS)) {
      RNG->SR = 0;
      return 0;
    } else if (READ_BIT(RNG->SR, RNG_SR_DRDY)) {
      *out = RNG->DR;
      return 1;
    }
  } while(1);
}

int rng(uint8_t* dst, unsigned int size) {
  SET_BIT(RNG->CR, RNG_CR_RNGEN);

  int word_count = size / 4;
  int rest = size % 4;
  int res = 0;

  uint32_t *wdst = (uint32_t *)wdst;
  for(int i = 0; i < word_count; i++) {
    if(!_rng(&wdst[i])) {
      goto end;
    }
  }

  uint32_t last;
  if(!_rng(&last)) {
    goto end;
  }

  switch(rest) {
  case 3:
    dst[size - 3] = last >> 24;
    /* no break */
  case 2:
    dst[size - 2] = (last >> 16) & 0xff;
    /* no break */
  case 1:
    dst[size - 1] = (last >> 8) & 0xff;
    /* no break */
  default:
    break;
  }

  res = 1;

  end:
  CLEAR_BIT(RNG->CR, RNG_CR_RNGEN);
  return res;
}

void _aes_cbc_enc(const uint8_t* key, int keylen, const uint8_t iv[AES_BLOCK_SIZE], const uint8_t* data, int blocks, uint8_t* out) {
  cf_cbc cbc_ctx;
  cf_aes_context aes_ctx;

  cf_aes_init(&aes_ctx, key, keylen);
  cf_cbc_init(&cbc_ctx, &cf_aes, &aes_ctx, iv);
  cf_cbc_encrypt(&cbc_ctx, data, out, blocks);
}

void _aes_cbc_dec(const uint8_t* key, int keylen, const uint8_t iv[AES_BLOCK_SIZE], const uint8_t* data, int blocks, uint8_t* out) {
  cf_cbc cbc_ctx;
  cf_aes_context aes_ctx;

  cf_aes_init(&aes_ctx, key, keylen);
  cf_cbc_init(&cbc_ctx, &cf_aes, &aes_ctx, iv);
  cf_cbc_decrypt(&cbc_ctx, data, out, blocks);
}

void aes128_cbc_enc(const uint8_t key[AES_128_KEYLEN], const uint8_t iv[AES_BLOCK_SIZE], const uint8_t* data, int blocks, uint8_t* out) {
  _aes_cbc_enc(key, AES_128_KEYLEN, iv, data, blocks, out);
}

void aes128_cbc_dec(const uint8_t key[AES_128_KEYLEN], const uint8_t iv[AES_BLOCK_SIZE], const uint8_t* data, int blocks, uint8_t* out) {
  _aes_cbc_dec(key, AES_128_KEYLEN, iv, data, blocks, out);
}
