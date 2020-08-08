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

#include "apdu_generate_ringsignatures.h"

#include <keys.h>
#include <transaction.h>
#include <utils.h>

#define APDU_GRS_SIZE \
    KEY_SIZE + sizeof(uint32_t) + KEY_SIZE + KEY_SIZE + (KEY_SIZE * RING_PARTICIPANTS) + sizeof(uint32_t)
#define APDU_GRS_TX_PUBLIC_KEY WORKING_SET
#define APDU_GRS_OUTPUT_INDEX_POS WORKING_SET + KEY_SIZE
#define APDU_GRS_OUTPUT_IDX readUint32BE(APDU_GRS_OUTPUT_INDEX_POS)
#define APDU_GRS_OUTPUT_KEY APDU_GRS_OUTPUT_INDEX_POS + sizeof(uint32_t)
#define APDU_GRS_TX_PREFIX_HASH APDU_GRS_OUTPUT_KEY + KEY_SIZE
#define APDU_GRS_INPUT_KEYS APDU_GRS_TX_PREFIX_HASH + KEY_SIZE
#define APDU_GRS_REAL_OUTPUT_POS APDU_GRS_INPUT_KEYS + (KEY_SIZE * RING_PARTICIPANTS)
#define APDU_GRS_REAL_OUTPUT_IDX readUint32BE(APDU_GRS_REAL_OUTPUT_POS)
#define APDU_GRS_SIGNATURES APDU_GRS_REAL_OUTPUT_POS + sizeof(uint32_t)

static void do_generate_ring_signatures()
{
    BEGIN_TRY
    {
        TRY
        {
            const uint16_t status = hw_generate_ring_signatures(
                APDU_GRS_SIGNATURES,
                APDU_GRS_TX_PUBLIC_KEY,
                APDU_GRS_OUTPUT_IDX,
                APDU_GRS_OUTPUT_KEY,
                APDU_GRS_TX_PREFIX_HASH,
                APDU_GRS_INPUT_KEYS,
                APDU_GRS_REAL_OUTPUT_IDX,
                N_turtlecoin_wallet->view.private,
                N_turtlecoin_wallet->spend.private,
                N_turtlecoin_wallet->spend.public);

            if (status != OP_OK)
            {
                THROW(status);
            }

            CLOSE_TRY;

            sendResponse(
                write_io_hybrid(
                    APDU_GRS_SIGNATURES, SIG_SIZE * RING_PARTICIPANTS, APDU_GENERATE_RING_SIGNATURES_NAME, true),
                true);
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
    ux_display_generate_ringsignatures_splash_1_step,
    pnn,
    do_generate_ring_signatures(),
    {&C_icon_turtlecoin, "Generating", "Ring Signatures..."});

UX_FLOW(ux_display_generate_ringsignatures_splash, &ux_display_generate_ringsignatures_splash_1_step);

UX_STEP_NOCB(ux_display_generate_ringsignatures_flow_1_step, pnn, {&C_icon_turtlecoin, "Generate Ring", "Signatures?"});

UX_STEP_NOCB(
    ux_display_generate_ringsignatures_flow_2_step,
    bnnn_paging,
    {.title = "Output Key", .text = (char *)DISPLAY_KEY_HEX});

UX_STEP_VALID(
    ux_display_generate_ringsignatures_flow_3_step,
    pb,
    ux_flow_init(0, ux_display_generate_ringsignatures_splash, NULL),
    {&C_icon_validate_14, "Approve"});

UX_STEP_VALID(ux_display_generate_ringsignatures_flow_4_step, pb, do_deny(), {&C_icon_crossmark, "Reject"});

UX_FLOW(
    ux_display_generate_ringsignatures_flow,
    &ux_display_generate_ringsignatures_flow_1_step,
    &ux_display_generate_ringsignatures_flow_2_step,
    &ux_display_generate_ringsignatures_flow_3_step,
    &ux_display_generate_ringsignatures_flow_4_step);

void handle_generate_ring_signatures(
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
    else if (dataLength != APDU_GRS_SIZE)
    {
        return sendError(ERR_WRONG_INPUT_LENGTH);
    }

    // copy the data buffer into the working set
    os_memmove(WORKING_SET, dataBuffer, dataLength);

    toHexString(APDU_GRS_OUTPUT_KEY, KEY_SIZE, DISPLAY_KEY_HEX, KEY_HEXSTR_SIZE);

    /**
     * If the APDU was sent requesting confirmation then
     * we need to start the UX flow and set the flags
     * to let the i/o handler know that the request is
     * async and will be completed shortly
     */
    if (p1 == P1_CONFIRM)
    {
        ux_flow_init(0, ux_display_generate_ringsignatures_flow, NULL);

        *flags |= IO_ASYNCH_REPLY;
    }
    else if (p1 == P1_NON_CONFIRM && DEBUG_BUILD == 1)
    {
        ux_flow_init(0, ux_display_generate_ringsignatures_splash, NULL);

        *flags |= IO_ASYNCH_REPLY;
    }
    else
    {
        sendError(ERR_OP_USER_REQUIRED);
    }
}
