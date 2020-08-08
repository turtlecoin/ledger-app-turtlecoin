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

#include "apdu_spend_secret_key.h"

#include <keys.h>
#include <transaction.h>
#include <utils.h>

#define APDU_SSK WORKING_SET

static void do_spend_secret_key()
{
    BEGIN_TRY
    {
        TRY
        {
            sendResponse(write_io_hybrid(PTR_SPEND_PRIVATE, KEY_SIZE, APDU_SPEND_SECRET_KEY_NAME, true), true);
        }
        CATCH_OTHER(e)
        {
            sendError(ERR_NVRAM_READ);
        }
        FINALLY
        {
            // Explicitly clear the working memory
            explicit_bzero(WORKING_SET, WORKING_SET_SIZE);
        };
    }
    END_TRY;
}

/**
 * This operation performs a double confirmation before permitting
 * the application to proceed to make absolute sure the user knows
 * that what they are doing is not in the spirit of operating this
 * hardware device
 */

UX_STEP_NOCB(
    ux_display_spend_secret_key_flow_4_step,
    bnnn_paging,
    {.title = "!! Warning !!",
     .text = "Doing this will result in your private spend key being "
             "exposed which defeats the purpose of this device..."});

UX_STEP_NOCB(
    ux_display_spend_secret_key_flow_5_step,
    bnnn_paging,
    {.title = "Private Spend", .text = (char *)APDU_SSK});

UX_STEP_VALID(ux_display_spend_secret_key_flow_6_step, pb, do_spend_secret_key(), {&C_icon_validate_14, "Approve"});

UX_STEP_VALID(ux_display_spend_secret_key_flow_7_step, pb, do_deny(), {&C_icon_crossmark, "Reject"});

UX_FLOW(
    ux_display_spend_secret_key_flow_2,
    &ux_display_spend_secret_key_flow_4_step,
    &ux_display_spend_secret_key_flow_5_step,
    &ux_display_spend_secret_key_flow_6_step,
    &ux_display_spend_secret_key_flow_7_step);

UX_STEP_NOCB(ux_display_spend_secret_key_flow_1_step, pnn, {&C_icon_turtlecoin, "Export Spend", "Private Key?"});

UX_STEP_VALID(
    ux_display_spend_secret_key_flow_2_step,
    pb,
    ux_flow_init(0, ux_display_spend_secret_key_flow_2, NULL),
    {&C_icon_validate_14, "Approve"});

UX_STEP_VALID(
    ux_display_spend_secret_key_flow_3_step,
    pb,
    sendError(ERR_OP_NOT_PERMITTED),
    {&C_icon_crossmark, "Reject"});

UX_FLOW(
    ux_display_spend_secret_key_flow,
    &ux_display_spend_secret_key_flow_1_step,
    &ux_display_spend_secret_key_flow_2_step,
    &ux_display_spend_secret_key_flow_3_step);

void handle_spend_secret_key(uint8_t p1, uint8_t p2, volatile unsigned int *flags, volatile unsigned int *tx)
{
    UNUSED(p2);

    if (tx_state() != TX_UNUSED)
    {
        return sendError(ERR_TRANSACTION_STATE);
    }

    toHexString(PTR_SPEND_PRIVATE, KEY_SIZE, APDU_SSK, KEY_HEXSTR_SIZE);

    /**
     * If the APDU was sent requesting confirmation then
     * we need to start the UX flow and set the flags
     * to let the i/o handler know that the request is
     * async and will be completed shortly
     */
    if (p1 == P1_CONFIRM)
    {
        ux_flow_init(0, ux_display_spend_secret_key_flow, NULL);

        *flags |= IO_ASYNCH_REPLY;
    }
    else if (p1 == P1_NON_CONFIRM && DEBUG_BUILD == 1)
    {
        do_spend_secret_key();

        *flags |= IO_ASYNCH_REPLY;
    }
    else
    {
        sendError(ERR_OP_USER_REQUIRED);
    }
}