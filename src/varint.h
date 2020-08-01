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

#ifndef VARINT_H
#define VARINT_H

#include <common.h>

unsigned int ptrLength(const unsigned char *ptr);

unsigned int encode_varint(unsigned char *output, const uint64_t value, const size_t max_length);

unsigned int decode_varint(const unsigned char *varint, const size_t max_length, uint64_t *value);

#endif // VARINT_H
