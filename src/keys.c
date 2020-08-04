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

#include "keys.h"

static const unsigned char W_MAGIC[KEY_SIZE] = {0x54, 0x75, 0x72, 0x74, 0x6c, 0x65, 0x43, 0x6f, 0x69, 0x6e, 0x20,
                                                0x69, 0x73, 0x20, 0x6e, 0x6f, 0x74, 0x20, 0x61, 0x20, 0x4d, 0x6f,
                                                0x6e, 0x65, 0x72, 0x6f, 0x20, 0x66, 0x6f, 0x72, 0x6b, 0x21};

static const uint8_t prefix[4] = {157, 246, 238, 1};

#ifdef TARGET_NANOX
const wallet_t N_state_pic;
#else
wallet_t N_state_pic;
#endif

int reset_keys()
{
    BEGIN_TRY
    {
        TRY
        {
            PRINTF("Resetting keys...\n");

            // Zero out the wallet structure in NVRAM
            nvm_write((void *)N_turtlecoin_wallet, NULL, sizeof(wallet_t));

            // Then reinitialize the keys
            init_keys();

            CLOSE_TRY;

            return 0;
        }
        CATCH_OTHER(e)
        {
            return 1;
        }
        FINALLY {}
    }
    END_TRY;
}

int init_keys()
{
    /**
     * Check to see if our wallet information already exists in the secure NVRAM
     * of the ledger hardware device, if not, then we need to initialize the
     * wallet This limits writes to NVRAM to on first boot OR on key reset only 7
     * writes to NVRAM each time it is used: wipe, private spend, public spend,
     * private view, public view, address, magic
     */
    if (os_memcmp((void *)N_turtlecoin_wallet->magic, (void *)W_MAGIC, sizeof(W_MAGIC)) != 0)
    {
        wallet_t wallet;

        BEGIN_TRY
        {
            TRY
            {
                // Zero out enough space in NVRAM for the size of our wallet structure
                nvm_write((void *)N_turtlecoin_wallet, NULL, sizeof(wallet_t));

                // Retrieve the private spend key for which all things are made
                if (hw_retrieve_private_spend_key(wallet.spend.private) != 0)
                {
                    THROW(ERR_PRIVATE_SPEND);
                }

                // Generate the public spend key from the private spend key
                if (hw_private_key_to_public_key(wallet.spend.public, wallet.spend.private) != 0)
                {
                    THROW(ERR_SECKEY_TO_PUBKEY);
                }

                // Generate the private view key from the private spend key
                if (hw_generate_private_view_key(wallet.view.private, wallet.spend.private) != 0)
                {
                    THROW(ERR_PRIVATE_VIEW);
                }

                // Generate the public view key from the private view key
                if (hw_private_key_to_public_key(wallet.view.public, wallet.view.private) != 0)
                {
                    THROW(ERR_SECKEY_TO_PUBKEY);
                }

                /**
                 * Write the magic bytes to the structure in RAM so that upon
                 * the next application load we do not have to perform these
                 * operations again
                 */
                os_memmove(wallet.magic, W_MAGIC, sizeof(W_MAGIC));

                // write the wallet structure to NVRAM
                nvm_write((void *)N_turtlecoin_wallet, (void *)&wallet, sizeof(wallet_t));

                CLOSE_TRY;

                return 0;
            }
            CATCH_OTHER(e)
            {
                return 1;
            }
            FINALLY
            {
                // Explicitly clear the working memory
                explicit_bzero(&wallet, sizeof(wallet_t));
            }
            END_TRY;
        }
    }
    else
    {
        return 0;
    }
}

int generate_public_address(const unsigned char *publicSpend, const unsigned char *publicView, unsigned char *address)
{
    BEGIN_TRY
    {
        TRY
        {
            unsigned char buffer[68] = {0};

            unsigned char hash[KEY_SIZE] = {0};

            int pos = 0;

            // Copy the prefix on to the front of the buffer
            os_memmove(buffer + pos, prefix, sizeof(prefix));
            pos += sizeof(prefix);

            // Copy the public spend key to the buffer
            os_memmove(buffer + pos, publicSpend, KEY_SIZE);
            pos += KEY_SIZE;

            // Copy the public view key to the buffer
            os_memmove(buffer + pos, publicView, KEY_SIZE);
            pos += KEY_SIZE;

            // Hash the buffer to generate the checksum
            hw_keccak(buffer, pos, hash);

            // Copy the checksum to the buffer
            os_memmove(buffer + pos, hash, CHECKSUM_SIZE);

            // Encode the buffer as Base58
            base58_encode(buffer, address);

            CLOSE_TRY;

            return 0;
        }
        CATCH_OTHER(e)
        {
            return 1;
        }
        FINALLY {}
    }
    END_TRY;
}