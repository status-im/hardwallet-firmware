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

#include "app/ui.h"

err_t ui_authenticate_user() {
  return ERR_UNKNOWN;
}

err_t ui_confirm() {
  return ERR_UNKNOWN;
}

err_t ui_display_mnemonic(const uint16_t mnemonic[], int mnlen) {
  return ERR_UNKNOWN;
}

err_t ui_read_mnemonic(uint16_t mnemonic[], int mnlen) {
  return ERR_UNKNOWN;
}

err_t ui_display_retry(void) {
  return ERR_UNKNOWN;
}

err_t ui_confirm_addr(int msg_id, uint8_t addr[WALLET_ADDR_LEN]) {
  return ERR_UNKNOWN;
}

err_t ui_confirm_amount(uint8_t* amount, int amount_len, const char* symbol) {
  return ERR_UNKNOWN;
}

err_t ui_get_pin(int msg_id, uint8_t pin[UI_MAX_PIN_LEN]) {
  return ERR_UNKNOWN;
}
