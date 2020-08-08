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

#include "base58.h"

const char alphabet[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

#define alphabet_size (sizeof(alphabet) - 1)

const unsigned int encoded_block_sizes[] = {0, 2, 3, 5, 6, 7, 9, 10, 11};

#define FULL_BLOCK_SIZE 8 //(sizeof(encoded_block_sizes) / sizeof(encoded_block_sizes[0]) - 1)
#define FULL_ENCODED_BLOCK_SIZE 11 // encoded_block_sizes[full_block_size];
#define ADDR_CHECKSUM_SIZE 4

static uint64_t uint_8be_to_64(const unsigned char *data, size_t size)
{
    uint64_t res = 0;
    switch (9 - size)
    {
        case 1:
            res |= *data++;
        case 2:
            res <<= 8;
            res |= *data++;
        case 3:
            res <<= 8;
            res |= *data++;
        case 4:
            res <<= 8;
            res |= *data++;
        case 5:
            res <<= 8;
            res |= *data++;
        case 6:
            res <<= 8;
            res |= *data++;
        case 7:
            res <<= 8;
            res |= *data++;
        case 8:
            res <<= 8;
            res |= *data;
            break;
    }

    return res;
}

static void encode_block(const unsigned char *block, size_t size, unsigned char *res)
{
    uint64_t num = uint_8be_to_64(block, size);

    int i = encoded_block_sizes[size];

    while (i--)
    {
        uint64_t remainder = num % alphabet_size;

        num /= alphabet_size;

        res[i] = alphabet[remainder];
    }
}

uint16_t base58_encode(const unsigned char *rawAddress, unsigned char *str_b58)
{
    BEGIN_TRY
    {
        TRY
        {
            unsigned int full_block_count = RAW_ADDRESS_SIZE / FULL_BLOCK_SIZE;

            unsigned int last_block_size = RAW_ADDRESS_SIZE % FULL_BLOCK_SIZE;

            for (size_t i = 0; i < full_block_count; ++i)
            {
                encode_block(rawAddress + i * FULL_BLOCK_SIZE, FULL_BLOCK_SIZE, &str_b58[i * FULL_ENCODED_BLOCK_SIZE]);
            }

            if (0 < last_block_size)
            {
                encode_block(
                    rawAddress + full_block_count * FULL_BLOCK_SIZE,
                    last_block_size,
                    &str_b58[full_block_count * FULL_ENCODED_BLOCK_SIZE]);
            }

            return OP_OK;
        }
        CATCH_OTHER(e)
        {
            THROW(ERR_BASE58);
        }
        FINALLY {};
    }
    END_TRY;
}