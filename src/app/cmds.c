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

#include <string.h>
#include "app/cmds.h"
#include "eth/eth.h"
#include "eth/rlp.h"
#include "app/ui.h"
#include "storage/pin.h"
#include "storage/pinless_list.h"
#include "storage/init.h"

static err_t cmd_path_decode(const uint8_t* rlp_wallet_path, const uint8_t* barrier, uint32_t wallet_path[WALLET_PATH_LEN], uint8_t** after_list) {
  uint8_t* next;
  uint8_t* value;

  int list_len = rlp_parse(rlp_wallet_path, &value, &next, barrier);
  if (list_len == -1) return ERR_INVALID_DATA;

  if (after_list) *after_list = next;
  const uint8_t* inner_barrier = next ? next : barrier;
  next = list_len ? value : NULL;

  wallet_path[0] = 0;

  while(next) {
    wallet_path[0]++;
    if (wallet_path[0] > 9) return ERR_INVALID_DATA;
    if (rlp_read_uint32(next, &wallet_path[wallet_path[0]], &next, inner_barrier) == -1) return ERR_INVALID_DATA;
  }

  return ERR_OK;
}

err_t cmd_sign(const uint8_t* rlp_data, const uint8_t* barrier, uint8_t** out_sig) {
  if (!init_ready()) return ERR_UNINITIALIZED;

  eth_tx_t tx;
  uint32_t wallet_path[WALLET_PATH_LEN];
  err_t err = cmd_path_decode(rlp_data, barrier, wallet_path, &tx.buffer);
  if (err != ERR_OK) return err;
  if (wallet_path[0] == 0) return ERR_INVALID_DATA;

  tx.barrier = barrier;
  if (eth_parse(&tx)) return ERR_INVALID_DATA;

  uint8_t src_addr[WALLET_ADDR_LEN];
  if (wallet_address(wallet_path, src_addr)) return ERR_UNKNOWN;
  err = ui_confirm_addr(UI_PROMPT_SRC_ADDR, src_addr);
  if (err != ERR_OK) return err;

  if (tx.erc20_idx != -1) {
    err = ui_confirm_addr(UI_PROMPT_DST_ADDR, tx.dst_addr);
    if (err != ERR_OK) return err;
    err = ui_confirm_amount(tx.value, tx.value_len, "ETH");
    if (err != ERR_OK) return err;
  } else {
    err = ui_confirm_addr(UI_PROMPT_DST_ADDR, &tx.data[16]);
    if (err != ERR_OK) return err;
    // implement ERC20 lookup
    err = ui_confirm_amount(&tx.data[36], 32, "ERC");
    if (err != ERR_OK) return err;
  }

  if (pinless_list_contains(wallet_path)) {
    err = ui_confirm();
  } else {
    err = ui_authenticate_user();
  }

  if (err != ERR_OK) return err;

  uint8_t priv_key[BIP32_KEY_COMPONENT_LEN];
  if (wallet_priv_key(wallet_path, priv_key) == -1) return ERR_UNKNOWN;
  if (eth_sign(&tx, priv_key) == -1) return ERR_UNKNOWN;

  *out_sig = tx.v;
  return err;
}

err_t cmd_get_address(const uint8_t* rlp_wallet_path, const uint8_t* barrier, uint8_t addr[WALLET_ADDR_LEN]) {
  if (!init_ready()) return ERR_UNINITIALIZED;

  uint32_t wallet_path[WALLET_PATH_LEN];
  err_t err = cmd_path_decode(rlp_wallet_path, barrier, wallet_path, NULL);
  if (err != ERR_OK) return err;

  int e;
  if (wallet_path[0]) {
    e = wallet_address(wallet_path, addr);
  } else {
    e = wallet_master_address(addr);
  }

  return (e == -1) ? ERR_UNKNOWN : ERR_OK;
}

err_t cmd_disable_pin(const uint8_t* rlp_wallet_path, const uint8_t* barrier) {
  if (!init_ready()) return ERR_UNINITIALIZED;

  uint32_t wallet_path[WALLET_PATH_LEN];
  err_t err = cmd_path_decode(rlp_wallet_path, barrier, wallet_path, NULL);
  if (err != ERR_OK) return err;
  if (wallet_path[0] == 0) return ERR_INVALID_DATA;

  err = ui_authenticate_user();
  if (err != ERR_OK) return err;

  int e = pinless_list_add(wallet_path);

  if (e == -2) {
    return ERR_LIMIT_EXCEEDED;
  } else if (e == -1) {
    return ERR_UNKNOWN;
  } else {
    return ERR_OK;
  }
}

err_t cmd_enable_pin(const uint8_t* rlp_wallet_path, const uint8_t* barrier) {
  if (!init_ready()) return ERR_UNINITIALIZED;

  uint32_t wallet_path[WALLET_PATH_LEN];
  err_t err = cmd_path_decode(rlp_wallet_path, barrier, wallet_path, NULL);
  if (err != ERR_OK) return err;

  err = ui_authenticate_user();
  if (err != ERR_OK) return err;

  return (pinless_list_remove(wallet_path) == -1) ? ERR_UNKNOWN : ERR_OK;
}

err_t cmd_change_pin() {
  if (!init_ready()) return ERR_UNINITIALIZED;

  pin_unverify();
  err_t err = ui_authenticate_user();
  if (err != ERR_OK) return err;

  uint8_t new_pin[UI_MAX_PIN_LEN + 1];
  uint8_t repeat_pin[UI_MAX_PIN_LEN + 1];
  int retry = 0;

  do {
    if (retry) ui_display_retry();
    err = ui_get_pin(UI_PROMPT_NEW_PIN, new_pin);
    if (err != ERR_OK) return err;
    err = ui_get_pin(UI_PROMPT_REPEAT_PIN, repeat_pin);
    if (err != ERR_OK) return err;
    retry = 1;
  } while(new_pin[0] != repeat_pin[0] || memcmp(&new_pin[1], &repeat_pin[1], new_pin[0]));

  return (pin_change(new_pin) == -1) ? ERR_UNKNOWN : ERR_OK;
}
