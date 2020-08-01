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

#include "globals.h"

unsigned char G_working_set[WORKING_SET_SIZE] = {0};

unsigned char G_display_key_hex[KEY_HEXSTR_SIZE] = {0};

ux_state_t G_ux;

bolos_ux_params_t G_ux_params;

// display stepped screens
unsigned int ux_step;

unsigned int ux_step_count;
