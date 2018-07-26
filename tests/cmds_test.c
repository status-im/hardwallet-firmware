#include "acutest.h"
#include <string.h>

#define __IO volatile
#define FLASH_BASE ((uintptr_t) 0)

#include "../Src/cmds.c"
#include "../Src/rlp.c"

static int test_init_ready;

static err_t test_ui_authenticate_user;
static int test_pinless_list_result;

static uint8_t test_rlp_valid_path[] = { 0xc6, 0x04, 0x84, 0x80, 0x00, 0x00, 0x03 };
static uint8_t test_rlp_master_path[] = { 0xc0 };
static uint8_t test_rlp_too_long_path[] = { 0xca, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a };

#define TEST_RLP_DATA_LEN 51
uint8_t test_rlp_data[] = {0xc6,0x04,0x84,0x80,0x00,0x00,0x03,0xeb,0x02,0x85,0x02,0xef,0x04,0xbe,0x00,0x82,0x52,0x08,0x94,0xd2,0xc5,0xde,0x69,0xd9,0x65,0xcb,0xfd,0x87,0x1e,0x4f,0x1c,0x21,0x78,0x82,0x19,0x10,0x0f,0x7c,0xfc,0x87,0x81,0x57,0x4f,0xbe,0x28,0x00,0x00,0x80,0x01,0x80,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};


static uint8_t test_wallet_addr[WALLET_ADDR_LEN];

static int pin_unverify_called;
static int test_ui_get_pin_called;

int init_ready() {
  return test_init_ready;
}

int wallet_priv_key(const uint32_t path[WALLET_PATH_LEN], uint8_t out_priv[BIP32_KEY_COMPONENT_LEN]) {
  return 0;
}

int wallet_address(const uint32_t path[WALLET_PATH_LEN], uint8_t out_addr[WALLET_ADDR_LEN]) {
  TEST_CHECK(path[0] == 2);
  TEST_CHECK(path[1] == 4);
  TEST_CHECK(path[2] == 0x80000003);

  out_addr[0] = 0xBB;
  return 0;
}

int wallet_master_address(uint8_t out_addr[WALLET_ADDR_LEN]) {
  out_addr[0] = 0xAA;
  return 0;
}

int eth_parse(eth_tx_t* tx) {
  TEST_CHECK(tx->buffer[0] == 0xeb);
  TEST_CHECK(tx->barrier == &test_rlp_data[TEST_RLP_DATA_LEN]);
  tx->v = &test_rlp_data[TEST_RLP_DATA_LEN - 3];
  return 0;
}

int eth_sign(eth_tx_t* tx, const uint8_t* priv_key) {
  return 0;
}

int pin_change(uint8_t* new_pin) {
  TEST_CHECK(pin_unverify_called);
  pin_unverify_called = 0;
  TEST_CHECK(new_pin[0] == 5);
  TEST_CHECK(!memcmp(&new_pin[1], "12345", 5));

  return 0;
}

void pin_unverify() {
  pin_unverify_called = 1;
}

int pinless_list_contains(const uint32_t path[WALLET_PATH_LEN]) {
  return test_pinless_list_result;
}

int pinless_list_add(const uint32_t path[WALLET_PATH_LEN]) {
  return test_pinless_list_result;
}

int pinless_list_remove(const uint32_t path[WALLET_PATH_LEN]) {
  return test_pinless_list_result;
}

err_t ui_authenticate_user() {
  return test_ui_authenticate_user;
}

err_t ui_confirm() {
  return ERR_OK;
}

err_t ui_display_retry(void) {
  return ERR_OK;
}

err_t ui_confirm_addr(int msg_id, uint8_t addr[WALLET_ADDR_LEN]) {
  return ERR_OK;
}
err_t ui_confirm_amount(uint8_t* amount, int amount_len, const char* symbol) {
  return ERR_OK;
}

err_t ui_get_pin(int msg_id, uint8_t pin[UI_MAX_PIN_LEN]) {
  switch(test_ui_get_pin_called++) {
  case 0:
    TEST_CHECK(msg_id == UI_PROMPT_NEW_PIN);
    return ERR_USER_CANCELLED;
  case 1:
  case 3:
  case 5:
  case 7:
    TEST_CHECK(msg_id == UI_PROMPT_NEW_PIN);
    pin[0] = 5; memcpy(&pin[1], "12345", 5);
    return ERR_OK;
  case 2:
    TEST_CHECK(msg_id == UI_PROMPT_REPEAT_PIN);
    return ERR_USER_CANCELLED;
  case 4:
    TEST_CHECK(msg_id == UI_PROMPT_REPEAT_PIN);
    pin[0] = 5; memcpy(&pin[1], "54321", 5);
    return ERR_OK;
  case 6:
    TEST_CHECK(msg_id == UI_PROMPT_REPEAT_PIN);
    pin[0] = 4; memcpy(&pin[1], "1234", 4);
    return ERR_OK;
  case 8:
    TEST_CHECK(msg_id == UI_PROMPT_REPEAT_PIN);
    pin[0] = 5; memcpy(&pin[1], "12345", 5);
    return ERR_OK;
  default:
    return ERR_UNKNOWN;
  }
}

