/*****************************************************************************
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

#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <keys.h>

#define TX_MAX_INPUTS 90
#define TX_MAX_OUTPUTS 90
#define TX_MAX_SIZE 38400 // bytes
#define TX_EXTRA_MAX_SIZE 80 // bytes
#define TX_MAX_DUMP_SIZE 500 // bytes

#define TX_EXTRA_TAG_SIZE 1
#define TX_EXTRA_PUBKEY_TAG 0x01
#define TX_EXTRA_NONCE_TAG 0x02
#define TX_EXTRA_NONCE_PAYMENT_ID_TAG 0x00

#define TX_UNUSED 0x00
#define TX_READY 0x01
#define TX_RECEIVING_INPUTS 0x02
#define TX_INPUTS_RECEIVED 0x03
#define TX_RECEIVING_OUTPUTS 0x04
#define TX_OUTPUTS_RECEIVED 0x05
#define TX_PREFIX_READY 0x06
#define TX_COMPLETE 0x07

typedef unsigned char raw_transaction_t[TX_MAX_SIZE];

typedef struct transaction_input_s
{
    unsigned char public_keys[RING_PARTICIPANTS * KEY_SIZE]; // 128-bytes

    unsigned char private_ephemeral[KEY_SIZE]; // 32-bytes

    unsigned char key_image[KEY_SIZE]; // 32-bytes

    uint8_t real_output_index; // 1-byte
} transaction_input_t;

typedef struct transaction_info_s
{
    unsigned char tx_public_key[KEY_SIZE]; // 32-bytes

    unsigned char payment_id[KEY_SIZE]; // 32-bytes
} transaction_info_t;

typedef struct transaction_s // 24-bytes
{
    uint64_t total_input_amount; // 8-bytes

    uint64_t total_output_amount; // 8-bytes

    uint16_t current_position; // 2-bytes

    uint8_t has_payment_id; // 1-byte

    uint8_t input_count; // 1-byte

    uint8_t received_input_count; // 1-byte

    uint8_t output_count; // 1-byte

    uint8_t received_output_count; // 1-byte

    uint8_t state; // 1-byte
} transaction_t;

typedef transaction_input_t tx_pre_signatures_t[TX_MAX_INPUTS];

#ifdef TARGET_NANOX
extern const raw_transaction_t N_state_raw_transaction_pic;
extern const tx_pre_signatures_t N_state_pre_signatures_pic;
extern const transaction_info_t N_state_transaction_info_pic;
#define N_raw_transaction ((volatile raw_transaction_t *)PIC(&N_state_raw_transaction_pic))
#define N_tx_pre_signatures ((volatile tx_pre_signatures_t *)PIC(&N_state_pre_signatures_pic))
#define N_tx_info ((volatile tx_info_t *)PIC(&N_state_transaction_info_pic))
#else
extern raw_transaction_t N_state_raw_transaction_pic;
extern tx_pre_signatures_t N_state_pre_signatures_pic;
extern transaction_info_t N_state_transaction_info_pic;
#define N_raw_transaction ((WIDE raw_transaction_t *)PIC(&N_state_raw_transaction_pic))
#define N_tx_pre_signatures ((WIDE tx_pre_signatures_t *)PIC(&N_state_pre_signatures_pic))
#define N_tx_info ((WIDE transaction_info_t *)PIC(&N_state_transaction_info_pic))
#endif

uint16_t init_tx();

uint16_t tx_dump(unsigned char *out, const uint16_t start_offset, const uint16_t length);

uint64_t tx_fee();

uint16_t tx_finalize_prefix();

unsigned int tx_has_payment_id();

uint16_t tx_hash(unsigned char *hash);

uint64_t tx_input_amount();

uint16_t tx_load_input(
    const unsigned char *tx_public_key,
    const uint8_t output_index,
    const uint64_t amount,
    const unsigned char *public_keys,
    const uint32_t *offsets,
    const uint8_t real_output_index);

uint16_t tx_load_output(const uint64_t amount, const unsigned char *key);

uint64_t tx_output_amount();

void tx_payment_id(unsigned char *payment_id);

uint16_t tx_reset();

uint16_t tx_sign();

uint16_t tx_size();

uint16_t tx_start(
    const uint64_t unlock_time,
    const uint8_t input_count,
    const uint8_t output_count,
    const unsigned char *tx_public_key,
    const uint8_t has_payment_id,
    const unsigned char *payment_id);

uint16_t tx_start_input_load();

uint16_t tx_start_output_load();

unsigned int tx_state();

#endif // TRANSACTION_H
