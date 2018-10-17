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

#ifndef CMDS_H_
#define CMDS_H_

#include "util/err.h"
#include "wallet/wallet.h"

/**
 * Signs a transaction. The data must contain an RLP-encoded wallet path followed by the Ethereum transaction.
 * The transaction is signed in place so the rlp_data buffer must be able to grow by at least 64 bytes.
 * The out_sig pointer will point to the beginning of the signature.
 *
 * Requires PIN-authentication, unless PIN entry has been disabled for the given wallet.
 *
 * The master wallet cannot be used to sign transactions.
 */
err_t cmd_sign(const uint8_t* rlp_data, const uint8_t* barrier, uint8_t** out_sig);

/**
 * Gets the address of the wallet by the given path.
 */
err_t cmd_get_address(const uint8_t* rlp_wallet_path, const uint8_t* barrier, uint8_t addr[WALLET_ADDR_LEN]);

/**
 * Disables PIN-entry for the given wallet. The master wallet path (empty path) is considered to be invalid.
 */
err_t cmd_disable_pin(const uint8_t* rlp_wallet_path, const uint8_t* barrier);

/**
 * Re-enables PIN-entry for the given wallet. Note that by default all wallets for which PIN-entry has not been
 * explicitly disabled will require a PIN.
 */
err_t cmd_enable_pin(const uint8_t* rlp_wallet_path, const uint8_t* barrier);

/**
 * Changes the device PIN. This happens by prompting the user to supply the current PIN and the new one.
 * When changing PIN, the user will be prompted to insert the old PIN regardless of whether it has been
 * verified in this session already.
 */
err_t cmd_change_pin();

#endif /* CMDS_H_ */
