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

#include "apdu_tx_sign.h"

#include <transaction.h>
#include <utils.h>

#define APDU_TSIGN_HASH WORKING_SET
#define APDU_TSIGN_END_OFFSET APDU_TSIGN_HASH + KEY_SIZE
#define APDU_TSIGN_AMOUNT APDU_TSIGN_END_OFFSET + sizeof(uint16_t)
#define APDU_TSIGN_FEE APDU_TSIGN_AMOUNT + KEY_SIZE // give the amount plenty of room to breath

#define APDU_TSIGN_RESPONSE APDU_TSIGN_HASH
#define APDU_TSIGN_RESPONSE_SIZE KEY_SIZE + sizeof(uint16_t)

static void do_tx_sign()
{
    BEGIN_TRY
    {
        TRY
        {
            uint16_t status = tx_sign();

            if (status != OP_OK)
            {
                THROW(status);
            }

            status = tx_hash(APDU_TSIGN_HASH);

            if (status != OP_OK)
            {
                THROW(status);
            }

            uint16ToChar(APDU_TSIGN_END_OFFSET, tx_size());

            CLOSE_TRY;

            sendResponse(write_io_hybrid(APDU_TSIGN_RESPONSE, APDU_TSIGN_RESPONSE_SIZE, APDU_TX_SIGN_NAME, true), true);
        }
        CATCH_OTHER(e)
        {
            sendError(e);
        }
        FINALLY {}
    }
    END_TRY;
}

UX_STEP_SPLASH(ux_tx_sign_1_step, pnn, do_tx_sign(), {&C_icon_turtlecoin, "Signing", "Transaction..."});

UX_FLOW(ux_tx_sign_flow, &ux_tx_sign_1_step);

UX_STEP_NOCB(ux_tx_sign_2_step, pnn, {&C_icon_turtlecoin, "Sign", "Transaction?"});

UX_STEP_NOCB(ux_tx_sign_3_step, bnnn_paging, {.title = "Amount to Spend", .text = (char *)APDU_TSIGN_AMOUNT});

UX_STEP_NOCB(ux_tx_sign_4_step, bnnn_paging, {.title = "Network Fee", .text = (char *)APDU_TSIGN_FEE});

UX_STEP_VALID(ux_tx_sign_5_step, pb, ux_flow_init(0, ux_tx_sign_flow, NULL), {&C_icon_validate_14, "Approve"});

UX_STEP_VALID(ux_tx_sign_6_step, pb, do_deny(), {&C_icon_crossmark, "Reject"});

UX_FLOW(
    ux_tx_sign_confirm_flow,
    &ux_tx_sign_2_step,
    &ux_tx_sign_3_step,
    &ux_tx_sign_4_step,
    &ux_tx_sign_5_step,
    &ux_tx_sign_6_step);

void handle_tx_sign(uint8_t p1, uint8_t p2, volatile unsigned int *flags, volatile unsigned int *tx)
{
    UNUSED(p2);

    if (tx_state() != TX_PREFIX_READY)
    {
        return sendError(ERR_TRANSACTION_STATE);
    }

    {
        unsigned int offset = amountToString(APDU_TSIGN_AMOUNT, tx_input_amount(), KEY_SIZE);

        // copy the ticker on to the end of the amount
        os_memmove(APDU_TSIGN_AMOUNT + offset - 1, TICKER, TICKER_SIZE);
    }

    {
        unsigned int offset = amountToString(APDU_TSIGN_FEE, tx_fee(), KEY_SIZE);

        // copy the ticker on to the end of the amount
        os_memmove(APDU_TSIGN_FEE + offset - 1, TICKER, TICKER_SIZE);
    }

    /**
     * If the APDU was sent requesting confirmation then
     * we need to start the UX flow and set the flags
     * to let the i/o handler know that the request is
     * async and will be completed shortly
     */
    if (p1 == P1_CONFIRM)
    {
        ux_flow_init(0, ux_tx_sign_confirm_flow, NULL);

        *flags |= IO_ASYNCH_REPLY;
    }
    else if (p1 == P1_NON_CONFIRM && DEBUG_BUILD == 1)
    {
        ux_flow_init(0, ux_tx_sign_flow, NULL);

        *flags |= IO_ASYNCH_REPLY;
    }
    else
    {
        sendError(ERR_OP_USER_REQUIRED);
    }
}