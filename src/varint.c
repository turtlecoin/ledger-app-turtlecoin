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

#include "varint.h"

unsigned int ptrLength(const unsigned char *ptr)
{
    const unsigned char *orig = ptr;

    unsigned int len = 0;

    while (*ptr)
    {
        len++;

        UNUSED(*ptr++);
    }

    return (unsigned int)(ptr - orig);
}

unsigned int encode_varint(unsigned char *output, const uint64_t value, const size_t max_length)
{
    unsigned int length = 0;

    uint64_t val = value;

    while (val >= 0x80)
    {
        if (length == (max_length - 1))
        {
            THROW(ERR_VARINT_DATA_RANGE);
        }

        output[length] = (val & 0x7f) | 0x80;

        val = val >> 7;

        length++;
    }

    output[length] = val;

    return length + 1;
}

unsigned int decode_varint(const unsigned char *varint, const size_t max_length, uint64_t *value)
{
    uint64_t val = 0;

    unsigned int length = 0;

    while ((varint[length]) & 0x80)
    {
        if (length == (max_length - 1))
        {
            THROW(ERR_VARINT_DATA_RANGE);
        }

        val = val + (((varint[length]) & 0x7f) << (length * 7));

        length++;
    }

    val = val + (((varint[length]) & 0x7f) << (length * 7));

    *value = val;

    return length + 1;
}