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

#include "apdu_check_ringsignatures.h"

#include <keys.h>
#include <utils.h>

#define APDU_CHRS_SIZE KEY_SIZE + KEY_SIZE + (KEY_SIZE * RING_PARTICIPANTS) + (SIG_SIZE * RING_PARTICIPANTS)
#define APDU_CHRS_TX_PREFIX_HASH WORKING_SET
#define APDU_CHRS_KEY_IMAGE APDU_CHRS_TX_PREFIX_HASH + KEY_SIZE
#define APDU_CHRS_PUBLIC_KEYS APDU_CHRS_KEY_IMAGE + KEY_SIZE
#define APDU_CHRS_SIGNATURES APDU_CHRS_PUBLIC_KEYS + (KEY_SIZE * RING_PARTICIPANTS)

static void do_check_ring_signatures()
{
    BEGIN_TRY
    {
        TRY
        {
            unsigned char status = hw_check_ring_signatures(
                APDU_CHRS_TX_PREFIX_HASH, APDU_CHRS_KEY_IMAGE, APDU_CHRS_PUBLIC_KEYS, APDU_CHRS_SIGNATURES);

            sendResponse(write_io_hybrid(&status, sizeof(status), APDU_CHECK_RING_SIGNATURES_NAME, true), true);
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
    ux_check_ring_signatures_1_step,
    pnn,
    do_check_ring_signatures(),
    {&C_icon_turtlecoin, "Checking", "Ring Signatures"});

UX_FLOW(ux_check_ring_signatures_flow, &ux_check_ring_signatures_1_step);

void handle_check_ring_signatures(
    uint8_t p1,
    uint8_t p2,
    uint8_t *dataBuffer,
    uint16_t dataLength,
    volatile unsigned int *flags,
    volatile unsigned int *tx)
{
    UNUSED(p2);

    if (dataLength != APDU_CHRS_SIZE)
    {
        return sendError(ERR_WRONG_INPUT_LENGTH);
    }

    // copy the data buffer into the working set
    os_memmove(WORKING_SET, dataBuffer, dataLength);

    ux_flow_init(0, ux_check_ring_signatures_flow, NULL);

    *flags |= IO_ASYNCH_REPLY;
}