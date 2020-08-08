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

#include "apdu_tx_reset.h"

#include <transaction.h>
#include <utils.h>

static void do_tx_reset()
{
    BEGIN_TRY
    {
        TRY
        {
            const uint16_t status = tx_reset();

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

UX_STEP_SPLASH(ux_tx_reset_1_step, pnn, do_tx_reset(), {&C_icon_turtlecoin, "Clearing", "Transaction State..."});

UX_FLOW(ux_tx_reset_flow, &ux_tx_reset_1_step);

UX_STEP_NOCB(ux_tx_reset_2_step, pnn, {&C_icon_turtlecoin, "Reset Transaction", "State?"});

UX_STEP_VALID(ux_tx_reset_3_step, pb, ux_flow_init(0, ux_tx_reset_flow, NULL), {&C_icon_validate_14, "Approve"});

UX_STEP_VALID(ux_tx_reset_4_step, pb, do_deny(), {&C_icon_crossmark, "Reject"});

UX_FLOW(ux_tx_reset_confirm_flow, &ux_tx_reset_2_step, &ux_tx_reset_3_step, &ux_tx_reset_4_step);

void handle_tx_reset(uint8_t p1, uint8_t p2, volatile unsigned int *flags, volatile unsigned int *tx)
{
    UNUSED(p2);

    // if we are not currently in a transaction construction state then we can return quickly
    if (tx_state() == TX_UNUSED)
    {
        return sendResponse(0, true);
    }

    /**
     * If the APDU was sent requesting confirmation then
     * we need to start the UX flow and set the flags
     * to let the i/o handler know that the request is
     * async and will be completed shortly
     */
    if (p1 == P1_CONFIRM)
    {
        ux_flow_init(0, ux_tx_reset_confirm_flow, NULL);

        *flags |= IO_ASYNCH_REPLY;
    }
    else if (p1 == P1_NON_CONFIRM && DEBUG_BUILD == 1)
    {
        ux_flow_init(0, ux_tx_reset_flow, NULL);

        *flags |= IO_ASYNCH_REPLY;
    }
    else
    {
        sendError(ERR_OP_USER_REQUIRED);
    }
}