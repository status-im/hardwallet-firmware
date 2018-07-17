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

#include "err.h"
#include "wallet.h"

err_t cmd_sign(const uint8_t* rlp_data, const uint8_t* barrier, uint8_t** out_sig);
err_t cmd_get_address(const uint8_t* rlp_wallet_path, const uint8_t* barrier, uint8_t addr[WALLET_ADDR_LEN]);

err_t cmd_disable_pin(const uint8_t* rlp_wallet_path, const uint8_t* barrier);
err_t cmd_enable_pin(const uint8_t* rlp_wallet_path, const uint8_t* barrier);

err_t cmd_change_pin();

#endif /* CMDS_H_ */
