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

#ifndef KEYS_H
#define KEYS_H

#include <base58.h>
#include <common.h>
#include <hw_crypto.h>

typedef struct key_pair_s
{
    unsigned char private[KEY_SIZE]; // 32-bytes

    unsigned char public[KEY_SIZE]; // 32-bytes
} key_pair_t;

typedef struct wallet_s
{
    key_pair_t spend; // 64-bytes

    key_pair_t view; // 64-bytes

    unsigned char magic[KEY_SIZE]; // 32-bytes
} wallet_t;

#ifdef TARGET_NANOX
extern const wallet_t N_state_pic;
#define N_turtlecoin_wallet ((volatile wallet_t *)PIC(&N_state_pic))
#else
extern wallet_t N_state_pic;
#define N_turtlecoin_wallet ((WIDE wallet_t *)PIC(&N_state_pic))
#endif

#define PTR_SPEND_PUBLIC ((unsigned char *)N_turtlecoin_wallet->spend.public)
#define PTR_SPEND_PRIVATE ((unsigned char *)N_turtlecoin_wallet->spend.private)
#define PTR_VIEW_PUBLIC ((unsigned char *)N_turtlecoin_wallet->view.public)
#define PTR_VIEW_PRIVATE ((unsigned char *)N_turtlecoin_wallet->view.private)

uint16_t init_keys();

uint16_t reset_keys();

uint16_t
    generate_public_address(const unsigned char *publicSpend, const unsigned char *publicView, unsigned char *address);

#endif // KEYS_H
