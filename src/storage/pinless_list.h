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

#ifndef PINLESS_LIST_H_
#define PINLESS_LIST_H_

#include "wallet/wallet.h"

/**
 * Returns 1 if the list contains the given path, 0 otherwise.
 */
int pinless_list_contains(const uint32_t path[WALLET_PATH_LEN]);

/**
 * Adds an entry to the list. If the list is full, -2 is returned, on any other error -1 is returned. Returns 1 if the item is added and 0 if it was already there.
 */
int pinless_list_add(const uint32_t path[WALLET_PATH_LEN]);

/**
 * Removes an entry from the list. On error -1 is returned. Returns 1 if the item was removed from the list and 0 if it wasn't there in the first place.
 */
int pinless_list_remove(const uint32_t path[WALLET_PATH_LEN]);

#endif /* PINLESS_LIST_H_ */
