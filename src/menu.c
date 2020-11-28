/*******************************************************************************
 *   Ledger Blue
 *   (c) 2016 Ledger
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
 ********************************************************************************/

#include "menu.h"

#include <transaction.h>

#define DISPLAY_ADDRESS WORKING_SET

static void app_kill(void)
{
    BEGIN_TRY_L(exit)
    {
        TRY_L(exit)
        {
            os_sched_exit(-1);
        }
        FINALLY_L(exit) {}
    }
    END_TRY_L(exit);
}

static void init_boot()
{
    if (init_keys() != OP_OK)
    {
        app_kill();
    }

    if (init_tx() != OP_OK)
    {
        app_kill();
    }

    ui_idle();
}

UX_STEP_VALID(ux_idle_addr_flow_1_step, bnnn_paging, ui_idle(), {.title = "Address", .text = (char *)DISPLAY_ADDRESS});

UX_FLOW(ux_idle_addr_flow, &ux_idle_addr_flow_1_step);

UX_STEP_NOCB(ux_idle_flow_1_step, pnn, {&C_icon_turtlecoin, "TurtleCoin", "  is Ready"});

UX_STEP_VALID(ux_idle_flow_2_step, pb, ui_display_address(), {&C_icon_turtlecoin, "Show Wallet", "Address"});

UX_STEP_NOCB(ux_idle_flow_3_step, bn, {"Version", APPVERSION});

UX_STEP_VALID(ux_idle_flow_4_step, pb, os_sched_exit(-1), {&C_icon_dashboard_x, "Quit"});

UX_FLOW(
    ux_idle_flow,
    &ux_idle_flow_1_step,
    &ux_idle_flow_2_step,
    &ux_idle_flow_3_step,
    &ux_idle_flow_4_step,
    FLOW_LOOP);

UX_STEP_SPLASH(ux_boot_splash_step_1, pnn, init_boot(), {&C_icon_turtlecoin, "Safe, Fun &", "  Easy...  "});

UX_FLOW(ux_boot_splash_flow, &ux_boot_splash_step_1);

void ui_idle()
{
    // reserve a display stack slot if none yet
    if (G_ux.stack_count == 0)
    {
        ux_stack_push();
    }

    ux_flow_init(0, ux_idle_flow, NULL);
}

void ui_splash()
{
    if (G_ux.stack_count == 0)
    {
        ux_stack_push();
    }

    ux_flow_init(0, ux_boot_splash_flow, NULL);
}

void ui_display_address()
{
    if (tx_state() != TX_UNUSED)
    {
        return;
    }

    const uint16_t status = generate_public_address(PTR_SPEND_PUBLIC, PTR_VIEW_PUBLIC, DISPLAY_ADDRESS);

    if (G_ux.stack_count == 0)
    {
        ux_stack_push();
    }

    ux_flow_init(0, ux_idle_addr_flow, NULL);
}