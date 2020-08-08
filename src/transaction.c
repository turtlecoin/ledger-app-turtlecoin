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

#include "transaction.h"

#include <keys.h>
#include <varint.h>

#ifdef TARGET_NANOX
const raw_transaction_t N_state_raw_transaction_pic;
const tx_pre_signatures_t N_state_pre_signatures_pic;
const transaction_info_t N_state_transaction_info_pic;
#else
raw_transaction_t N_state_raw_transaction_pic;
tx_pre_signatures_t N_state_pre_signatures_pic;
transaction_info_t N_state_transaction_info_pic;
#endif

// locally stored meta data about the current transaction construction
static transaction_t L_transaction;

#define TX_RESET()                                           \
    nvm_write((void *)N_raw_transaction, NULL, TX_MAX_SIZE); \
    L_transaction.current_position = 0;

#define TX_WRITE(payload, length)                                                                    \
    nvm_write((void *)N_raw_transaction + L_transaction.current_position, (void *)&payload, length); \
    L_transaction.current_position += length

#define TX_WRITE_PTR(payload, length)                                                               \
    nvm_write((void *)N_raw_transaction + L_transaction.current_position, (void *)payload, length); \
    L_transaction.current_position += length

#define PRE_SIG_RESET() nvm_write((void *)N_tx_pre_signatures, NULL, sizeof(tx_pre_signatures_t))

#define PRE_SIG_WRITE(payload)                                           \
    nvm_write(                                                           \
        (void *)N_tx_pre_signatures[L_transaction.received_input_count], \
        (void *)&payload,                                                \
        sizeof(transaction_input_t))

#define TX_INFO_RESET() nvm_write((void *)N_tx_info, NULL, sizeof(transaction_info_t))

#define TX_INFO_WRITE(payload) nvm_write((void *)N_tx_info, (void *)&payload, sizeof(transaction_info_t))

/**
 * Initializes our internal transaction structure that holds
 * some basic values that are used to navigate our transaction
 * state while it is in memory somewhere
 * @return
 */
uint16_t init_tx()
{
    BEGIN_TRY
    {
        TRY
        {
            L_transaction.total_input_amount = 0;

            L_transaction.total_output_amount = 0;

            L_transaction.has_payment_id = 0;

            L_transaction.input_count = 0;

            L_transaction.received_input_count = 0;

            L_transaction.output_count = 0;

            L_transaction.received_output_count = 0;

            L_transaction.state = TX_UNUSED;

            CLOSE_TRY;

            return OP_OK;
        }
        CATCH_OTHER(e)
        {
            return ERR_TX_INIT;
        }
        FINALLY {}
    }
    END_TRY;
}

/**
 * Dumps the raw transaction to the given unsigned character array
 * @param out the pointer to dump to
 * @param start_offset the starting offset
 */
uint16_t tx_dump(unsigned char *out, const uint16_t start_offset, const uint16_t length)
{
    BEGIN_TRY
    {
        TRY
        {
            os_memmove(out, (unsigned char *)N_raw_transaction + start_offset, length);

            CLOSE_TRY;

            return OP_OK;
        }
        CATCH_OTHER(e)
        {
            return ERR_TX_DUMP;
        }
        FINALLY {}
    }
    END_TRY;
}

/**
 * Returns the transaction network fee
 */
uint64_t tx_fee()
{
    return (tx_input_amount() - tx_output_amount());
}

/**
 * Finalizes a transaction prefix by constructing the TX_EXTRA field
 */
