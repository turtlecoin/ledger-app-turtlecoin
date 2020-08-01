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

#ifndef BASE58_H
#define BASE58_H

#include <stddef.h>
#include <stdint.h>

#define RAW_ADDRESS_SIZE 72
#define BASE58_ADDRESS_SIZE 99
#define CHECKSUM_SIZE 4
#define BASE58_ADDRESS_STR_SIZE BASE58_ADDRESS_SIZE + 1

int base58_encode(const unsigned char *rawAddress, unsigned char *str_b58);

#endif // BASE58_H
