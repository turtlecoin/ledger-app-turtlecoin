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

#include "apdu_private_to_public.h"

#include <keys.h>
#include <utils.h>

#define APDU_PTP_SIZE KEY_SIZE
#define APDU_PTP_PRIVATE_KEY WORKING_SET
#define APDU_PTP_PUBLIC_KEY APDU_PTP_PRIVATE_KEY + KEY_SIZE

static void do_private_to_public()
{
    BEGIN_TRY
    {
        TRY
        {
            if (hw_private_key_to_public_key(APDU_PTP_PUBLIC_KEY, APDU_PTP_PRIVATE_KEY) != 0)
            {
                THROW(ERR_PRIVATE_TO_PUBLIC);
            }

            CLOSE_TRY;

            sendResponse(write_io_hybrid(APDU_PTP_PUBLIC_KEY, KEY_SIZE, APDU_PRIVATE_TO_PUBLIC_NAME, true), true);
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

UX_STEP_SPLASH(ux_private_to_public_1_step, pnn, do_private_to_public(), {&C_icon_turtlecoin, "Checking", "Signature"});

UX_FLOW(ux_private_to_public_flow, &ux_private_to_public_1_step);

void handle_private_to_public(
    uint8_t p1,
    uint8_t p2,
    uint8_t *dataBuffer,
    uint16_t dataLength,
    volatile unsigned int *flags,
    volatile unsigned int *tx)
{
    UNUSED(p2);

    if (dataLength != APDU_PTP_SIZE)
    {
        return sendError(ERR_WRONG_INPUT_LENGTH);
    }

    // copy the data buffer into the working set
    os_memmove(WORKING_SET, dataBuffer, dataLength);

    ux_flow_init(0, ux_private_to_public_flow, NULL);

    *flags |= IO_ASYNCH_REPLY;
}