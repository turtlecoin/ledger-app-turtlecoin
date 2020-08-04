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

#include <apdu_address.h>
#include <keys.h>
#include <utils.h>

#define APDU_ADDRESS WORKING_SET

void do_address()
{
    BEGIN_TRY
    {
        TRY
        {
            if (generate_public_address(PTR_SPEND_PUBLIC, PTR_VIEW_PUBLIC, APDU_ADDRESS) != 0)
            {
                THROW(ERR_ADDRESS);
            }

            sendResponse(write_io_hybrid(APDU_ADDRESS, BASE58_ADDRESS_SIZE, APDU_ADDRESS_NAME, false), true);
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

UX_STEP_NOCB(ux_display_address_1_step, pnn, {&C_icon_turtlecoin, "Export Wallet", "   Address?  "});

UX_STEP_NOCB(ux_display_address_2_step, bnnn_paging, {.title = "Address", .text = (char *)APDU_ADDRESS});

UX_STEP_VALID(ux_display_address_3_step, pb, do_address(), {&C_icon_validate_14, "Approve"});

UX_STEP_VALID(ux_display_address_4_step, pb, do_deny(), {&C_icon_crossmark, "Reject"});

UX_FLOW(
    ux_display_address_flow,
    &ux_display_address_1_step,
    &ux_display_address_2_step,
    &ux_display_address_3_step,
    &ux_display_address_4_step);

void handle_address(uint8_t p1, uint8_t p2, volatile unsigned int *flags, volatile unsigned int *tx)
{
    UNUSED(p2);

    /**
     * If the APDU was sent requesting confirmation then
     * we need to start the UX flow and set the flags
     * to let the i/o handler know that the request is
     * async and will be completed shortly
     */
    if (p1 == P1_CONFIRM)
    {
        ux_flow_init(0, ux_display_address_flow, NULL);

        *flags |= IO_ASYNCH_REPLY;
    }
    else if (p1 == P1_NON_CONFIRM && DEBUG_BUILD == 1)
    {
        do_address();

        *flags |= IO_ASYNCH_REPLY;
    }
    else
    {
        sendError(ERR_OP_USER_REQUIRED);
    }
}
