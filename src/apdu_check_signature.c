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

#include "apdu_check_signature.h"

#include <keys.h>
#include <transaction.h>
#include <utils.h>

#define APDU_CS_SIZE KEY_SIZE + KEY_SIZE + SIG_SIZE
#define APDU_CS_MESSAGE_DIGEST WORKING_SET
#define APDU_CS_PUBLIC_KEY APDU_CS_MESSAGE_DIGEST + KEY_SIZE
#define APDU_CS_SIGNATURE APDU_CS_PUBLIC_KEY + KEY_SIZE

static void do_check_signature()
{
    BEGIN_TRY
    {
        TRY
        {
            unsigned char status = hw_check_signature(APDU_CS_MESSAGE_DIGEST, APDU_CS_PUBLIC_KEY, APDU_CS_SIGNATURE);

            sendResponse(write_io_hybrid(&status, sizeof(status), APDU_CHECK_SIGNATURE_NAME, true), true);
        }
        CATCH_OTHER(e)
        {
            sendError(ERR_CHECK_SIGNATURE);
        }
        FINALLY
        {
            // Explicitly clear the working memory
            explicit_bzero(WORKING_SET, WORKING_SET_SIZE);
        }
    }
    END_TRY;
}

UX_STEP_SPLASH(ux_check_signature_1_step, pnn, do_check_signature(), {&C_icon_turtlecoin, "Checking", "Signature"});

UX_FLOW(ux_check_signature_flow, &ux_check_signature_1_step);

void handle_check_signature(
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
    else if (dataLength != APDU_CS_SIZE)
    {
        return sendError(ERR_WRONG_INPUT_LENGTH);
    }

    // copy the data buffer into the working set
    os_memmove(WORKING_SET, dataBuffer, dataLength);

    ux_flow_init(0, ux_check_signature_flow, NULL);

    *flags |= IO_ASYNCH_REPLY;
}