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

#include "apdu_view_wallet_keys.h"

#include <keys.h>
#include <transaction.h>
#include <utils.h>

#define APDU_WK_KEYS WORKING_SET
#define APDU_WK_SPEND_PUBLIC WORKING_SET + KEY_SIZE + KEY_SIZE
#define APDU_WK_VIEW_PRIVATE APDU_WK_SPEND_PUBLIC + KEY_HEXSTR_SIZE + 1

static void do_view_wallet_keys()
{
    BEGIN_TRY
    {
        TRY
        {
            sendResponse(write_io_hybrid(APDU_WK_KEYS, KEY_SIZE + KEY_SIZE, APDU_WALLET_KEYS_NAME, true), true);
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

UX_STEP_NOCB(ux_display_wallet_keys_flow_1_step, pnn, {&C_icon_turtlecoin, "Export View", "Wallet Keys?"});

UX_STEP_NOCB(ux_display_wallet_keys_flow_2_step, bnnn_paging, {.title = "Public Spend", .text = (char *)APDU_WK_SPEND_PUBLIC});

UX_STEP_NOCB(ux_display_wallet_keys_flow_3_step, bnnn_paging, {.title = "Private View", .text = (char *)APDU_WK_VIEW_PRIVATE});

UX_STEP_VALID(ux_display_wallet_keys_flow_4_step, pb, do_view_wallet_keys(), {&C_icon_validate_14, "Approve"});

UX_STEP_VALID(ux_display_wallet_keys_flow_5_step, pb, do_deny(), {&C_icon_crossmark, "Reject"});

UX_FLOW(
        ux_display_wallet_keys_flow,
&ux_display_wallet_keys_flow_1_step,
&ux_display_wallet_keys_flow_2_step,
&ux_display_wallet_keys_flow_3_step,
&ux_display_wallet_keys_flow_4_step,
&ux_display_wallet_keys_flow_5_step);

void handle_view_wallet_keys(uint8_t p1, uint8_t p2, volatile unsigned int *flags, volatile unsigned int *tx)
{
    UNUSED(p2);

    if (tx_state() != TX_UNUSED)
    {
        return sendError(ERR_TRANSACTION_STATE);
    }

    toHexString(PTR_SPEND_PUBLIC, KEY_SIZE, APDU_WK_SPEND_PUBLIC, KEY_HEXSTR_SIZE);

    toHexString(PTR_VIEW_PRIVATE, KEY_SIZE, APDU_WK_VIEW_PRIVATE, KEY_HEXSTR_SIZE);

    os_memmove(APDU_WK_KEYS, PTR_SPEND_PUBLIC, KEY_SIZE);

    os_memmove(APDU_WK_KEYS + KEY_SIZE, PTR_VIEW_PRIVATE, KEY_SIZE);

    /**
     * If the APDU was sent requesting confirmation then
     * we need to start the UX flow and set the flags
     * to let the i/o handler know that the request is
     * async and will be completed shortly
     */
    if (p1 == P1_CONFIRM)
    {
        ux_flow_init(0, ux_display_wallet_keys_flow, NULL);

        *flags |= IO_ASYNCH_REPLY;
    }
    else if (p1 == P1_NON_CONFIRM && DEBUG_BUILD == 1)
    {
        do_view_wallet_keys();

        *flags |= IO_ASYNCH_REPLY;
    }
    else
    {
        sendError(ERR_OP_USER_REQUIRED);
    }
}