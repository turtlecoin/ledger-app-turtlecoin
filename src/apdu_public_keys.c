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

#include "apdu_public_keys.h"

#include <keys.h>
#include <utils.h>

#define APDU_PK_KEYS WORKING_SET
#define APDU_PK_SPEND WORKING_SET + KEY_SIZE + KEY_SIZE
#define APDU_PK_VIEW APDU_PK_SPEND + KEY_HEXSTR_SIZE + 1

static void do_public_keys()
{
    BEGIN_TRY
    {
        TRY
        {
            sendResponse(write_io_hybrid(APDU_PK_KEYS, KEY_SIZE + KEY_SIZE, APDU_PUBLIC_KEYS_NAME, true), true);
        }
        CATCH_OTHER(e)
        {
            sendError(ERR_UNKNOWN_ERROR);
        }
        FINALLY
        {
            // Explicitly clear the working memory
            explicit_bzero(WORKING_SET, WORKING_SET_SIZE);
        };
    }
    END_TRY;
}

UX_STEP_NOCB(ux_display_public_keys_flow_1_step, pnn, {&C_icon_turtlecoin, "   Export   ", "Public Keys?"});

UX_STEP_NOCB(ux_display_public_keys_flow_2_step, bnnn_paging, {.title = "Public Spend", .text = (char *)APDU_PK_SPEND});

UX_STEP_NOCB(ux_display_public_keys_flow_3_step, bnnn_paging, {.title = "Public View", .text = (char *)APDU_PK_VIEW});

UX_STEP_VALID(ux_display_public_keys_flow_4_step, pb, do_public_keys(), {&C_icon_validate_14, "Approve"});

UX_STEP_VALID(ux_display_public_keys_flow_5_step, pb, do_deny(), {&C_icon_crossmark, "Reject"});

UX_FLOW(
    ux_display_public_keys_flow,
    &ux_display_public_keys_flow_1_step,
    &ux_display_public_keys_flow_2_step,
    &ux_display_public_keys_flow_3_step,
    &ux_display_public_keys_flow_4_step,
    &ux_display_public_keys_flow_5_step);

void handle_public_keys(uint8_t p1, uint8_t p2, volatile unsigned int *flags, volatile unsigned int *tx)
{
    UNUSED(p2);

    toHexString(PTR_SPEND_PUBLIC, KEY_SIZE, APDU_PK_SPEND, KEY_HEXSTR_SIZE);

    toHexString(PTR_VIEW_PUBLIC, KEY_SIZE, APDU_PK_VIEW, KEY_HEXSTR_SIZE);

    os_memmove(APDU_PK_KEYS, PTR_SPEND_PUBLIC, KEY_SIZE);

    os_memmove(APDU_PK_KEYS + KEY_SIZE, PTR_VIEW_PUBLIC, KEY_SIZE);

    /**
     * If the APDU was sent requesting confirmation then
     * we need to start the UX flow and set the flags
     * to let the i/o handler know that the request is
     * async and will be completed shortly
     */
    if (p1 == P1_CONFIRM)
    {
        ux_flow_init(0, ux_display_public_keys_flow, NULL);

        *flags |= IO_ASYNCH_REPLY;
    }
    else if (p1 == P1_NON_CONFIRM && DEBUG_BUILD == 1)
    {
        do_public_keys();

        *flags |= IO_ASYNCH_REPLY;
    }
    else
    {
        sendError(ERR_OP_USER_REQUIRED);
    }
}