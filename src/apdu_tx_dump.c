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

#include "apdu_tx_dump.h"

#include <transaction.h>
#include <utils.h>

#define APDU_TXD_SIZE sizeof(uint16_t)

#define APDU_TXD_START_OFFSET_INDEX WORKING_SET
#define APDU_TXD_START_OFFSET readUint16BE(APDU_TXD_START_OFFSET_INDEX)

#define APDU_TXD_RESPONSE WORKING_SET

static void do_tx_dump()
{
    BEGIN_TRY
    {
        TRY
        {
            if (APDU_TXD_START_OFFSET > tx_size())
            {
                THROW(ERR_OUT_OF_RANGE);
            }

            uint16_t length = tx_size() - APDU_TXD_START_OFFSET;

            if (length > TX_MAX_DUMP_SIZE)
            {
                length = TX_MAX_DUMP_SIZE;
            }

            const uint16_t status = tx_dump(APDU_TXD_RESPONSE, APDU_TXD_START_OFFSET, length);

            if (status != OP_OK)
            {
                THROW(status);
            }

            CLOSE_TRY;

            sendResponse(write_io_hybrid(APDU_TXD_RESPONSE, length, APDU_TX_DUMP_NAME, true), true);
        }
        CATCH_OTHER(e)
        {
            sendError(e);
        }
        FINALLY {}
    }
    END_TRY;
}

UX_STEP_SPLASH(ux_tx_dump_1_step, pnn, do_tx_dump(), {&C_icon_turtlecoin, "Dumping", "Transaction..."});

UX_FLOW(ux_tx_dump_flow, &ux_tx_dump_1_step);

void handle_tx_dump(
    uint8_t p1,
    uint8_t p2,
    uint8_t *dataBuffer,
    uint16_t dataLength,
    volatile unsigned int *flags,
    volatile unsigned int *tx)
{
    UNUSED(p1);

    UNUSED(p2);

    if (tx_state() != TX_COMPLETE)
    {
        return sendError(ERR_TRANSACTION_STATE);
    }
    else if (dataLength != APDU_TXD_SIZE)
    {
        return sendError(ERR_WRONG_INPUT_LENGTH);
    }

    // copy the data buffer into the working set
    os_memmove(WORKING_SET, dataBuffer, dataLength);

    ux_flow_init(0, ux_tx_dump_flow, NULL);

    *flags |= IO_ASYNCH_REPLY;
}