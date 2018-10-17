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

#include "storage/fw.h"
#include "storage/flash.h"
#include "app/system.h"

err_t fw_load(uint8_t page_num, uint16_t byte_len, const uint32_t* fw) {
  if ((page_num < UPGRADE_FW_FIRST_PAGE) || (page_num > (UPGRADE_FW_FIRST_PAGE + FIRMWARE_PAGE_COUNT))) return ERR_INVALID_DATA;
  if ((byte_len > FLASH_PAGE_SIZE) || (byte_len % 8)) return ERR_INVALID_DATA;

  if (flash_erase(page_num, 1)) return ERR_UNKNOWN;
  if (flash_copy(fw, FLASH_PAGE_ADDR(page_num), (byte_len >> 2))) return ERR_UNKNOWN;

  return ERR_OK;
}

err_t fw_upgrade() {
  if (system_valid_firmware(UPGRADE_FW_START)) {
    system_schedule_reboot();
  } else {
    flash_erase(UPGRADE_FW_FIRST_PAGE, FIRMWARE_PAGE_COUNT);
    return ERR_INVALID_FW;
  }
  return ERR_OK;
}