uint16_t tx_finalize_prefix()
{
    // check to make sure we are in the correct state
    if (tx_state() != TX_OUTPUTS_RECEIVED)
    {
        return ERR_TRANSACTION_STATE;
    }

    // Are we trying to create more in outputs than we supplied in inputs?
    if (tx_output_amount() > tx_input_amount())
    {
        return ERR_TX_AMOUNT;
    }

    unsigned char extra[TX_EXTRA_MAX_SIZE] = {0};

    BEGIN_TRY
    {
        TRY
        {
            unsigned int pos = 0;

            // figure out how long the extra field will be
            {
                unsigned int extra_size = TX_EXTRA_TAG_SIZE + KEY_SIZE; // include the public key at minimum

                if (L_transaction.has_payment_id == 1)
                {
                    // nonce_tag + size + paymentid_tag + key
                    extra_size += TX_EXTRA_TAG_SIZE + TX_EXTRA_TAG_SIZE + TX_EXTRA_TAG_SIZE + KEY_SIZE;
                }

                pos = encode_varint(extra, extra_size, sizeof(extra));
            }

            // write the tx public key to extra
            {
                unsigned char tag = TX_EXTRA_PUBKEY_TAG;

                os_memmove(extra + pos, &tag, TX_EXTRA_TAG_SIZE);

                pos += TX_EXTRA_TAG_SIZE;

                os_memmove(extra + pos, N_tx_info->tx_public_key, KEY_SIZE);

                pos += KEY_SIZE;
            }

            if (L_transaction.has_payment_id == 1)
            {
                // write the nonce tag to extra
                {
                    unsigned char tag = TX_EXTRA_NONCE_TAG;

                    os_memmove(extra + pos, &tag, TX_EXTRA_TAG_SIZE);

                    pos += TX_EXTRA_TAG_SIZE;
                }

                // write the size of the nonce field in extra
                {
                    pos += encode_varint(extra + pos, TX_EXTRA_TAG_SIZE + KEY_SIZE, TX_EXTRA_TAG_SIZE);
                }

                // write the payment id key to extra
                {
                    unsigned char tag = TX_EXTRA_NONCE_PAYMENT_ID_TAG;

                    os_memmove(extra + pos, &tag, TX_EXTRA_TAG_SIZE);

                    pos += TX_EXTRA_TAG_SIZE;

                    os_memmove(extra + pos, N_tx_info->payment_id, KEY_SIZE);

                    pos += KEY_SIZE;
                }
            }

            // batch write to NVRAM
            TX_WRITE(extra, pos);

            L_transaction.state = TX_PREFIX_READY;

            CLOSE_TRY;

            return OP_OK;
        }
        CATCH_OTHER(e)
        {
            return ERR_TX_FINALIZE_PREFIX;
        }
        FINALLY
        {
            explicit_bzero(extra, sizeof(extra));
        }
    }
    END_TRY;
}

/**
 * Returns if the transaction uses a payment id
 */
unsigned int tx_has_payment_id()
{
    return (unsigned int)L_transaction.has_payment_id;
}

/**
 * Returns the transaction hash
 * @param hash the pointer to put the hash into
 */
uint16_t tx_hash(unsigned char *hash)
{
    BEGIN_TRY
    {
        TRY
        {
            const uint16_t status = hw_keccak((unsigned char *)N_raw_transaction, L_transaction.current_position, hash);

            if (status != OP_OK)
            {
                THROW(status);
            }

            CLOSE_TRY;

            return OP_OK;
        }
        CATCH_OTHER(e)
        {
            return e;
        }
        FINALLY {}
    }
    END_TRY;
}

/**
 * Returns the total amount of the inputs
 */
uint64_t tx_input_amount()
{
    return L_transaction.total_input_amount;
}

/**
 * Loads a transaction input
 * @param tx_public_key
 * @param output_index
 * @param amount
 * @param public_keys
 * @param offsets
 * @param real_output_index
 */
