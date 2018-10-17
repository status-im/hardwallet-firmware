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

#ifndef BIP39_H_
#define BIP39_H_

#include <stdint.h>

#define BIP39_SEED_LEN 64
#define BIP39_MAX_MNEMO 24

#define BIP39_WORD_COUNT(cslen) (cslen * 3)
#define BIP39_ENTROPY_LEN(cslen) (cslen * 4)

#define BIP39_MAX_STRLEN (BIP39_MAX_MNEMO * 9)

/**
 * Generates a random sequence of mnemonic according to the BIP39 standard. The cslen parameters is used to determine the number of mnemonic indexes to generate. Returns the number of generates indexes, -1 on error.
 */
int bip39_generate_mnemonic(int cslen, uint16_t mnemonic[]);

/**
 * Returns the mnemonic index for the given word. Returns -1 if not found.
 */
int16_t bip39_find_word(const char* word);

/**
 * Verifies that this sequence of mnemonic is valid according to the BIP39 standard. The cslen parameters is used to determine the number of mnemonic indexes in the array.
 */
int bip39_verify(int cslen, const uint16_t mnemonic[]);

/**
 * Renders the array of indexes to a string of space separated words from which the seed can be generated.
 */
void bip39_render_mnemonic(const uint16_t mnemonic[], int len, uint8_t* mnstr, int* mnlen);

/**
 * Generates a binary seed from the given mnemonic string, protected by an optional passphrase long at most 64 bytes.
 */
int bip39_generate_seed(const uint8_t* mnstr, int mnlen, const uint8_t* passphrase, int pplen, uint8_t seed[BIP39_SEED_LEN]);

#endif /* BIP39_H_ */
