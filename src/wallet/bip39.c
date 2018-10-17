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

#include "wallet/bip39.h"
#include "wallet/bip39_english.h"
#include "util/crypto.h"
#include "util/handy.h"
#include <string.h>

#define BIP39_PBKDF2_ITERATIONS 2048
#define BIP39_MIN_CS_LEN 4
#define BIP39_MAX_CS_LEN 8

int bip39_generate_mnemonic(int cslen, uint16_t mnemonic[]) {
  if(cslen < BIP39_MIN_CS_LEN || cslen > BIP39_MAX_CS_LEN) return -1;
  int entlen = BIP39_ENTROPY_LEN(cslen);
  uint8_t ent[BIP39_ENTROPY_LEN(BIP39_MAX_CS_LEN) + SHA_256_LEN];
  if (!rng(ent, entlen)) return -1;
  sha256(ent, entlen, &ent[entlen]);

  int mlen = BIP39_WORD_COUNT(cslen);
  int i, j, idx;

  for (i = 0; i < mlen; i++) {
    idx = 0;
    for (j = 0; j < 11; j++) {
      idx <<= 1;
      idx += (ent[(i * 11 + j) / 8] & (1 << (7 - ((i * 11 + j) % 8)))) > 0;
    }

    mnemonic[i] = idx;
  }

  mem_clean(ent, sizeof(ent));
  return mlen;
}

int16_t bip39_find_word(const char* word) {
  int16_t low = 0;
  int16_t high = BIP39_WORDLIST_LEN - 1;

  // no mnemonic is shorter than 4 bytes (3 letters + string terminator) and 4 bytes is enough to
  // distinguish a word from another. For this reason, we can compare the words as if they were integers.
  uint32_t x = word[3] | (word[2] << 8) | (word[1] << 16) | (word[0] << 24);

  while(low <= high) {
    int16_t mid = (low + high) / 2;
    uint32_t v = bip39_wordlist[mid][3] | (bip39_wordlist[mid][2] << 8) | (bip39_wordlist[mid][1] << 16) | (bip39_wordlist[mid][0] << 24);

    if (x < v) {
      high = mid - 1;
    } else if (x > v) {
      low = mid + 1;
    } else {
      return mid;
    }
  }

  return -1;
}

int bip39_verify(int cslen, const uint16_t mnemonic[]) {
  if(cslen < BIP39_MIN_CS_LEN || cslen > BIP39_MAX_CS_LEN) return -1;
  uint8_t ent[BIP39_ENTROPY_LEN(BIP39_MAX_CS_LEN) + 1 + SHA_256_LEN];
  int entlen = BIP39_ENTROPY_LEN(cslen);
  int mlen = BIP39_WORD_COUNT(cslen);

  int bi = 0;
  mem_clean(ent, entlen + 1);

  for (int i = 0; i < mlen; i++) {
    uint16_t k = mnemonic[i];
    for (int ki = 0; ki < 11; ki++) {
      if (k & (1 << (10 - ki))) {
        ent[bi / 8] |= 1 << (7 - (bi % 8));
      }

      bi++;
    }
  }

  sha256(ent, entlen, &ent[entlen+1]);

  uint8_t mask = 0xff00 >> cslen;
  return (ent[entlen] & mask) == (ent[entlen + 1] & mask) ? 0 : -1;
}

void bip39_render_mnemonic(const uint16_t mnemonic[], int len, uint8_t* mnstr, int* mnlen) {
  *mnlen = 0;

  for(int i = 0; i < len; i++) {
    const char *word = bip39_wordlist[mnemonic[i]];
    while(*word != '\0') {
      mnstr[(*mnlen)++] = *word++;
    }

    mnstr[(*mnlen)++] = ' ';
  }

  mnstr[--(*mnlen)] = '\0';
}

int bip39_generate_seed(const uint8_t* mnstr, int mnlen, const uint8_t* passphrase, int pplen, uint8_t seed[BIP39_SEED_LEN]) {
  if (pplen > BIP39_SEED_LEN) return -1;

  uint8_t salt[BIP39_SEED_LEN + 8];
  memcpy(salt, "mnemonic", 8);
  memcpy(&salt[8], passphrase, pplen);

  pbkdf2_sha512(mnstr, mnlen, salt, (pplen + 8), BIP39_PBKDF2_ITERATIONS, seed, BIP39_SEED_LEN);

  return 0;
}