void test_cmd_sign(void) {
  test_init_ready = 0;
  uint8_t* out_sig;
  TEST_CHECK(cmd_sign(test_rlp_data, &test_rlp_data[TEST_RLP_DATA_LEN], &out_sig) == ERR_UNINITIALIZED);
  test_init_ready = 1;
  test_pinless_list_result = 1;
  test_ui_authenticate_user = ERR_UNAUTHORIZED;
  TEST_CHECK(cmd_sign(test_rlp_data, &test_rlp_data[TEST_RLP_DATA_LEN], &out_sig) == ERR_OK);
  test_pinless_list_result = 0;
  TEST_CHECK(cmd_sign(test_rlp_data, &test_rlp_data[TEST_RLP_DATA_LEN], &out_sig) == ERR_UNAUTHORIZED);
  test_ui_authenticate_user = ERR_OK;
  TEST_CHECK(cmd_sign(test_rlp_data, &test_rlp_data[TEST_RLP_DATA_LEN], &out_sig) == ERR_OK);
}

void test_cmd_get_address(void) {
  test_init_ready = 0;
  TEST_CHECK(cmd_get_address(test_rlp_master_path, &test_rlp_master_path[1], test_wallet_addr) == ERR_UNINITIALIZED);
  test_init_ready = 1;
  TEST_CHECK(cmd_get_address(test_rlp_too_long_path, &test_rlp_too_long_path[11], test_wallet_addr) == ERR_INVALID_DATA);
  TEST_CHECK(cmd_get_address(test_rlp_master_path, &test_rlp_master_path[1], test_wallet_addr) == ERR_OK);
  TEST_CHECK(test_wallet_addr[0] == 0xAA);
  TEST_CHECK(cmd_get_address(test_rlp_valid_path, &test_rlp_valid_path[7], test_wallet_addr) == ERR_OK);
  TEST_CHECK(test_wallet_addr[0] == 0xBB);
}

void test_cmd_disable_pin(void) {
  test_init_ready = 0;
  TEST_CHECK(cmd_disable_pin(test_rlp_valid_path, &test_rlp_valid_path[7]) == ERR_UNINITIALIZED);
  test_init_ready = 1;
  test_ui_authenticate_user = ERR_UNAUTHORIZED;
  TEST_CHECK(cmd_disable_pin(test_rlp_valid_path, &test_rlp_valid_path[7]) == ERR_UNAUTHORIZED);
  test_ui_authenticate_user = ERR_OK;
  TEST_CHECK(cmd_disable_pin(test_rlp_master_path, &test_rlp_master_path[1]) == ERR_INVALID_DATA);
  test_pinless_list_result = 1;
  TEST_CHECK(cmd_disable_pin(test_rlp_valid_path, &test_rlp_valid_path[7]) == ERR_OK);
  test_pinless_list_result = 0;
  TEST_CHECK(cmd_disable_pin(test_rlp_valid_path, &test_rlp_valid_path[7]) == ERR_OK);
  test_pinless_list_result = -2;
  TEST_CHECK(cmd_disable_pin(test_rlp_valid_path, &test_rlp_valid_path[7]) == ERR_LIMIT_EXCEEDED);
}

void test_cmd_enable_pin(void) {
  test_init_ready = 0;
  TEST_CHECK(cmd_enable_pin(test_rlp_valid_path, &test_rlp_valid_path[7]) == ERR_UNINITIALIZED);
  test_init_ready = 1;
  test_ui_authenticate_user = ERR_UNAUTHORIZED;
  TEST_CHECK(cmd_enable_pin(test_rlp_valid_path, &test_rlp_valid_path[7]) == ERR_UNAUTHORIZED);
  test_ui_authenticate_user = ERR_OK;
  test_pinless_list_result = 1;
  TEST_CHECK(cmd_enable_pin(test_rlp_valid_path, &test_rlp_valid_path[7]) == ERR_OK);
  test_pinless_list_result = 0;
  TEST_CHECK(cmd_enable_pin(test_rlp_valid_path, &test_rlp_valid_path[7]) == ERR_OK);
}

void test_cmd_change_pin(void) {
  // most of the test logic happens in the ui_get_pin function
  test_init_ready = 0;
  test_ui_get_pin_called = 0;
  TEST_CHECK(cmd_change_pin() == ERR_UNINITIALIZED);
  test_init_ready = 1;
  test_ui_authenticate_user = ERR_UNAUTHORIZED;
  TEST_CHECK(cmd_change_pin() == ERR_UNAUTHORIZED);
  test_ui_authenticate_user = ERR_OK;
  TEST_CHECK(cmd_change_pin() == ERR_USER_CANCELLED);
  TEST_CHECK(test_ui_get_pin_called == 1);
  TEST_CHECK(cmd_change_pin() == ERR_USER_CANCELLED);
  TEST_CHECK(test_ui_get_pin_called == 3);
  TEST_CHECK(cmd_change_pin() == ERR_OK);
  TEST_CHECK(test_ui_get_pin_called == 9);
}

TEST_LIST = {
   { "cmd_sign", test_cmd_sign },
   { "cmd_get_address", test_cmd_get_address },
   { "cmd_disable_pin", test_cmd_disable_pin },
   { "cmd_enable_pin", test_cmd_enable_pin },
   { "cmd_change_pin", test_cmd_change_pin },
   { 0 }
};
