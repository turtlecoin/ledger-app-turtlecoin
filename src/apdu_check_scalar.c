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

#include "apdu_check_scalar.h"

#include <keys.h>
#include <transaction.h>
#include <utils.h>

#define APDU_CS_SCALAR WORKING_SET

static void do_check_scalar()
{
    BEGIN_TRY
    {
        TRY
        {
            unsigned char status = hw_check_scalar(APDU_CS_SCALAR);

            sendResponse(write_io_hybrid(&status, sizeof(status), APDU_CHECK_SCALAR_NAME, true), true);
        }
        CATCH_OTHER(e)
        {
            sendError(ERR_CHECK_SCALAR);
        }
        FINALLY
        {
            // Explicitly clear the working memory
            explicit_bzero(WORKING_SET, WORKING_SET_SIZE);
        };
    }
    END_TRY;
}

UX_STEP_SPLASH(ux_check_scalar_1_step, pnn, do_check_scalar(), {&C_icon_turtlecoin, "Checking", "Scalar"});

UX_FLOW(ux_check_scalar_flow, &ux_check_scalar_1_step);

void handle_check_scalar(
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
    else if (dataLength != KEY_SIZE)
    {
        return sendError(ERR_WRONG_INPUT_LENGTH);
    }

    // copy the data buffer into the working set
    os_memmove(WORKING_SET, dataBuffer, dataLength);

    ux_flow_init(0, ux_check_scalar_flow, NULL);

    *flags |= IO_ASYNCH_REPLY;
}