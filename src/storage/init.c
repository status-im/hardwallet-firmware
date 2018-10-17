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
#include "storage/init.h"
#include "storage/fs.h"
#include "storage/pin.h"
#include "util/crypto.h"
#include "wallet/bip39.h"

static int init_phase = -1;
static uint16_t mnemonic[BIP39_MAX_MNEMO];
static int mnlen;

int init_boot() {
  if (!fs_initialized()) {
    init_phase = -1;
  } else if (!pin_is_set()) {
    init_phase = 0;
  } else if (!wallet_created()) {
    init_phase = 1;
  } else {
    init_phase = 4;
  }

  return init_phase;
}

int init_ready() {
  return init_phase == 4;
}

err_t init_phase_0() {
  if (init_phase > 3) {
    err_t auth_err = ui_authenticate_user();

    if (auth_err != ERR_OK) return auth_err;
  }

  if (fs_init()) return ERR_UNKNOWN;

  init_phase = 0;
  return ERR_OK;
}

err_t init_phase_1() {
  if (init_phase != 0) return ERR_INVALID_DATA;

  uint8_t pin[UI_MAX_PIN_LEN + 1];
  pin[0] = UI_MAX_PIN_LEN;
  if (!rng(&pin[1], UI_MAX_PIN_LEN)) return ERR_UNKNOWN;

  for (int i = 1; i <= UI_MAX_PIN_LEN; i++) {
    pin[i] = 0x30 + (pin[i] % 10);
  }

  if (pin_set(pin)) return ERR_UNKNOWN;

  init_phase = 1;
  return ERR_OK;
}

err_t init_phase_2a(uint8_t cslen) {
  if (init_phase != 1) return ERR_INVALID_DATA;

  mnlen = bip39_generate_mnemonic(cslen, mnemonic);
  if (mnlen < 0) return ERR_UNKNOWN;

  err_t err = ui_display_mnemonic(mnemonic, mnlen);
  if (err == ERR_OK) init_phase = 2;

  return err;
}

err_t init_phase_2b(uint8_t cslen) {
  if (init_phase != 1) return ERR_INVALID_DATA;
  mnlen = BIP39_WORD_COUNT(cslen);

  int retry = 0;

  do {
    if (retry) ui_display_retry();
    err_t err = ui_read_mnemonic(mnemonic, mnlen);
    if (err != ERR_OK) return err;
    retry = 1;
  } while(bip39_verify(cslen, mnemonic));

  init_phase = 2;
  return ERR_OK;
}

err_t init_phase_4(const uint8_t* passphrase, uint8_t pplen, uint8_t addr[WALLET_ADDR_LEN]) {
  if (init_phase != 2) return ERR_INVALID_DATA;

  uint8_t seed[BIP39_SEED_LEN];
  uint8_t mnstr[BIP39_MAX_STRLEN];
  int mnstrlen;

  bip39_render_mnemonic(mnemonic, mnlen, mnstr, &mnstrlen);
  if (bip39_generate_seed(mnstr, mnstrlen, passphrase, pplen, seed)) return ERR_UNKNOWN;

  if (wallet_new(seed, BIP39_SEED_LEN)) return ERR_UNKNOWN;
  if (wallet_master_address(addr)) return ERR_UNKNOWN;

  init_phase = 4;
  return ERR_OK;
}
