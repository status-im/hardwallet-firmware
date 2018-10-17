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

#ifndef INIT_H_
#define INIT_H_

#include "wallet/wallet.h"
#include "util/err.h"

/**
 * Must be called on boot to determine the initialization phase. Returns the current phase.
 */
int init_boot();

/**
 * Returns 1 if the device is ready for normal operation, 0 if it must be initialized.
 */
int init_ready();

/**
 * Performs phase 0 of device initialization. Refer to PROTOCOL.MD for details
 */
err_t init_phase_0();

/**
 * Performs phase 1 of device initialization. Refer to PROTOCOL.MD for details
 */
err_t init_phase_1();

/**
 * Performs phase 2a of device initialization. Refer to PROTOCOL.MD for details
 */
err_t init_phase_2a(uint8_t cslen);

/**
 * Performs phase 2b of device initialization. Refer to PROTOCOL.MD for details
 */
err_t init_phase_2b(uint8_t cslen);

/**
 * Performs phase 4 of device initialization. Refer to PROTOCOL.MD for details
 */
err_t init_phase_4(const uint8_t* passphrase, uint8_t pplen, uint8_t addr[WALLET_ADDR_LEN]);

#endif /* INIT_H_ */
