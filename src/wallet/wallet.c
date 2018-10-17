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

#include "wallet/wallet.h"
#include "storage/fs.h"
#include "util/crypto.h"
#include "util/handy.h"

#define WALLET_ENC_KEY_IDX 508
#define WALLET_MK_IDX 2
#define WALLET_MK_SIZE 32
#define WALLET_KEY_SIZE WALLET_MK_SIZE + WALLET_PATH_LEN

#define WALLET_RAND_LEN 11
#define WALLET_PUBKEY_LEN (BIP32_KEY_COMPONENT_LEN + 1)
#define WALLET_ADDR_LEN 20

typedef struct {
  uint8_t rand[WALLET_RAND_LEN];
  uint8_t pub_key[WALLET_PUBKEY_LEN];
  uint8_t priv_key[BIP32_KEY_COMPONENT_LEN];
  uint8_t chain[BIP32_KEY_COMPONENT_LEN];
  uint8_t addr[WALLET_ADDR_LEN];
} wallet_key_t;

static uint8_t* _wallet_enc_key() {
  uint32_t* page = fs_get_page(FS_WRITE_ONCE_PAGE, 0);
  return (uint8_t*)(&page[WALLET_ENC_KEY_IDX]);
}

static uint32_t* _wallet_mk() {
  uint32_t* page = fs_get_page(FS_WRITE_ONCE_PAGE, 0);
  return &page[WALLET_MK_IDX];
}

static void _wallet_make_addr(bip32_pub_key_t* pub, uint8_t out[WALLET_ADDR_LEN]) {
  uint8_t hash[SHA_256_LEN];
  sha256(pub->x, (BIP32_KEY_COMPONENT_LEN * 2), hash);
  memcpy(out, hash, WALLET_ADDR_LEN);
}

static int _wallet_make_key(wallet_key_t* key, bip32_priv_key_t* priv, bip32_pub_key_t* pub) {
  if (!rng(key->rand, WALLET_RAND_LEN)) return -1;

  memcpy(key->pub_key, &pub->y_comp, WALLET_PUBKEY_LEN);
  aes128_cbc_enc(_wallet_enc_key(), key->rand, priv->key, ((BIP32_KEY_COMPONENT_LEN * 2) / 16), key->priv_key); // encrypts private key and chain
  _wallet_make_addr(pub, key->addr);

  return 0;
}

int wallet_new(const uint8_t* seed, int seed_len) {
  uint32_t* mk = _wallet_mk();
  if (mk[0] != 0xffffffff) return -1;

  bip32_priv_key_t priv;
  bip32_pub_key_t pub;
  bip32_master_key(seed, seed_len, &priv, &pub);

  uint32_t mk_data[WALLET_MK_SIZE];
  if (_wallet_make_key((wallet_key_t*) mk_data, &priv, &pub)) return -1;

  return fs_write_entry(mk, mk_data, WALLET_MK_SIZE);
}

inline int wallet_created() {
  uint32_t* mk = _wallet_mk();
  return mk[0] != 0xffffffff;
}

static wallet_key_t* _wallet_closest_cache_entry(const uint32_t path[WALLET_PATH_LEN], int* found_level) {
  *found_level = 0;

  uint32_t* mk = _wallet_mk();
  if (mk[0] == 0xffffffff) return NULL;

  wallet_key_t* res = (wallet_key_t*) mk;

  for (int i = 0; i < FS_KEY_CACHE_COUNT; i++) {
    uint32_t* page = fs_get_page(FS_KEY_CACHE_PAGE, i);

    for (int i = FS_HEADER_SIZE; i < FS_FLASH_PAGE_SIZE; i += WALLET_KEY_SIZE) {
      if (page[i] == 0xffffffff) {
        return res;
      } else if ((page[i] <= path[0]) && (page[i] > *found_level)) {
        if (!wordcmp(&page[i+1], &path[1], page[i])) {
          *found_level = page[i];
          res = (wallet_key_t*)(&page[i + WALLET_PATH_LEN]);

          if (page[i] == path[0]) {
            return res;
          }
        }
      }
    }
  }

  return res;
}

