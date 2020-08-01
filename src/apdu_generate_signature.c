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

#include "apdu_generate_signature.h"

#include <keys.h>
#include <utils.h>

#define APDU_GS_SIZE KEY_SIZE
#define APDU_GS_MESSAGE_DIGEST WORKING_SET
#define APDU_GS_SIGNATURE APDU_GS_MESSAGE_DIGEST + KEY_SIZE
#define APDU_GS_MD_HEX APDU_GS_SIGNATURE + SIG_SIZE

static void do_generate_signature()
{
    BEGIN_TRY
    {
        TRY
        {
            if (hw_generate_signature(
                    APDU_GS_SIGNATURE,
                    APDU_GS_MESSAGE_DIGEST,
                    N_turtlecoin_wallet->spend.public,
                    N_turtlecoin_wallet->spend.private)
                != 0)
            {
                THROW(ERR_GENERATE_SIGNATURE);
            }

            CLOSE_TRY;

            sendResponse(write_io_hybrid(APDU_GS_SIGNATURE, SIG_SIZE, APDU_GENERATE_SIGNATURE_NAME, true), true);
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

UX_STEP_SPLASH(
    ux_display_generate_signature_splash_1_step,
    pnn,
    do_generate_signature(),
    {&C_icon_turtlecoin, "Signing", "Digest..."});

UX_FLOW(ux_display_generate_signature_splash, &ux_display_generate_signature_splash_1_step);

UX_STEP_NOCB(
    ux_display_generate_signature_flow_1_step,
    bnnn_paging,
    {.title = "Sign Digest?", .text = (char *)APDU_GS_MD_HEX});

UX_STEP_VALID(
    ux_display_generate_signature_flow_2_step,
    pb,
    ux_flow_init(0, ux_display_generate_signature_splash, NULL),
    {&C_icon_validate_14, "Approve"});

UX_STEP_VALID(ux_display_generate_signature_flow_3_step, pb, do_deny(), {&C_icon_crossmark, "Reject"});

UX_FLOW(
    ux_display_generate_signature_flow,
    &ux_display_generate_signature_flow_1_step,
    &ux_display_generate_signature_flow_2_step,
    &ux_display_generate_signature_flow_3_step);

void handle_generate_signature(
    uint8_t p1,
    uint8_t p2,
    uint8_t *dataBuffer,
    uint16_t dataLength,
    volatile unsigned int *flags,
    volatile unsigned int *tx)
{
    UNUSED(p2);

    if (dataLength != APDU_GS_SIZE)
    {
        return sendError(ERR_WRONG_INPUT_LENGTH);
    }

    // copy the data buffer into the working set
    os_memmove(WORKING_SET, dataBuffer, dataLength);

    toHexString(APDU_GS_MESSAGE_DIGEST, KEY_SIZE, APDU_GS_MD_HEX, KEY_HEXSTR_SIZE);

    /**
     * If the APDU was sent requesting confirmation then
     * we need to start the UX flow and set the flags
     * to let the i/o handler know that the request is
     * async and will be completed shortly
     */
    if (p1 == P1_CONFIRM)
    {
        ux_flow_init(0, ux_display_generate_signature_flow, NULL);

        *flags |= IO_ASYNCH_REPLY;
    }
    else if (p1 == P1_NON_CONFIRM && DEBUG_BUILD == 1)
    {
        ux_flow_init(0, ux_display_generate_signature_splash, NULL);

        *flags |= IO_ASYNCH_REPLY;
    }
    else
    {
        sendError(ERR_OP_USER_REQUIRED);
    }
}