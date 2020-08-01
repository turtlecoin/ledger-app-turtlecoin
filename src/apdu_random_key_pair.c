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

#include "apdu_random_key_pair.h"

#include <keys.h>
#include <utils.h>

static void do_generate_random_key_pair()
{
    unsigned char combined[64] = {0};

    BEGIN_TRY
    {
        TRY
        {
            if (hw_generate_keypair(combined, combined + KEY_SIZE) != 0)
            {
                THROW(ERR_UNKNOWN_ERROR);
            }

            /**
             * This is static non-privileged information and as thus
             * can be returned without any additional checking
             */
            sendResponse(write_io_hybrid(combined, sizeof(combined), APDU_RANDOM_KEY_PAIR_NAME, true), true);
        }
        CATCH_OTHER(e)
        {
            sendError(ERR_UNKNOWN_ERROR);
        }
        FINALLY
        {
            // Explicitly clear the working memory
            explicit_bzero(combined, sizeof(combined));
        }
    }
    END_TRY;
}

UX_STEP_SPLASH(
    ux_generate_random_key_pair_flow_1_step,
    pnn,
    do_generate_random_key_pair(),
    {&C_icon_turtlecoin, "Generating", "Random Keys..."});

UX_FLOW(ux_generate_random_key_pair_flow, &ux_generate_random_key_pair_flow_1_step);

void handle_generate_random_key_pair(volatile unsigned int *flags)
{
    ux_flow_init(0, ux_generate_random_key_pair_flow, NULL);

    *flags |= IO_ASYNCH_REPLY;
}