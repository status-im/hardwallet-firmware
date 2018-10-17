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
 #include "nrf_crypto.h"
 #include "app_error.h"

static nrf_crypto_aes_context_t nrf52_aes_ctx;
static nrf_crypto_hash_context_t nrf52_hash_ctx;
static nrf_crypto_hmac_context_t nrf52_hmac_ctx;

int rng(uint8_t* dst, unsigned int size) {
  ret_code_t rc = nrf_crypto_rng_vector_generate(dst, size);
  return (rc == NRF_SUCCESS) ? 1 : 0;
}

void aes128_cbc_enc(const uint8_t key[AES_128_KEYLEN], const uint8_t iv[AES_BLOCK_SIZE], const uint8_t* data, int blocks, uint8_t* out) {
  int data_size = blocks * AES_BLOCK_SIZE;
  ret_code_t rc = nrf_crypto_aes_crypt(&nrf52_aes_ctx, &g_nrf_crypto_aes_cbc_128_info, NRF_CRYPTO_ENCRYPT, (uint8_t *)key, (uint8_t *)iv, (uint8_t *)data, data_size, out, &data_size);
  APP_ERROR_CHECK(rc);
}

void aes128_cbc_dec(const uint8_t key[AES_128_KEYLEN], const uint8_t iv[AES_BLOCK_SIZE], const uint8_t* data, int blocks, uint8_t* out) {
  int data_size = blocks * AES_BLOCK_SIZE;
  ret_code_t rc = nrf_crypto_aes_crypt(&nrf52_aes_ctx, &g_nrf_crypto_aes_cbc_128_info, NRF_CRYPTO_DECRYPT, (uint8_t *)key, (uint8_t *)iv, (uint8_t *)data, data_size, out, &data_size);
  APP_ERROR_CHECK(rc);
}

void sha256(const uint8_t* data, int len, uint8_t hash[SHA_256_LEN]) {
  int hash_size = SHA_256_LEN;
  ret_code_t rc = nrf_crypto_hash_calculate(&nrf52_hash_ctx, &g_nrf_crypto_hash_sha256_info, data, len, hash, &hash_size);
  APP_ERROR_CHECK(rc);
}

void hmac_sha512(const uint8_t* key, int keylen, const uint8_t* data, int len, uint8_t hash[SHA_512_LEN]) {
  int hash_size = SHA_256_LEN;
  ret_code_t rc = nrf_crypto_hmac_calculate(&nrf52_hmac_ctx, &g_nrf_crypto_hmac_sha512_info, hash, &hash_size, key, keylen, data, len);
  APP_ERROR_CHECK(rc);
}