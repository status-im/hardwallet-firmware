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

#include "hci.h"
#include "bluenrg_itf.h"
#include "bluenrg_hal_aci.h"
#include "bluenrg_gap_aci.h"
#include "bluenrg_gatt_aci.h"
#include "bluenrg_gap.h"
#include "sm.h"

const uint8_t BLE_DEVICE_NAME[] = {'S', 't', 'a', 't', 'u', 's', ' ', 'H', 'a', 'r', 'd', 'w', 'a', 'r', 'e', ' ', 'W', 'a', 'l', 'l', 'e', 't', ' ', 'P', 'r', 'o'};

void ble_init() {
  HCI_Init();
  BNRG_SPI_Init();
  BlueNRG_RST();

  uint8_t bdaddr[] = {0x12, 0x34, 0x00, 0xE1, 0x80, 0x02};
  uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
  aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET,CONFIG_DATA_PUBADDR_LEN, bdaddr);
  aci_gatt_init();
  aci_gap_init_IDB05A1(GAP_PERIPHERAL_ROLE_IDB05A1, PRIVACY_ENABLED, sizeof(BLE_DEVICE_NAME), &service_handle, &dev_name_char_handle, &appearance_char_handle);
  aci_gatt_update_char_value(service_handle, dev_name_char_handle, 0, sizeof(BLE_DEVICE_NAME), BLE_DEVICE_NAME);
  aci_gap_set_auth_requirement(MITM_PROTECTION_REQUIRED, OOB_AUTH_DATA_ABSENT, NULL, 16, 16, DONOT_USE_FIXED_PIN_FOR_PAIRING, 0, BONDING);
}

void ble_process() {
  HCI_Process();
}

void ble_isr() {
  HCI_Isr();
}

void HCI_Event_CB(void *pckt) {

}
