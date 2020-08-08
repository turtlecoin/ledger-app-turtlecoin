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

#include "apdu_tx_input_load.h"

#include <transaction.h>
#include <utils.h>

#define APDU_TX_LOAD_INPUT_SIZE                                                    \
    KEY_SIZE + sizeof(uint8_t) + sizeof(uint64_t) + (KEY_SIZE * RING_PARTICIPANTS) \
        + (sizeof(uint32_t) * RING_PARTICIPANTS) + sizeof(uint8_t)

#define APDU_TLI_TX_PUBLIC_KEY WORKING_SET

#define APDU_TLI_OUTPUT_INDEX_IDX APDU_TLI_TX_PUBLIC_KEY + KEY_SIZE
#define APDU_TLI_OUTPUT_INDEX readUint8(APDU_TLI_OUTPUT_INDEX_IDX)

#define APDU_TLI_AMOUNT_IDX APDU_TLI_OUTPUT_INDEX_IDX + sizeof(uint8_t)
#define APDU_TLI_AMOUNT readUint64BE(APDU_TLI_AMOUNT_IDX)

#define APDU_TLI_PUBLIC_KEYS APDU_TLI_AMOUNT_IDX + sizeof(uint64_t)

#define APDU_TLI_OFFSETS_IDX APDU_TLI_PUBLIC_KEYS + (KEY_SIZE * RING_PARTICIPANTS)

#define APDU_TLI_REAL_OUTPUT_INDEX_IDX APDU_TLI_OFFSETS_IDX + (sizeof(uint32_t) * RING_PARTICIPANTS)
#define APDU_TLI_REAL_OUTPUT_INDEX readUint8(APDU_TLI_REAL_OUTPUT_INDEX_IDX)

static void do_tx_input_load()
{
    BEGIN_TRY
    {
        TRY
        {
            uint32_t offsets[RING_PARTICIPANTS];

            int i;

            for (i = 0; i < RING_PARTICIPANTS; i++)
            {
                offsets[i] = readUint32BE(APDU_TLI_OFFSETS_IDX + (i * sizeof(uint32_t)));
            }

            const uint16_t status = tx_load_input(
                APDU_TLI_TX_PUBLIC_KEY,
                APDU_TLI_OUTPUT_INDEX,
                APDU_TLI_AMOUNT,
                APDU_TLI_PUBLIC_KEYS,
                offsets,
                APDU_TLI_REAL_OUTPUT_INDEX);

            if (status != OP_OK)
            {
                THROW(status);
            }

            CLOSE_TRY;

            sendResponse(0, true);
        }
        CATCH_OTHER(e)
        {
            sendError(e);
        }
        FINALLY {}
    }
    END_TRY;
}

UX_STEP_SPLASH(ux_tx_input_load_1_step, pnn, do_tx_input_load(), {&C_icon_turtlecoin, "Loading Tx", "Input..."});

UX_FLOW(ux_tx_input_load_flow, &ux_tx_input_load_1_step);

void handle_tx_input_load(
    uint8_t p1,
    uint8_t p2,
    uint8_t *dataBuffer,
    uint16_t dataLength,
    volatile unsigned int *flags,
    volatile unsigned int *tx)
{
    UNUSED(p2);

    if (tx_state() != TX_RECEIVING_INPUTS)
    {
        return sendError(ERR_TRANSACTION_STATE);
    }
    else if (dataLength != APDU_TX_LOAD_INPUT_SIZE)
    {
        return sendError(ERR_WRONG_INPUT_LENGTH);
    }

    // copy the data buffer into the working set
    os_memmove(WORKING_SET, dataBuffer, dataLength);

    ux_flow_init(0, ux_tx_input_load_flow, NULL);

    *flags |= IO_ASYNCH_REPLY;
}