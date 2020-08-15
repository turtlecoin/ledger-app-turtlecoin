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

#ifndef APDU_H
#define APDU_H

#include <apdu_address.h>
#include <apdu_check_key.h>
#include <apdu_check_ringsignatures.h>
#include <apdu_check_scalar.h>
#include <apdu_check_signature.h>
#include <apdu_complete_ringsignature.h>
#include <apdu_debug.h>
#include <apdu_derive_public_key.h>
#include <apdu_derive_secret_key.h>
#include <apdu_generate_key_derivation.h>
#include <apdu_generate_keyimage.h>
#include <apdu_generate_keyimage_primitive.h>
#include <apdu_generate_ringsignatures.h>
#include <apdu_generate_signature.h>
#include <apdu_ident.h>
#include <apdu_private_to_public.h>
#include <apdu_public_keys.h>
#include <apdu_random_key_pair.h>
#include <apdu_reset_keys.h>
#include <apdu_spend_secret_key.h>
#include <apdu_tx_dump.h>
#include <apdu_tx_finalize_prefix.h>
#include <apdu_tx_input_load.h>
#include <apdu_tx_output_load.h>
#include <apdu_tx_reset.h>
#include <apdu_tx_sign.h>
#include <apdu_tx_start.h>
#include <apdu_tx_start_input_load.h>
#include <apdu_tx_start_output_load.h>
#include <apdu_tx_state.h>
#include <apdu_version.h>
#include <apdu_view_secret_key.h>
#include <apdu_view_wallet_keys.h>

/**
 * @returns major || minor || patch {3 bytes}
 */
#define APDU_VERSION 0x01

/**
 * @returns debug {1 byte}
 */
#define APDU_DEBUG 0x02

/**
 * @returns ident_value {32 bytes}
 */
#define APDU_IDENT 0x05

/**
 * @returns spend_public_key || view_public_key {64 bytes}
 */
#define APDU_PUBLIC_KEYS 0x10

/**
 * @returns view_secret_key {32 bytes}
 */
#define APDU_VIEW_SECRET_KEY 0x11

/**
 * @returns spend_secret_key {32 bytes}
 */
#define APDU_SPEND_SECRET_KEY 0x12

/**
 * @returns spend_public_key || view_private_key {64 bytes}
 */
#define APDU_VIEW_WALLET_KEYS 0x13

/**
 * @param public_key {32 bytes}
 * @returns valid {1 byte}
 */
#define APDU_CHECK_KEY 0x16

/**
 * @param private_key {32 bytes}
 * @returns valid {1 byte}
 */
#define APDU_CHECK_SCALAR 0x17

/**
 * @param private_key {32 bytes}
 * @returns public_key {32 bytes}
 */
#define APDU_PRIVATE_TO_PUBLIC 0x18

/**
 * @returns public_key || private_key {64 bytes}
 */
#define APDU_RANDOM_KEY_PAIR 0x19

/**
 * @returns wallet_address {99 bytes}
 */
#define APDU_ADDRESS 0x30

/**
 * Input payload of 68 bytes
 *
 * @param tx_public_key {32 bytes}
 * @param output_index {4 bytes}
 * @param output_key {32 bytes}
 * @returns key_image {32 bytes}
 */
#define APDU_GENERATE_KEYIMAGE 0x40

/**
 * Input payload of 68 bytes
 *
 * @param derivation {32 bytes}
 * @param output_index {4 bytes}
 * @param output_key {32 bytes}
 * @returns key_image {32 bytes}
 */
#define APDU_GENERATE_KEYIMAGE_PRIMITIVE 0x41

/**
 * Input payload of 232 bytes
 *
 * @param tx_public_key {32 bytes}
 * @param output_index {4 bytes}
 * @param output_key {32 bytes}
 * @param tx_prefix_hash {32 bytes}
 * @param input_keys[] {32 bytes * 4}
 * @param real_output_index {4 bytes}
 * @returns signatures[] {64 bytes * 4 = 256 bytes}
 */
#define APDU_GENERATE_RING_SIGNATURES 0x50

/**
 * Input payload of 164 bytes
 *
 * @param tx_public_key {32 bytes}
 * @param output_index {4 bytes}
 * @param output_key {32 bytes}
 * @param k (random key used in prepare ring signatures) {32 bytes}
 * @param signature (signature to complete) {64 bytes}
 * @returns signature {64 bytes}
 */
#define APDU_COMPLETE_RING_SIGUATURE 0x51

/**
 * Input payload of 448 bytes
 *
 * @param tx_public_key {32 bytes}
 * @param key_image {32 bytes}
 * @param public_keys {32 bytes * 4}
 * @param signatures {64 bytes * 4}
 * @returns !valid {1 byte}
 */
#define APDU_CHECK_RING_SIGNATURES 0x52

/**
 * @param message_digest {32 bytes}
 * @returns signature {64 bytes}
 */
#define APDU_GENERATE_SIGNATURE 0x55

/**
 * Input payload of 128 bytes
 *
 * @param message_digest {32 bytes}
 * @param public_key {32 bytes}
 * @param signature {64 bytes}
 * @returns !valid {1 byte}
 */
#define APDU_CHECK_SIGNATURE 0x56

/**
 * @param tx_public_key
 * @returns key_derivation {32 bytes}
 */
#define APDU_GENERATE_KEY_DERIVATION 0x60

/**
 * Input payload of 36 bytes
 *
 * @param derivation {32 bytes}
 * @param output_index {4 bytes}
 * @returns publicEphemeral {32 bytes}
 */
#define APDU_DERIVE_PUBLIC_KEY 0x61

/**
 * Input payload of 36 bytes
 *
 * @param derivation {32 bytes}
 * @param output_index {4 bytes}
 * @returns privateEphemeral {32 bytes}
 */
#define APDU_DERIVE_SECRET_KEY 0x62

/**
 * @returns state {1 byte}
 */
#define APDU_TX_STATE 0x70

/**
 * Input payload of 43 bytes OR 75 bytes
 *
 * @param unlock_time {8 bytes}
 * @param input_count {1 byte}
 * @param output_count {1 byte}
 * @param tx_public_key {32 bytes}
 * @param has_payment_id {1 byte}
 * @param payment_id {32 bytes} (optional)
 */
#define APDU_TX_START 0x71

/**
 * @returns
 */
#define APDU_TX_START_INPUT_LOAD 0x72

/**
 * Input payload of 185 bytes
 *
 * @param input_tx_public_key {32 bytes}
 * @param input_output_index {1 byte}
 * @param amount {8 bytes}
 * @param public_keys {32 bytes * 4} (ring participant public keys)
 * @param offsets {4 bytes * 4} (relative global index offsets)
 * @param real_output_index {1 byte}
 */
#define APDU_TX_LOAD_INPUT 0x73

/**
 * @returns
 */
#define APDU_TX_START_OUTPUT_LOAD 0x74

/**
 * @param amount {8 bytes}
 * @param key {32 bytes}
 */
#define APDU_TX_LOAD_OUTPUT 0x75

/**
 * @returns
 */
#define APDU_TX_FINALIZE_PREFIX 0x76

/**
 * @returns tx_hash || tx_size {34 bytes}
 */
#define APDU_TX_SIGN 0x77

/**
 * @param start_offset {2 bytes}
 * @returns raw_transaction {0 - 500 bytes}
 */
#define APDU_TX_DUMP 0x78

/**
 * @returns
 */
#define APDU_TX_RESET 0x79

/**
 * @returns nothing
 */
#define APDU_RESET_KEYS 0xff

#endif // APDU_H
