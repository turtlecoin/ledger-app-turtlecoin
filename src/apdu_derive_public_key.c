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

#include "apdu_derive_public_key.h"

#include <keys.h>
#include <transaction.h>
#include <utils.h>

#define APDU_DPK_SIZE KEY_SIZE + sizeof(uint32_t)
#define APDU_DPK_DERIVATION WORKING_SET
#define APDU_DPK_OUTPUT_IDX readUint32BE(APDU_DPK_DERIVATION + KEY_SIZE)
#define APDU_DPK_PUBLIC_KEY APDU_DPK_DERIVATION + KEY_SIZE + sizeof(uint32_t)

static unsigned int pre_approved = 0;

static void do_derive_public_key(const size_t approve_all_this_session)
{
    BEGIN_TRY
    {
        TRY
        {
            const uint16_t status =
                hw_derive_public_key(APDU_DPK_PUBLIC_KEY, APDU_DPK_DERIVATION, APDU_DPK_OUTPUT_IDX, PTR_SPEND_PUBLIC);

            if (status != OP_OK)
            {
                THROW(status);
            }

            pre_approved = approve_all_this_session;

            CLOSE_TRY;

            sendResponse(write_io_hybrid(APDU_DPK_PUBLIC_KEY, KEY_SIZE, APDU_DERIVE_PUBLIC_KEY_NAME, true), true);
        }
        CATCH_OTHER(e)
        {
            sendError(e);
        }
        FINALLY
        {
            // Explicitly clear the working memory
            explicit_bzero(WORKING_SET, WORKING_SET_SIZE);
        };
    }
    END_TRY;
}

UX_STEP_SPLASH(
    ux_display_derive_public_key_manual_flow_1_step,
    pnn,
    do_derive_public_key(0),
    {&C_icon_turtlecoin, "Deriving", "Public Key..."});

UX_STEP_SPLASH(
    ux_display_derive_public_key_auto_flow_1_step,
    pnn,
    do_derive_public_key(1),
    {&C_icon_turtlecoin, "Deriving", "Public Key..."});

UX_FLOW(ux_display_derive_public_key_manual_flow, &ux_display_derive_public_key_manual_flow_1_step);

UX_FLOW(ux_display_derive_public_key_auto_flow, &ux_display_derive_public_key_auto_flow_1_step);

UX_STEP_NOCB(ux_display_derive_public_key_flow_1_step, pnn, {&C_icon_turtlecoin, " Derive ", "Public Key?"});

UX_STEP_VALID(
    ux_display_derive_public_key_flow_2_step,
    pb,
    ux_flow_init(0, ux_display_derive_public_key_auto_flow, NULL),
    {&C_icon_validate_14, "Approve All"});

UX_STEP_VALID(
    ux_display_derive_public_key_flow_3_step,
    pb,
    ux_flow_init(0, ux_display_derive_public_key_manual_flow, NULL),
    {&C_icon_validate_14, "Approve"});

UX_STEP_VALID(ux_display_derive_public_key_flow_4_step, pb, do_deny(), {&C_icon_crossmark, "Reject"});

UX_FLOW(
    ux_display_derive_public_key_flow,
    &ux_display_derive_public_key_flow_1_step,
    &ux_display_derive_public_key_flow_2_step,
    &ux_display_derive_public_key_flow_3_step,
    &ux_display_derive_public_key_flow_4_step);

void handle_derive_public_key(
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
    else if (dataLength != APDU_DPK_SIZE)
    {
        return sendError(ERR_WRONG_INPUT_LENGTH);
    }

    // copy the data buffer into the working set
    os_memmove(WORKING_SET, dataBuffer, dataLength);

    /**
     * If the APDU was sent requesting confirmation then
     * we need to start the UX flow and set the flags
     * to let the i/o handler know that the request is
     * async and will be completed shortly unless approval
     * was already provided in this application "session"
     */
    if (pre_approved == 1)
    {
        ux_flow_init(0, ux_display_derive_public_key_auto_flow, NULL);

        *flags |= IO_ASYNCH_REPLY;
    }
    else if (p1 == P1_CONFIRM)
    {
        ux_flow_init(0, ux_display_derive_public_key_flow, NULL);

        *flags |= IO_ASYNCH_REPLY;
    }
    else if (p1 == P1_NON_CONFIRM && DEBUG_BUILD == 1)
    {
        ux_flow_init(0, ux_display_derive_public_key_auto_flow, NULL);

        *flags |= IO_ASYNCH_REPLY;
    }
    else
    {
        sendError(ERR_OP_USER_REQUIRED);
    }
}