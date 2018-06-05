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

#include "bip39.h"
#include "bip39_english.h"
#include "crypto.h"

#define BIP39_PBKDF2_ITERATIONS 2048

int bip39_generate_mnemonic(int cslen, uint16_t* mnemonic) {

}

uint16_t bip39_find_word(uint8_t* word) {

}

int bip39_verify(int cslen, uint16_t* mnemonic) {

}

void bip39_render_mnemonic(uint16_t* mnemonic, int len, uint8_t* mnstr, int* mnlen) {

}

int bip39_generate_seed(uint8_t* mnstr, int mnlen, uint8_t* passphrase, int pplen, uint8_t seed[BIP39_SEED_LEN]) {
  if (pplen > BIP39_SEED_LEN) return -1;

  uint8_t salt[BIP39_SEED_LEN + 8];
  memcpy(salt, "mnemonic", 8);
  memcpy(&salt[8], passphrase, pplen);

  pbkdf2_sha512(mnstr, mnlen, salt, (pplen + 8), BIP39_PBKDF2_ITERATIONS, seed, BIP39_SEED_LEN);

  return 0;
}
