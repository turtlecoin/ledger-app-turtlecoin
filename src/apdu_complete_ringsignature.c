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

#include "apdu_complete_ringsignature.h"

#include <keys.h>
#include <transaction.h>
#include <utils.h>

#define APDU_CRS_SIZE KEY_SIZE + sizeof(uint32_t) + KEY_SIZE + KEY_SIZE + SIG_SIZE
#define APDU_CRS_TX_PUBLIC_KEY WORKING_SET
#define APDU_CRS_OUTPUT_INDEX readUint32BE(WORKING_SET + KEY_SIZE)
#define APDU_CRS_OUTPUT_KEY WORKING_SET + KEY_SIZE + sizeof(uint32_t)
#define APDU_CRS_K APDU_CRS_OUTPUT_KEY + KEY_SIZE
#define APDU_CRS_SIGNATURE APDU_CRS_K + KEY_SIZE

static void do_complete_ring_signature()
{
    BEGIN_TRY
    {
        TRY
        {
            const uint16_t status = hw_complete_ring_signature(
                APDU_CRS_SIGNATURE,
                APDU_CRS_TX_PUBLIC_KEY,
                APDU_CRS_OUTPUT_INDEX,
                APDU_CRS_OUTPUT_KEY,
                APDU_CRS_K,
                N_turtlecoin_wallet->view.private,
                N_turtlecoin_wallet->spend.private,
                N_turtlecoin_wallet->spend.public);

            if (status != OP_OK)
            {
                THROW(status);
            }

            CLOSE_TRY;

            sendResponse(write_io_hybrid(APDU_CRS_SIGNATURE, SIG_SIZE, APDU_COMPLETE_RING_SIGNATURE_NAME, true), true);
        }
        CATCH_OTHER(e)
        {
            sendError(e);
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
    ux_display_complete_ringsignature_splash_flow_1_step,
    pnn,
    do_complete_ring_signature(),
    {&C_icon_turtlecoin, "Completing", "Ring Signature..."});

UX_FLOW(ux_display_complete_ringsignature_splash, &ux_display_complete_ringsignature_splash_flow_1_step);

UX_STEP_NOCB(ux_display_complete_ringsignature_flow_1_step, pnn, {&C_icon_turtlecoin, "Complete", "Ring Signature?"});

UX_STEP_NOCB(
    ux_display_complete_ringsignature_flow_2_step,
    bnnn_paging,
    {.title = "Output Key", .text = (char *)DISPLAY_KEY_HEX});

UX_STEP_VALID(
    ux_display_complete_ringsignature_flow_3_step,
    pb,
    ux_flow_init(0, ux_display_complete_ringsignature_splash, NULL),
    {&C_icon_validate_14, "Approve"});

UX_STEP_VALID(ux_display_complete_ringsignature_flow_4_step, pb, do_deny(), {&C_icon_crossmark, "Reject"});

UX_FLOW(
    ux_display_complete_ringsignature_flow,
    &ux_display_complete_ringsignature_flow_1_step,
    &ux_display_complete_ringsignature_flow_2_step,
    &ux_display_complete_ringsignature_flow_3_step,
    &ux_display_complete_ringsignature_flow_4_step);

/**
 * @param tx_public_key {32 bytes}
 * @param output_index {4 bytes}
 * @param output_key {32 bytes}
 * @param k (random key used in prepare ring signatures) {32 bytes}
 * @param signature (signature to complete) {64 bytes}
 * @returns signature
 */
void handle_complete_ring_signature(
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
    else if (dataLength != APDU_CRS_SIZE)
    {
        return sendError(ERR_WRONG_INPUT_LENGTH);
    }

    // copy the data buffer into the working set
    os_memmove(WORKING_SET, dataBuffer, dataLength);

    toHexString(APDU_CRS_OUTPUT_KEY, KEY_SIZE, DISPLAY_KEY_HEX, KEY_HEXSTR_SIZE);

    /**
     * If the APDU was sent requesting confirmation then
     * we need to start the UX flow and set the flags
     * to let the i/o handler know that the request is
     * async and will be completed shortly
     */
    if (p1 == P1_CONFIRM)
    {
        ux_flow_init(0, ux_display_complete_ringsignature_flow, NULL);

        *flags |= IO_ASYNCH_REPLY;
    }
    else if (p1 == P1_NON_CONFIRM && DEBUG_BUILD == 1)
    {
        ux_flow_init(0, ux_display_complete_ringsignature_splash, NULL);

        *flags |= IO_ASYNCH_REPLY;
    }
    else
    {
        sendError(ERR_OP_USER_REQUIRED);
    }
}