static wallet_key_t* _wallet_derive_key(const uint32_t path[WALLET_PATH_LEN], wallet_key_t* base_key, int level, uint8_t plain_priv_key[BIP32_KEY_COMPONENT_LEN]) {
  bip32_priv_key_t priv1;
  bip32_priv_key_t priv2;
  bip32_pub_key_t pub1;
  bip32_pub_key_t pub2;

  uint32_t key_entry[WALLET_KEY_SIZE];
  memcpy(&key_entry[1], &path[1], (level << 2));
  memset(&key_entry[1 + level], 0xff, ((WALLET_PATH_LEN - level - 1) << 2));

  bip32_priv_key_t* parent_priv = &priv1;
  bip32_pub_key_t* parent_pub = &pub1;

  bip32_priv_key_t* child_priv = &priv2;
  bip32_pub_key_t* child_pub = &pub2;

  aes128_cbc_dec(_wallet_enc_key(), base_key->rand, base_key->priv_key, ((BIP32_KEY_COMPONENT_LEN * 2) / 16), parent_priv->key); // decrypts private key and chain
  memcpy(&parent_pub->y_comp, base_key->pub_key, WALLET_PUBKEY_LEN);

  uint32_t *res = NULL;

  while(++level <= path[0]) {
    if (bip32_ckd_private(path[level], parent_priv, parent_pub, child_priv, child_pub)) return NULL;

    key_entry[0] = level;
    key_entry[level] = path[level];

    if (_wallet_make_key((wallet_key_t*)&key_entry[WALLET_PATH_LEN], child_priv, child_pub)) return NULL;
    res = fs_cache_entry(FS_KEY_CACHE_PAGE, FS_KEY_CACHE_COUNT, WALLET_KEY_SIZE, key_entry);

    if (!res) {
      return NULL;
    }

    SWAP(parent_priv, child_priv);
    SWAP(parent_pub, child_pub);
  }

  if (plain_priv_key) {
    memcpy(plain_priv_key, parent_priv->key, BIP32_KEY_COMPONENT_LEN);
  }

  return (wallet_key_t*) (res ? &res[WALLET_PATH_LEN] : res);
}

int _wallet_get_key(const uint32_t path[WALLET_PATH_LEN], wallet_key_t** out, uint8_t plain_priv_key[BIP32_KEY_COMPONENT_LEN]) {
  if (path[0] < 1 || path[0] > 9) return -1;

  int level;
  wallet_key_t* key = _wallet_closest_cache_entry(path, &level);

  if (key == NULL) return -1;

  if (level == path[0]) {
    if (plain_priv_key) {
      aes128_cbc_dec(_wallet_enc_key(), key->rand, key->priv_key, (BIP32_KEY_COMPONENT_LEN / 16), plain_priv_key);
    }
  } else {
    key = _wallet_derive_key(path, key, level, plain_priv_key);
    if (key == NULL) return -1;
  }

  if (out) {
    *out = key;
  }

  return 0;
}

int wallet_priv_key(const uint32_t path[WALLET_PATH_LEN], uint8_t out_priv[BIP32_KEY_COMPONENT_LEN]) {
  return _wallet_get_key(path, NULL, out_priv);
}

int wallet_address(const uint32_t path[WALLET_PATH_LEN], uint8_t out_addr[WALLET_ADDR_LEN]) {
  wallet_key_t* key;

  if (_wallet_get_key(path, &key, NULL)) return -1;
  memcpy(out_addr, key->addr, WALLET_ADDR_LEN);

  return 0;
}

int wallet_master_address(uint8_t out_addr[WALLET_ADDR_LEN]) {
  uint32_t* mk = _wallet_mk();
  if (mk[0] == 0xffffffff) return -1;

  wallet_key_t* key = (wallet_key_t*) mk;
  memcpy(out_addr, key->addr, WALLET_ADDR_LEN);

  return 0;
}
