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

#include "apdu_derive_secret_key.h"

#include <keys.h>
#include <utils.h>

#define APDU_DSK_SIZE KEY_SIZE + sizeof(uint32_t)
#define APDU_DSK_DERIVATION WORKING_SET
#define APDU_DSK_OUTPUT_IDX readUint32BE(APDU_DSK_DERIVATION + KEY_SIZE)
#define APDU_DSK_SECRET_KEY APDU_DSK_DERIVATION + KEY_SIZE + sizeof(uint32_t)

static void do_derive_secret_key()
{
    BEGIN_TRY
    {
        TRY
        {
            if (hw_derive_secret_key(APDU_DSK_SECRET_KEY, APDU_DSK_DERIVATION, APDU_DSK_OUTPUT_IDX, PTR_SPEND_PRIVATE)
                != 0)
            {
                THROW(ERR_DERIVE_PUBKEY);
            }

            CLOSE_TRY;

            sendResponse(write_io_hybrid(APDU_DSK_SECRET_KEY, KEY_SIZE, APDU_DERIVE_SECRET_KEY_NAME, true), true);
        }
        CATCH_OTHER(e)
        {
            sendError(ERR_UNKNOWN_ERROR);
        }
        FINALLY
        {
            // Explicitly clear the working memory
            explicit_bzero(WORKING_SET, WORKING_SET_SIZE);

            // Explicitly clear any display information
            explicit_bzero(DISPLAY_KEY_HEX, KEY_HEXSTR_SIZE);
        };
    }
    END_TRY;
}

UX_STEP_SPLASH(
    ux_display_derive_secret_key_manual_flow_1_step,
    pnn,
    do_derive_secret_key(),
    {&C_icon_turtlecoin, "Deriving", "Secret Key..."});

UX_FLOW(ux_display_derive_secret_key_manual_flow, &ux_display_derive_secret_key_manual_flow_1_step);

UX_STEP_NOCB(ux_display_derive_secret_key_flow_1_step, pnn, {&C_icon_turtlecoin, "Derive", "Secret Key?"});

UX_STEP_NOCB(ux_display_derive_secret_key_flow_2_step, bnnn_paging,
    {.title = "Derivation", .text = (char *)DISPLAY_KEY_HEX});

UX_STEP_VALID(
    ux_display_derive_secret_key_flow_3_step,
    pb,
    ux_flow_init(0, ux_display_derive_secret_key_manual_flow, NULL),
    {&C_icon_validate_14, "Approve"});

UX_STEP_VALID(ux_display_derive_secret_key_flow_4_step, pb, do_deny(), {&C_icon_crossmark, "Reject"});

UX_FLOW(
    ux_display_derive_secret_key_flow,
    &ux_display_derive_secret_key_flow_1_step,
    &ux_display_derive_secret_key_flow_2_step,
    &ux_display_derive_secret_key_flow_3_step,
    &ux_display_derive_secret_key_flow_4_step);

void handle_derive_secret_key(
    uint8_t p1,
    uint8_t p2,
    uint8_t *dataBuffer,
    uint16_t dataLength,
    volatile unsigned int *flags,
    volatile unsigned int *tx)
{
    UNUSED(p2);

    if (dataLength != APDU_DSK_SIZE)
    {
        return sendError(ERR_WRONG_INPUT_LENGTH);
    }

    // copy the data buffer into the working set
    os_memmove(WORKING_SET, dataBuffer, dataLength);

    toHexString(APDU_DSK_DERIVATION, KEY_SIZE, DISPLAY_KEY_HEX, KEY_HEXSTR_SIZE);

    /**
     * If the APDU was sent requesting confirmation then
     * we need to start the UX flow and set the flags
     * to let the i/o handler know that the request is
     * async and will be completed shortly
     */
    if (p1 == P1_CONFIRM)
    {
        ux_flow_init(0, ux_display_derive_secret_key_flow, NULL);

        *flags |= IO_ASYNCH_REPLY;
    }
    else if (p1 == P1_NON_CONFIRM && DEBUG_BUILD == 1)
    {
        ux_flow_init(0, ux_display_derive_secret_key_manual_flow, NULL);

        *flags |= IO_ASYNCH_REPLY;
    }
    else
    {
        sendError(ERR_OP_USER_REQUIRED);
    }
}