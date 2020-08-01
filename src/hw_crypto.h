/*****************************************************************************
 *   (c) 2017-2020 Cedric Mesnil <cslashm@gmail.com>, Ledger SAS.
 *   (c) 2020 Ledger SAS.
 *   (c) 2020 The TurtleCoin Developers
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

#ifndef HW_CRYPTO_H
#define HW_CRYPTO_H

#include <common.h>
#include <string.h>
#include <varint.h>

int hw_check_key(const unsigned char *key);

int hw_check_scalar(const unsigned char *scalar);

int hw_check_signature(
    const unsigned char *message_digest,
    const unsigned char *public_key,
    const unsigned char *signature);

int hw_check_ring_signatures(
    const unsigned char *tx_prefix_hash,
    const unsigned char *key_image,
    const unsigned char *public_keys,
    const unsigned char *signatures);

int hw_complete_ring_signature(
    unsigned char *signature,
    const unsigned char *tx_public_key,
    const size_t output_index,
    const unsigned char *output_key,
    const unsigned char *k,
    const unsigned char *privateView,
    const unsigned char *privateSpend,
    const unsigned char *publicSpend);

int hw_derive_public_key(
    unsigned char *key,
    const unsigned char *derivation,
    const size_t output_index,
    const unsigned char *publicSpend);

int hw_derive_secret_key(
    unsigned char *key,
    const unsigned char *derivation,
    const size_t output_index,
    const unsigned char *privateSpend);

int hw_generate_key_derivation(unsigned char *derivation, const unsigned char *public, const unsigned char *private);

int hw_generate_key_image(
    unsigned char *key_image,
    const unsigned char *tx_public_key,
    const size_t output_index,
    const unsigned char *output_key,
    const unsigned char *privateView,
    const unsigned char *privateSpend,
    const unsigned char *publicSpend);

int hw_generate_keypair(unsigned char *public, unsigned char *private);

int hw_generate_private_view_key(unsigned char *privateView, const unsigned char *privateSpend);

int hw_generate_ring_signatures(
    unsigned char *signatures,
    const unsigned char *tx_public_key,
    const size_t output_index,
    const unsigned char *output_key,
    const unsigned char *tx_prefix_hash,
    const unsigned char *public_keys,
    const size_t real_output_index,
    const unsigned char *privateView,
    const unsigned char *privateSpend,
    const unsigned char *publicSpend);

int hw_generate_signature(
    unsigned char *signature,
    const unsigned char *message_digest,
    const unsigned char *public_key,
    const unsigned char *private_key);

int hw_keccak(const unsigned char *in, size_t len, unsigned char *out);

int hw_private_key_to_public_key(unsigned char *public, const unsigned char *private);

int hw_retrieve_private_spend_key(unsigned char *private);

#endif // HW_CRYPTO_H
