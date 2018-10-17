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

#ifndef UI_H_
#define UI_H_

#include "util/err.h"
#include "wallet/wallet.h"
#include "storage/pin.h"

#define UI_PROMPT_SRC_ADDR 0
#define UI_PROMPT_DST_ADDR 1
#define UI_PROMPT_NEW_PIN 2
#define UI_PROMPT_REPEAT_PIN 3

#define UI_MAX_PIN_LEN 5

/**
 * Authenticates user by asking for a PIN. If the current session has been already authenticated, returns ERR_OK directly.
 */
err_t ui_authenticate_user();

/**
 * Asks the user to confirm an action
 */
err_t ui_confirm();

/**
 * Display the mnemonic phrase, word by word.
 */
err_t ui_display_mnemonic(const uint16_t mnemonic[], int mnlen);

/**
 * Allows the user to input the mnemonic phrase word by word and stores the associated indexes in the given array.
 */
err_t ui_read_mnemonic(uint16_t mnemonic[], int mnlen);

/**
 * Display retry prompt to the user.
 */
err_t ui_display_retry(void);

/**
 * Prompt the user to confirm an ethereum address.
 */
err_t ui_confirm_addr(int msg_id, uint8_t addr[WALLET_ADDR_LEN]);

/**
 * Prompts the user to confirm an amount
 */
err_t ui_confirm_amount(uint8_t* amount, int amount_len, const char* symbol);

/**
 * Gets a PIN by prompting the user to insert one.
 */
err_t ui_get_pin(int msg_id, uint8_t pin[UI_MAX_PIN_LEN]);

#endif /* UI_H_ */
