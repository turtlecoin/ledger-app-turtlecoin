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

#include "apdu_tx_start.h"

#include <transaction.h>
#include <utils.h>

#define APDU_TX_START_SIZE sizeof(uint64_t) + sizeof(uint8_t) + sizeof(uint8_t) + KEY_SIZE + sizeof(uint8_t)
#define APDU_TX_START_ALT_SIZE APDU_TX_START_SIZE + KEY_SIZE

#define APDU_TS_UNLOCK_IDX WORKING_SET
#define APDU_TS_UNLOCK readUint64BE(APDU_TS_UNLOCK_IDX)

#define APDU_TS_INPUT_COUNT_IDX APDU_TS_UNLOCK_IDX + sizeof(uint64_t)
#define APDU_TS_INPUT_COUNT readUint8(APDU_TS_INPUT_COUNT_IDX)

#define APDU_TS_OUTPUT_COUNT_IDX APDU_TS_INPUT_COUNT_IDX + sizeof(uint8_t)
#define APDU_TS_OUTPUT_COUNT readUint8(APDU_TS_OUTPUT_COUNT_IDX)

#define APDU_TS_TX_PUBLIC_KEY APDU_TS_OUTPUT_COUNT_IDX + sizeof(uint8_t)

#define APDU_TS_HAS_PAYMENT_ID_IDX APDU_TS_TX_PUBLIC_KEY + KEY_SIZE
#define APDU_TS_HAS_PAYMENT_ID readUint8(APDU_TS_HAS_PAYMENT_ID_IDX)

#define APDU_TS_PAYMENT_ID APDU_TS_HAS_PAYMENT_ID_IDX + sizeof(uint8_t)

static void do_tx_start()
{
    BEGIN_TRY
    {
        TRY
        {
            const uint16_t status = tx_start(
                APDU_TS_UNLOCK,
                APDU_TS_INPUT_COUNT,
                APDU_TS_OUTPUT_COUNT,
                APDU_TS_TX_PUBLIC_KEY,
                APDU_TS_HAS_PAYMENT_ID,
                APDU_TS_PAYMENT_ID);

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

UX_STEP_SPLASH(ux_tx_start_1_step, pnn, do_tx_start(), {&C_icon_turtlecoin, "Starting", "Transaction..."});

UX_FLOW(ux_tx_start_flow, &ux_tx_start_1_step);

void handle_tx_start(
    uint8_t p1,
    uint8_t p2,
    uint8_t *dataBuffer,
    uint16_t dataLength,
    volatile unsigned int *flags,
    volatile unsigned int *tx)
{
    UNUSED(p2);

    if (tx_state() != TX_UNUSED)
    {
        return sendError(ERR_TRANSACTION_STATE);
    }
    else if (dataLength != APDU_TX_START_SIZE && dataLength != APDU_TX_START_ALT_SIZE)
    {
        return sendError(ERR_WRONG_INPUT_LENGTH);
    }

    // copy the data buffer into the working set
    os_memmove(WORKING_SET, dataBuffer, dataLength);

    ux_flow_init(0, ux_tx_start_flow, NULL);

    *flags |= IO_ASYNCH_REPLY;
}