uint16_t tx_load_input(
    const unsigned char *tx_public_key,
    const uint8_t output_index,
    const uint64_t amount,
    const unsigned char *public_keys,
    const uint32_t *offsets,
    const uint8_t real_output_index)
{
    // return an error if we are not in the correct state
    if (tx_state() != TX_RECEIVING_INPUTS)
    {
        return ERR_TRANSACTION_STATE;
    }

    unsigned char tx[TX_EXTRA_MAX_SIZE] = {0}; // 80-bytes

    transaction_input_t tx_input; // 193-bytes

// we are shadowing these into the tx_input structure to save memory
#define DERIVATION (unsigned char *)&tx_input.public_keys
#define PUBLIC_EPHEMERAL DERIVATION + KEY_SIZE
#define PUBLIC_EPHEMERAL2 PUBLIC_EPHEMERAL + KEY_SIZE

    BEGIN_TRY
    {
        TRY
        {
            unsigned int pos = 0;

            // generate the key derivation using our private view key and the transaction public key
            uint16_t status = hw_generate_key_derivation(DERIVATION, tx_public_key, PTR_VIEW_PRIVATE);

            if (status != OP_OK)
            {
                THROW(status);
            }

            // derive the public ephemeral
            status = hw_derive_public_key(PUBLIC_EPHEMERAL, DERIVATION, output_index, PTR_SPEND_PUBLIC);

            if (status != OP_OK)
            {
                THROW(status);
            }

            // check to make sure that the calculated output key is in the position specified
            status = os_memcmp(PUBLIC_EPHEMERAL, public_keys + (real_output_index * KEY_SIZE), KEY_SIZE);

            if (status != OP_OK)
            {
                THROW(status);
            }

            // derive the private ephemeral
            status = hw_derive_secret_key(tx_input.private_ephemeral, DERIVATION, output_index, PTR_SPEND_PRIVATE);

            if (status != OP_OK)
            {
                THROW(status);
            }

            // calculate the public key of the private ephemeral for matching down below
            status = hw_private_key_to_public_key(PUBLIC_EPHEMERAL2, tx_input.private_ephemeral);

            if (status != OP_OK)
            {
                THROW(status);
            }

            // compare the derived public ephemeral against the one calculated by the private ephemeral
            status = os_memcmp(PUBLIC_EPHEMERAL, PUBLIC_EPHEMERAL2, KEY_SIZE);

            if (status != OP_OK)
            {
                THROW(status);
            }

            // generate the input key image using the public ephemeral and private ephemeral
            status = hw__generate_key_image(tx_input.key_image, PUBLIC_EPHEMERAL, tx_input.private_ephemeral);

            if (status != OP_OK)
            {
                THROW(status);
            }

            // write the input type to the transaction prefix
            {
                unsigned char type = 0x02;

                os_memmove(tx, &type, TX_EXTRA_TAG_SIZE);

                pos += TX_EXTRA_TAG_SIZE;
            }

            // write the input amount to the transaction prefix
            {
                pos += encode_varint(tx + pos, amount, sizeof(tx));
            }

            L_transaction.total_input_amount += amount;

            // write number of global index offsets to the transaction prefix
            {
                unsigned char offset_length = RING_PARTICIPANTS;

                os_memmove(tx + pos, &offset_length, sizeof(tx));

                pos += TX_EXTRA_TAG_SIZE;
            }

            // write the input offsets to the transaction prefix
            {
                int i;
                for (i = 0; i < RING_PARTICIPANTS; i++)
                {
                    pos += encode_varint(tx + pos, offsets[i], sizeof(tx));
                }
            }

            // write the key image to the transaction prefix
            {
                os_memmove(tx + pos, tx_input.key_image, KEY_SIZE);

                pos += KEY_SIZE;
            }

            // batch write to NVRAM
            TX_WRITE(tx, pos);

            /**
             * There's some information that we need to save off for when we generate the
             * ring signatures at the end of the transaction construction process
             * these parts are not included in the transaction itself that is broadcasted
             * to the network so we'll throw them over in NVRAM as a nice structure for
             * later when we need them as we need to limit RAM usage
             */
            {
                os_memmove(tx_input.public_keys, public_keys, RING_PARTICIPANTS * KEY_SIZE);

                tx_input.real_output_index = real_output_index;

                PRE_SIG_WRITE(tx_input);
            }

            L_transaction.received_input_count++;

            // if we've now received all of the inputs that we expected, change the transaction state
            if (L_transaction.received_input_count == L_transaction.input_count)
            {
                L_transaction.state = TX_INPUTS_RECEIVED;
            }

            CLOSE_TRY;

            return OP_OK;
        }
        CATCH_OTHER(e)
        {
            return e;
        }
        FINALLY
        {
#undef PUBLIC_EPHEMERAL
#undef DERIVATION
            explicit_bzero((void *)&tx_input, sizeof(transaction_input_t));

            explicit_bzero(tx, sizeof(tx));
        }
    }
    END_TRY;
}

