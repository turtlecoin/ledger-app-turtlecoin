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

#ifndef UTILS_H
#define UTILS_H

#include "menu.h"

#include <common.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <varint.h>

#define UX_STEP_TIMEOUT_CB(stepname, layoutkind, timeout_ms, validate_cb, ...) \
    UX_FLOW_CALL(stepname##_validate, { validate_cb; })                        \
    void stepname##_init(unsigned int stack_slot)                              \
    {                                                                          \
        ux_layout_##layoutkind##_init(stack_slot);                             \
        ux_layout_set_timeout(stack_slot, timeout_ms);                         \
    }                                                                          \
    const ux_layout_##layoutkind##_params_t stepname##_val = __VA_ARGS__;      \
    const ux_flow_step_t stepname = {                                          \
        stepname##_init,                                                       \
        &stepname##_val,                                                       \
        stepname##_validate,                                                   \
        NULL,                                                                  \
    }

#define UX_STEP_SPLASH(stepname, layoutkind, validate_cb, ...) \
    UX_STEP_TIMEOUT_CB(stepname, layoutkind, 1, validate_cb, __VA_ARGS__)

typedef enum rlpTxType
{
    TX_LENGTH = 0,
    TX_TYPE,
    TX_SENDER,
    TX_RECIPIENT,
    TX_AMOUNT,
    TX_FEE
} rlpTxType;

unsigned int ui_prepro(const bagl_element_t *element);

uint32_t readUint32BE(uint8_t *buffer);

uint16_t readUint16BE(uint8_t *buffer);

void sendResponse(size_t tx, bool approve);

void sendError(const uint16_t errCode);

void do_deny();

void toHexString(const unsigned char *in, const unsigned int in_len, unsigned char *out, const unsigned int out_len);

size_t write_io(const unsigned char *output, const unsigned char *name, bool hexData);

size_t write_io_hybrid(
    const unsigned char *output,
    const unsigned int output_size,
    const unsigned char *name,
    bool hexData);

size_t write_io_fixed(
    const unsigned char *output,
    const unsigned int output_size,
    const unsigned char *name,
    const unsigned int name_size,
    bool hexData);

#define UI_BUTTONS                                                                                                    \
    {{BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0},                                \
     NULL,                                                                                                            \
     0,                                                                                                               \
     0,                                                                                                               \
     0,                                                                                                               \
     NULL,                                                                                                            \
     NULL,                                                                                                            \
     NULL},                                                                                                           \
        {{BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_CROSS},                       \
         NULL,                                                                                                        \
         0,                                                                                                           \
         0,                                                                                                           \
         0,                                                                                                           \
         NULL,                                                                                                        \
         NULL,                                                                                                        \
         NULL},                                                                                                       \
    {                                                                                                                 \
        {BAGL_ICON, 0x00, 117, 13, 8, 6, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_CHECK}, NULL, 0, 0, 0, NULL, \
            NULL, NULL                                                                                                \
    }

#define UI_FIRST 1
#define UI_SECOND 0

#define UI_LABELINE(userId, text, isFirst, font, horizontalScrollSpeed) \
    {                                                                   \
        {BAGL_LABELINE,                                                 \
         (userId),                                                      \
         23,                                                            \
         (isFirst) ? 12 : 26,                                           \
         82,                                                            \
         12,                                                            \
         (horizontalScrollSpeed) ? BAGL_STROKE_FLAG_ONESHOT | 10 : 0,   \
         0,                                                             \
         0,                                                             \
         0xFFFFFF,                                                      \
         0x000000,                                                      \
         (font) | BAGL_FONT_ALIGNMENT_CENTER,                           \
         horizontalScrollSpeed},                                        \
            (text), 0, 0, 0, NULL, NULL, NULL                           \
    }

#endif // UTILS_H
