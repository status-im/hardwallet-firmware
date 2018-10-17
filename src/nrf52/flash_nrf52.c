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

#ifdef NRF52840_XXAA

#include "storage/flash.h"

#include "nrf.h"
#include "nrf_soc.h"
#include "nordic_common.h"
#include "app_util.h"
#include "app_error.h"
#include "nrf_fstorage.h"

#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_fstorage_sd.h"

#include "nrf_log.h"

static void fstorage_evt_handler(nrf_fstorage_evt_t* p_evt);

NRF_FSTORAGE_DEF(static nrf_fstorage_t flash_user_data_fstorage) = {
  .evt_handler = fstorage_evt_handler,
  .start_addr = FLASH_USER_AREA_START,
  .end_addr   = (FLASH_USER_AREA_START + FLASH_USER_AREA_SIZE - 1),
};

static ret_code_t flash_error_code;

static ret_code_t wait_for_flash_ready() {
  while (nrf_fstorage_is_busy(&flash_user_data_fstorage)) {
    (void) sd_app_evt_wait();
  }

  return flash_error_code;
}

static void fstorage_evt_handler(nrf_fstorage_evt_t* p_evt) {
  flash_error_code = p_evt->result;

  if (p_evt->result != NRF_SUCCESS) {
    NRF_LOG_INFO("--> Event received: ERROR while executing an fstorage operation.");
    return;
  }

  switch (p_evt->id) {
    case NRF_FSTORAGE_EVT_WRITE_RESULT:
      NRF_LOG_INFO("--> Event received: wrote %d bytes at address 0x%x.", p_evt->len, p_evt->addr);
      break;
    case NRF_FSTORAGE_EVT_ERASE_RESULT:
      NRF_LOG_INFO("--> Event received: erased %d page from address 0x%x.", p_evt->len, p_evt->addr);
      break;
    default:
      break;
  }
}

void _nrf52_flash_init() {
  ret_code_t rc = nrf_fstorage_init(&flash_user_data_fstorage, &nrf_fstorage_sd, NULL);
  APP_ERROR_CHECK(rc);
}

int flash_lock() { return 0; }

int flash_unlock() { return 0; }

int flash_copy(const uint32_t* src, uint32_t* dst, uint32_t size) {
  ret_code_t rc = nrf_fstorage_write(&flash_user_data_fstorage, (uint32_t) dst, src, size, NULL);
  APP_ERROR_CHECK(rc);

  wait_for_flash_ready();

  return (flash_error_code == NRF_SUCCESS) ? 0 : -1;
}

int flash_erase(uint8_t page, uint8_t page_count) {
  ret_code_t rc = nrf_fstorage_erase(&flash_user_data_fstorage, (page * FLASH_PAGE_SIZE), page_count, NULL);
  return (flash_error_code == NRF_SUCCESS) ? 0 : -1;
}

#endif