/**
 * Loads an output into the transaction
 * @param amount
 * @param key
 */
uint16_t tx_load_output(const uint64_t amount, const unsigned char *key)
{
    if (tx_state() != TX_RECEIVING_OUTPUTS)
    {
        return ERR_TRANSACTION_STATE;
    }

    unsigned char tx[TX_EXTRA_MAX_SIZE] = {0};

    BEGIN_TRY
    {
        TRY
        {
            unsigned int pos = 0;

            // write the amount to the transaction
            {
                pos += encode_varint(tx, amount, sizeof(tx));
            }

            // write the type to the transaction
            {
                unsigned char type = 0x02;

                os_memmove(tx + pos, &type, TX_EXTRA_TAG_SIZE);

                pos += TX_EXTRA_TAG_SIZE;
            }

            // write the key to the transaction
            {
                os_memmove(tx + pos, key, KEY_SIZE);

                pos += KEY_SIZE;
            }

            // batch write to NVRAM
            TX_WRITE(tx, pos);

            L_transaction.received_output_count++;

            // if we have now received all of the outputs that we were expecting update the transaction state
            if (L_transaction.received_output_count == L_transaction.output_count)
            {
                L_transaction.state = TX_OUTPUTS_RECEIVED;
            }

            CLOSE_TRY;

            return OP_OK;
        }
        CATCH_OTHER(e)
        {
            return ERR_TX_LOAD_OUTPUT;
        }
        FINALLY
        {
            explicit_bzero(tx, sizeof(tx));
        }
    }
    END_TRY;
}

/**
 * Returns the total amount of the outputs
 */
uint64_t tx_output_amount()
{
    return L_transaction.total_output_amount;
}

/**
 * Returns the payment id information
 */
void tx_payment_id(unsigned char *payment_id)
{
    os_memmove(payment_id, N_tx_info->payment_id, KEY_SIZE);
}

/**
 * Resets the internal transaction state of the device
 */
uint16_t tx_reset()
{
    BEGIN_TRY
    {
        TRY
        {
            TX_RESET();

            PRE_SIG_RESET();

            TX_INFO_RESET();

            if (init_tx() != 0)
            {
                THROW(ERR_TX_RESET);
            }

            CLOSE_TRY;

            return OP_OK;
        }
        CATCH_OTHER(e)
        {
            return e;
        }
        FINALLY {}
    }
    END_TRY;
}

/**
 * Completes the ring signatures for the transaction currently in memory
 */
uint16_t tx_sign()
{
    if (tx_state() != TX_PREFIX_READY)
    {
        return ERR_TRANSACTION_STATE;
    }

#define SIGNATURES WORKING_SET
#define PREFIX_HASH SIGNATURES + (SIG_SIZE * RING_PARTICIPANTS)

    BEGIN_TRY
    {
        TRY
        {
            // calculate the transaction prefix hash and hold it for later
            uint16_t status =
                hw_keccak((unsigned char *)N_raw_transaction, L_transaction.current_position, PREFIX_HASH);

            if (status != OP_OK)
            {
                THROW(status);
            }

            int i;
            for (i = 0; i < L_transaction.input_count; i++)
            {
                status = hw__generate_ring_signatures(
                    SIGNATURES,
                    PREFIX_HASH,
                    N_tx_pre_signatures[i]->key_image,
                    N_tx_pre_signatures[i]->public_keys,
                    N_tx_pre_signatures[i]->private_ephemeral,
                    N_tx_pre_signatures[i]->real_output_index);

                if (status != OP_OK)
                {
                    THROW(status);
                }

                /**
                 * Normally, we'd try to batch the write to NVRAM but due to memory constraints
                 * and the fact that each signature set is 256-bytes, we need to commit the
                 * signatures to NVRAM during each pass of the loop as there can be 90 inputs
                 * processed and 90 * 256 = 23,040 bytes -- too many
                 */
                TX_WRITE_PTR(SIGNATURES, SIG_SIZE * RING_PARTICIPANTS);
            }

            L_transaction.state = TX_COMPLETE;

            CLOSE_TRY;

            return OP_OK;
        }
        CATCH_OTHER(e)
        {
            return e;
        }
        FINALLY
        {
#undef PREFIX_HASH
#undef SIGNATURES
            explicit_bzero(WORKING_SET, WORKING_SET_SIZE);
        }
    }
    END_TRY;
}

