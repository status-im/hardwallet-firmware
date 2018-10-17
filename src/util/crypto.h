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

#ifndef CRYPTO_H_
#define CRYPTO_H_

#include <stdint.h>

#define KEKKAC_256_LEN 32
#define SHA_256_LEN 32
#define SHA_512_LEN 64
#define ECDSA_256_SIG_LEN 64
#define AES_BLOCK_SIZE 16
#define AES_128_KEYLEN 16

// Use the function defined here for cryptography. This is a wrapper around external dependencies and/or device specific code
// All lengths here are in bytes.

/**
 * Calculates the Kekkac-256 hash of the given data and stores it into the hash array.
 */
void kekkac256(const uint8_t* data, int len, uint8_t hash[KEKKAC_256_LEN]);

/**
 * Calculates the SHA-256 hash of the given data and stores it into the hash array.
 */
void sha256(const uint8_t* data, int len, uint8_t hash[SHA_256_LEN]);

/**
 * Calculates the HMAC-SHA-512 hash of the given data with the given key and stores it into the hash array.
 */
void hmac_sha512(const uint8_t* key, int keylen, const uint8_t* data, int len, uint8_t hash[SHA_512_LEN]);

/**
 * Generates a key of the specified length from the given password and salt using the PBKDF2 with SHA512 algorithm. The result is stored in the out array.
 */
void pbkdf2_sha512(const uint8_t *pw, int pwlen, const uint8_t *salt, int saltlen, uint32_t iterations, uint8_t *out, int outlen);

/**
 * Signs the given hash with the given private key. On output it gives the recovery id (to recover the public key) and the signature. Returns
 * 1 on success, 0 on failure. Since random data is involved, it is worth retrying the call on failure.
 */
int ecdsa(const uint8_t* privkey, const uint8_t hash[KEKKAC_256_LEN], uint8_t* recid, uint8_t signature[ECDSA_256_SIG_LEN]);

/**
 * Generates a number of random bytes. Returns 1 on success, 0 otherwise. It is worth retrying the call on failure at least a couple of times.
 *
 * The implementation of this function must use a secure random source. On STM32 chips the embedded RNG hardware is used.
 */
int rng(uint8_t* dst, unsigned int size);

/**
 * AES128-CBC encryption with the given key, iv and data. The length must be given in number of blocks. A block is 16 bytes long.
 */
void aes128_cbc_enc(const uint8_t key[AES_128_KEYLEN], const uint8_t iv[AES_BLOCK_SIZE], const uint8_t* data, int blocks, uint8_t* out);

/**
 * AES128-CBC decryption with the given key, iv and data. The length must be given in number of blocks. A block is 16 bytes long.
 */
void aes128_cbc_dec(const uint8_t key[AES_128_KEYLEN], const uint8_t iv[AES_BLOCK_SIZE], const uint8_t* data, int blocks, uint8_t* out);

#endif /* CRYPTO_H_ */
