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

#ifndef ERROR_CODES_H
#define ERROR_CODES_H

#define ERR_STR ((unsigned char *)"ERROR")

#define ERR_OP_NOT_PERMITTED 0x4000
#define ERR_OP_USER_REQUIRED 0x4001
#define ERR_WRONG_INPUT_LENGTH 0x4002
#define ERR_UNKNOWN_ERROR 0x4444

#define ERR_VARINT_DATA_RANGE 0x6000

#define ERR_PRIVATE_SPEND 0x9400
#define ERR_PRIVATE_VIEW 0x9401
#define ERR_RESET_KEYS 0x9402

#define ERR_ADDRESS 0x9450

#define ERR_KEY_DERIVATION 0x9500
#define ERR_DERIVE_PUBKEY 0x9501
#define ERR_PUBKEY_MISMATCH 0x9502
#define ERR_DERIVE_SECKEY 0x9503
#define ERR_KECCAK 0x9504
#define ERR_COMPLETE_RING_SIG 0x9505
#define ERR_GENERATE_KEY_IMAGE 0x9506
#define ERR_SECKEY_TO_PUBKEY 0x9507
#define ERR_GENERATE_RING_SIGS 0x9508
#define ERR_GENERATE_SIGNATURE 0x9509
#define ERR_PRIVATE_TO_PUBLIC 0x9510

#endif // ERROR_CODES_H