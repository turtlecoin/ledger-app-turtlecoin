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

#include "apdu_tx_output_load.h"

#include <transaction.h>
#include <utils.h>

#define APDU_TX_LOAD_OUTPUT_SIZE sizeof(uint64_t) + KEY_SIZE

#define APDU_TLO_AMOUNT_IDX WORKING_SET
#define APDU_TLO_AMOUNT readUint64BE(APDU_TLO_AMOUNT_IDX)

#define APDU_TLO_KEY APDU_TLO_AMOUNT_IDX + sizeof(uint64_t)

static void do_tx_output_load()
{
    BEGIN_TRY
    {
        TRY
        {
            const uint16_t status = tx_load_output(APDU_TLO_AMOUNT, APDU_TLO_KEY);

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

UX_STEP_SPLASH(ux_tx_output_load_1_step, pnn, do_tx_output_load(), {&C_icon_turtlecoin, "Loading Tx", "Output..."});

UX_FLOW(ux_tx_output_load_flow, &ux_tx_output_load_1_step);

void handle_tx_output_load(
    uint8_t p1,
    uint8_t p2,
    uint8_t *dataBuffer,
    uint16_t dataLength,
    volatile unsigned int *flags,
    volatile unsigned int *tx)
{
    UNUSED(p2);

    if (tx_state() != TX_RECEIVING_OUTPUTS)
    {
        return sendError(ERR_TRANSACTION_STATE);
    }
    else if (dataLength != APDU_TX_LOAD_OUTPUT_SIZE)
    {
        return sendError(ERR_WRONG_INPUT_LENGTH);
    }

    // copy the data buffer into the working set
    os_memmove(WORKING_SET, dataBuffer, dataLength);

    ux_flow_init(0, ux_tx_output_load_flow, NULL);

    *flags |= IO_ASYNCH_REPLY;
}