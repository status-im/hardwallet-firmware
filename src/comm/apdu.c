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

#include "comm/apdu.h"

#define APDU_BUFFER_SIZE 4096
#define APDU_FIRST_SEGMENT_MASK 0x80
#define APDU_LAST_SEGMENT_MASK 0x40
#define APDU_OPCODE_MASK 0x3f

#define APDU_OP_INIT 0x00
#define APDU_OP_SIGN 0x01
#define APDU_OP_GET_ADDRESS 0x02
#define APDU_OP_DISABLE_PIN 0x03
#define APDU_OP_ENABLE_PIN 0x04
#define APDU_OP_CHANGE_PIN 0x05
#define APDU_OP_GET_STATUS 0x06
#define APDU_OP_LOAD_FW 0x10
#define APDU_OP_UPGRADE_FW 0x11

static uint32_t _apdu[APDU_BUFFER_SIZE/4]; // word alignment
static uint8_t* apdu = (uint8_t*)_apdu;

static int apdu_in_idx = 0;
static int apdu_out_idx;
static int apdu_cmd_len;
static int apdu_resp_off;
static int apdu_resp_len;

static int apdu_buffer_direction = 0;
static int apdu_current_opcode;

#include "app/cmds.h"
#include "storage/fw.h"
#include "storage/init.h"
#include "app/system.h"
#include "util/handy.h"
#include <string.h>

apdu_status_t apdu_receive_segment(uint8_t* in, int len) {
  uint8_t header = in[0];

  if (header & APDU_FIRST_SEGMENT_MASK) {
    apdu_buffer_direction = 0;
    apdu_in_idx = 0;
    apdu_current_opcode = header & APDU_OPCODE_MASK;
  } else if ((apdu_buffer_direction != 0) || (apdu_in_idx == 0) || (apdu_current_opcode != (header & APDU_OPCODE_MASK))) {
    return APDU_STATUS_INVALID_SEGMENT;
  }

  if ((apdu_in_idx + len) > APDU_BUFFER_SIZE) return APDU_STATUS_INVALID_SEGMENT;
  memcpy(&apdu[apdu_in_idx], &in[1], len);
  apdu_in_idx += len;

  if (header & APDU_LAST_SEGMENT_MASK) {
    apdu_cmd_len = apdu_in_idx;
    apdu_in_idx = 0;
    return APDU_STATUS_COMPLETE;
  }

  return APDU_STATUS_PARTIAL;
}

static err_t apdu_process_init() {
  err_t res;

  switch(apdu[1]) {
  case 0:
    res = init_phase_0();
    break;
  case 1:
    res = init_phase_1();
    break;
  case 2:
    res = init_phase_2a(apdu[2]);
    break;
  case 3:
    res = init_phase_2b(apdu[2]);
    break;
  case 4:
    res = init_phase_4(&apdu[4], apdu[3], &apdu[apdu_cmd_len]);
    apdu_resp_len += WALLET_ADDR_LEN;
    break;
  default:
    res = ERR_INVALID_DATA;
    break;
  }

  return res;
}

apdu_status_t apdu_process() {
  if (apdu_buffer_direction == 1 || apdu_cmd_len == 0) return APDU_STATUS_INVALID_SEGMENT;

  err_t res;
  apdu_resp_off = apdu_cmd_len - 1;
  apdu_resp_len = 1;
  int low_battery = system_low_battery() << 6;
  uint8_t* out_sig;

  switch(apdu_current_opcode) {
  case APDU_OP_INIT:
    if (low_battery) {
      res = ERR_LOW_BATTERY;
    } else {
      res = apdu_process_init();
    }
    break;
  case APDU_OP_SIGN:
    res = cmd_sign(apdu, &apdu[apdu_cmd_len], &out_sig);
    apdu_resp_off = ((int)(out_sig - apdu)) - 1;
    apdu_resp_len += 67;
    break;
  case APDU_OP_GET_ADDRESS:
    res = cmd_get_address(apdu, &apdu[apdu_cmd_len], &apdu[apdu_cmd_len]);
    apdu_resp_len += WALLET_ADDR_LEN;
    break;
  case APDU_OP_DISABLE_PIN:
    res = cmd_disable_pin(apdu, &apdu[apdu_cmd_len]);
    break;
  case APDU_OP_ENABLE_PIN:
    res = cmd_change_pin(apdu, &apdu[apdu_cmd_len]);
    break;
  case APDU_OP_CHANGE_PIN:
    res = cmd_change_pin(apdu, &apdu[apdu_cmd_len]);
    break;
  case APDU_OP_GET_STATUS:
    res = ERR_UNKNOWN;
    break;
  case APDU_OP_LOAD_FW:
    if (low_battery) {
      res = ERR_LOW_BATTERY;
    } else {
      res =  fw_load(apdu[1], (apdu[2] << 8 | apdu[3]), &_apdu[1]);
    }
    break;
  case APDU_OP_UPGRADE_FW:
    if (low_battery) {
      res = ERR_LOW_BATTERY;
    } else {
      res =  fw_upgrade();
    }
    break;
  default:
    res = ERR_UNKNOWN;
    break;
  }

  if (res != ERR_OK) apdu_resp_len = 1;
  apdu[apdu_resp_off] = (low_battery | res);
  apdu_out_idx = 0;
  apdu_buffer_direction = 1;
  apdu_cmd_len = 0;

  return APDU_STATUS_COMPLETE;
}

apdu_status_t apdu_send_segment(uint8_t* out, int len) {
  if (apdu_buffer_direction != 1) return APDU_STATUS_INVALID_SEGMENT;
  apdu_status_t data_len = MIN((len - 1), (apdu_resp_len - apdu_out_idx));

  uint8_t header = apdu_current_opcode;

  if (apdu_out_idx == 0) {
    header |= APDU_FIRST_SEGMENT_MASK;
  }

  memcpy(&apdu[apdu_resp_off + apdu_out_idx], &out[1], data_len);
  apdu_out_idx += data_len;

  if (apdu_out_idx >= apdu_resp_len) {
    apdu_buffer_direction = 0;
    header |= APDU_LAST_SEGMENT_MASK;
    data_len = APDU_STATUS_COMPLETE | (data_len + 1);
  } else {
    data_len = APDU_STATUS_PARTIAL | (data_len + 1);
  }

  out[0] = header;

  return data_len;
}