/**
 * Returns the current size of the transaction in memory
 */
uint16_t tx_size()
{
    return L_transaction.current_position;
}

/**
 * Initializes a new transaction
 * @param unlock_time
 * @param input_count
 * @param output_count
 * @param tx_public_key
 * @param has_payment_id
 * @param payment_id
 */
uint16_t tx_start(
    const uint64_t unlock_time,
    const uint8_t input_count,
    const uint8_t output_count,
    const unsigned char *tx_public_key,
    const uint8_t has_payment_id,
    const unsigned char *payment_id)
{
    unsigned char tx[KEY_SIZE];

    BEGIN_TRY
    {
        TRY
        {
            unsigned int pos = 0;

            if (tx_reset() != 0)
            {
                THROW(ERR_TX_RESET);
            }

            // validate that we are not trying to start a transaction with more than we can handle
            if (input_count > TX_MAX_INPUTS || output_count > TX_MAX_OUTPUTS)
            {
                THROW(ERR_TX_INPUT_OUTPUT_OUT_OF_RANGE);
            }

            // write the transaction version to the transaction data
            {
                unsigned char version = 1;

                os_memmove(tx, &version, TX_EXTRA_TAG_SIZE);

                pos += TX_EXTRA_TAG_SIZE;
            }

            // write the unlock time to the transaction data
            {
                pos += encode_varint(tx + pos, unlock_time, sizeof(tx));
            }

            L_transaction.input_count = input_count;

            // write the number of inputs to the transaction data
            {
                pos += encode_varint(tx + pos, input_count, sizeof(tx));
            }

            L_transaction.output_count = output_count;

            L_transaction.has_payment_id = has_payment_id;

            // we can go ahead and store the start of the transaction prefix
            TX_WRITE(tx, pos);

            // allocate an info structure
            transaction_info_t tx_info;

            // copy the transaction public key to the structure
            os_memmove(tx_info.tx_public_key, tx_public_key, KEY_SIZE);

            // if we have a payment_id, then copy that over to the structure
            if (has_payment_id == 1)
            {
                os_memmove(tx_info.payment_id, payment_id, KEY_SIZE);
            }

            // write the structure to NVRAM so that we can use it later (saves RAM)
            TX_INFO_WRITE(tx_info);

            L_transaction.state = TX_READY;

            CLOSE_TRY;

            return OP_OK;
        }
        CATCH_OTHER(e)
        {
            return e;
        }
        FINALLY
        {
            explicit_bzero(tx, sizeof(tx));
        }
    }
    END_TRY;
}

/**
 * Updates the state to expect input loading
 */
uint16_t tx_start_input_load()
{
    if (tx_state() != TX_READY)
    {
        return ERR_TRANSACTION_STATE;
    }

    L_transaction.state = TX_RECEIVING_INPUTS;

    return OP_OK;
}

/**
 * Updates the state to expect output loading
 */
uint16_t tx_start_output_load()
{
    // check to validate that we are in the proper state to load outputs
    if (tx_state() != TX_INPUTS_RECEIVED)
    {
        return ERR_TRANSACTION_STATE;
    }

    unsigned char tx[TX_EXTRA_MAX_SIZE] = {0};

    unsigned int pos = 0;

    pos += encode_varint(tx, L_transaction.output_count, sizeof(tx));

    // write the number of outputs to the transaction prefix
    TX_WRITE(tx, pos);

    L_transaction.state = TX_RECEIVING_OUTPUTS;

    return OP_OK;
}

/**
 * Returns the internal transaction state
 */
unsigned int tx_state()
{
    return (unsigned int)L_transaction.state;
}
