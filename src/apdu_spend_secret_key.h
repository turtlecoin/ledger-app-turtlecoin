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

#ifndef APDU_SPEND_SECRET_KEY_H
#define APDU_SPEND_SECRET_KEY_H

#include <stdint.h>

#define APDU_SPEND_SECRET_KEY_NAME ((unsigned char *)"SPENDSECRET")

void handle_spend_secret_key(uint8_t p1, uint8_t p2, volatile unsigned int *flags, volatile unsigned int *tx);

#endif // APDU_SPEND_SECRET_KEY_H
