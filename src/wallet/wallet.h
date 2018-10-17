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

#ifndef WALLET_H_
#define WALLET_H_

#include "wallet/bip32.h"

#define WALLET_PATH_LEN 10
#define WALLET_ADDR_LEN 20

/**
 * Creates a new BIP32-compatible wallet starting from the given seed. If a wallet already exist or if an error is encountered when generating the wallet, -1 is returned. Returns 0 on success.
 */
int wallet_new(const uint8_t* seed, int seed_len);

/**
 * Returns 1 if the wallet has been created, 0 otherwise.
 */
int wallet_created();

/**
 * Gets the private key for the child wallet with the given path. Returns 0 on success, -1 on failure. Performs key derivation internally if needed.
 */
int wallet_priv_key(const uint32_t path[WALLET_PATH_LEN], uint8_t out_priv[BIP32_KEY_COMPONENT_LEN]);

/**
 * Gets the address for the child wallet with the given path. Returns 0 on success, -1 on failure. Performs key derivation internally if needed.
 */
int wallet_address(const uint32_t path[WALLET_PATH_LEN], uint8_t out_addr[WALLET_ADDR_LEN]);

/**
 * Gets the address for the master wallet. Returns 0 on success, -1 on failure.
 */
int wallet_master_address(uint8_t out_addr[WALLET_ADDR_LEN]);

#endif /* WALLET_H_ */
