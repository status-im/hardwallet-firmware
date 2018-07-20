#include "acutest.h"
#include <string.h>

#define __IO volatile
#define FLASH_BASE ((uintptr_t) 0)

#include "../Src/cmds.c"

int init_ready() {
  return 1;
}

int wallet_priv_key(const uint32_t path[WALLET_PATH_LEN], uint8_t out_priv[BIP32_KEY_COMPONENT_LEN]) {
  return -1;
}

int wallet_address(const uint32_t path[WALLET_PATH_LEN], uint8_t out_addr[WALLET_ADDR_LEN]) {
  return -1;
}

int wallet_master_address(uint8_t out_addr[WALLET_ADDR_LEN]) {
  return -1;
}

int eth_parse(eth_tx_t* tx) {
  return -1;
}

int eth_sign(eth_tx_t* tx, const uint8_t* priv_key) {
  return -1;
}

int rlp_parse(const uint8_t* item, uint8_t** value, uint8_t** next, const uint8_t* barrier) {
  return -1;
}

int rlp_read_uint32(const uint8_t* item, uint32_t* value, uint8_t** next, const uint8_t* barrier) {
  return -1;
}

int pin_change(uint8_t* new_pin) {
  return -1;
}

void pin_unverify() {

}

int pinless_list_contains(const uint32_t path[WALLET_PATH_LEN]) {
  return -1;
}

int pinless_list_add(const uint32_t path[WALLET_PATH_LEN]) {
  return -1;
}

int pinless_list_remove(const uint32_t path[WALLET_PATH_LEN]) {
  return -1;
}

err_t ui_authenticate_user() {
  return ERR_UNKNOWN;
}

err_t ui_confirm() {
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

err_t ui_get_pin(int msg_id, uint8_t pin[UI_PIN_LEN]) {
  return ERR_UNKNOWN;
}

void test_cmd_sign(void) {

}

void test_cmd_get_address(void) {

}

void test_cmd_disable_pin(void) {

}

void test_cmd_enable_pin(void) {

}

void test_cmd_change_pin(void) {

}

TEST_LIST = {
   { "cmd_sign", test_cmd_sign },
   { "cmd_get_address", test_cmd_get_address },
   { "cmd_disable_pin", test_cmd_disable_pin },
   { "cmd_enable_pin", test_cmd_enable_pin },
   { "cmd_change_pin", test_cmd_change_pin },
   { 0 }
};
