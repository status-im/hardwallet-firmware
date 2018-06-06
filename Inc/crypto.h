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

void kekkac256(const uint8_t* data, int len, uint8_t hash[KEKKAC_256_LEN]);
void sha256(const uint8_t* data, int len, uint8_t hash[SHA_256_LEN]);
void hmac_sha512(const uint8_t* key, int keylen, const uint8_t* data, int len, uint8_t hash[SHA_512_LEN]);
void pbkdf2_sha512(const uint8_t *pw, int pwlen, const uint8_t *salt, int saltlen, uint32_t iterations, uint8_t *out, int outlen);
int ecdsa(const uint8_t* privkey, const uint8_t hash[KEKKAC_256_LEN], uint8_t *recid, uint8_t signature[ECDSA_256_SIG_LEN]);
int rng(uint8_t *dst, unsigned int size);

#endif /* CRYPTO_H_ */
