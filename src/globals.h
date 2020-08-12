/*******************************************************************************
 *   Ledger Blue
 *   (c) 2016 Ledger
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
 ********************************************************************************/

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <common.h>

#define RING_PARTICIPANTS 4
#define BIP32_PATH 5
#define KEY_SIZE 32
#define KEY_STR_SIZE KEY_SIZE + 1
#define KEY_HEXSTR_SIZE (KEY_SIZE * 2) + 1
#define SIG_SIZE 64
#define SIG_STR_SIZE SIG_SIZE + 1
#define SIG_SET_SIZE SIG_SIZE *RING_PARTICIPANTS
#define KECCAK_BITS 256
#define DECIMAL_PLACES 2
#define TICKER ((unsigned char *)" TRTL")
#define TICKER_SIZE 5

#define P1_CONFIRM 0x01
#define P1_NON_CONFIRM 0x00
#define P1_FIRST 0x00
#define P1_MORE 0x80
#define WORKING_SET_SIZE 480 // reserve 480 bytes of working memory for handling APU inputs

extern unsigned char G_working_set[WORKING_SET_SIZE];

#define WORKING_SET ((unsigned char *)G_working_set)

extern unsigned char G_display_key_hex[KEY_HEXSTR_SIZE];

#define DISPLAY_KEY_HEX ((unsigned char *)G_display_key_hex)

extern ux_state_t ux;

// display stepped screens
extern unsigned int ux_step;

extern unsigned int ux_step_count;

#endif
