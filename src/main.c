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

#include "apdu.h"
#include "menu.h"

unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

#define CLA 0xE0

#define OFFSET_CLA 0
#define OFFSET_INS 1
#define OFFSET_P1 2
#define OFFSET_P2 3
#define OFFSET_LC 4
#define OFFSET_CDATA 6

void handleApdu(volatile unsigned int *flags, volatile unsigned int *tx)
{
    unsigned short sw = 0;

    BEGIN_TRY
    {
        TRY
        {
            // Explicitly clear the working memory
            explicit_bzero(WORKING_SET, WORKING_SET_SIZE);

            // Explicitly clear any display information
            explicit_bzero(DISPLAY_KEY_HEX, KEY_HEXSTR_SIZE);

            if (G_io_apdu_buffer[OFFSET_CLA] != CLA)
            {
                THROW(0x6E00);
            }

            uint16_t data_length = data_length = readUint16BE((uint8_t *)&G_io_apdu_buffer[OFFSET_LC]);

            switch (G_io_apdu_buffer[OFFSET_INS])
            {
                case APDU_VERSION:
                    handle_version();
                    break;

                case APDU_DEBUG:
                    handle_debug();
                    break;

                case APDU_IDENT:
                    handle_ident();
                    break;

                case APDU_PUBLIC_KEYS:
                    handle_public_keys(G_io_apdu_buffer[OFFSET_P1], G_io_apdu_buffer[OFFSET_P2], flags, tx);
                    break;

                case APDU_VIEW_SECRET_KEY:
                    handle_view_secret_key(G_io_apdu_buffer[OFFSET_P1], G_io_apdu_buffer[OFFSET_P2], flags, tx);
                    break;

                case APDU_SPEND_SECRET_KEY:
                    handle_spend_secret_key(G_io_apdu_buffer[OFFSET_P1], G_io_apdu_buffer[OFFSET_P2], flags, tx);
                    break;

                case APDU_VIEW_WALLET_KEYS:
                    handle_view_wallet_keys(G_io_apdu_buffer[OFFSET_P1], G_io_apdu_buffer[OFFSET_P2], flags, tx);
                    break;

                case APDU_PRIVATE_TO_PUBLIC:
                    handle_private_to_public(
                        G_io_apdu_buffer[OFFSET_P1],
                        G_io_apdu_buffer[OFFSET_P2],
                        G_io_apdu_buffer + OFFSET_CDATA,
                        data_length,
                        flags,
                        tx);
                    break;

                case APDU_RANDOM_KEY_PAIR:
                    handle_generate_random_key_pair(flags);
                    break;

                case APDU_CHECK_KEY:
                    handle_check_key(
                        G_io_apdu_buffer[OFFSET_P1],
                        G_io_apdu_buffer[OFFSET_P2],
                        G_io_apdu_buffer + OFFSET_CDATA,
                        data_length,
                        flags,
                        tx);
                    break;

                case APDU_CHECK_SCALAR:
                    handle_check_scalar(
                        G_io_apdu_buffer[OFFSET_P1],
                        G_io_apdu_buffer[OFFSET_P2],
                        G_io_apdu_buffer + OFFSET_CDATA,
                        data_length,
                        flags,
                        tx);
                    break;

                case APDU_ADDRESS:
                    handle_address(G_io_apdu_buffer[OFFSET_P1], G_io_apdu_buffer[OFFSET_P2], flags, tx);
                    break;

                case APDU_GENERATE_KEYIMAGE:
                    handle_generate_keyimage(
                        G_io_apdu_buffer[OFFSET_P1],
                        G_io_apdu_buffer[OFFSET_P2],
                        G_io_apdu_buffer + OFFSET_CDATA,
                        data_length,
                        flags,
                        tx);
                    break;

                case APDU_GENERATE_KEYIMAGE_PRIMITIVE:
                    handle_generate_keyimage_primitive(
                        G_io_apdu_buffer[OFFSET_P1],
                        G_io_apdu_buffer[OFFSET_P2],
                        G_io_apdu_buffer + OFFSET_CDATA,
                        data_length,
                        flags,
                        tx);
                    break;

                case APDU_GENERATE_RING_SIGNATURES:
                    handle_generate_ring_signatures(
                        G_io_apdu_buffer[OFFSET_P1],
                        G_io_apdu_buffer[OFFSET_P2],
                        G_io_apdu_buffer + OFFSET_CDATA,
                        data_length,
                        flags,
                        tx);
                    break;

                case APDU_COMPLETE_RING_SIGUATURE:
                    handle_complete_ring_signature(
                        G_io_apdu_buffer[OFFSET_P1],
                        G_io_apdu_buffer[OFFSET_P2],
                        G_io_apdu_buffer + OFFSET_CDATA,
                        data_length,
                        flags,
                        tx);
                    break;

                case APDU_CHECK_RING_SIGNATURES:
                    handle_check_ring_signatures(
                        G_io_apdu_buffer[OFFSET_P1],
                        G_io_apdu_buffer[OFFSET_P2],
                        G_io_apdu_buffer + OFFSET_CDATA,
                        data_length,
                        flags,
                        tx);
                    break;

                case APDU_GENERATE_SIGNATURE:
                    handle_generate_signature(
                        G_io_apdu_buffer[OFFSET_P1],
                        G_io_apdu_buffer[OFFSET_P2],
                        G_io_apdu_buffer + OFFSET_CDATA,
                        data_length,
                        flags,
                        tx);
                    break;

                case APDU_CHECK_SIGNATURE:
                    handle_check_signature(
                        G_io_apdu_buffer[OFFSET_P1],
                        G_io_apdu_buffer[OFFSET_P2],
                        G_io_apdu_buffer + OFFSET_CDATA,
                        data_length,
                        flags,
                        tx);
                    break;

                case APDU_GENERATE_KEY_DERIVATION:
                    handle_generate_key_derivation(
                        G_io_apdu_buffer[OFFSET_P1],
                        G_io_apdu_buffer[OFFSET_P2],
                        G_io_apdu_buffer + OFFSET_CDATA,
                        data_length,
                        flags,
                        tx);
                    break;

                case APDU_DERIVE_PUBLIC_KEY:
                    handle_derive_public_key(
                        G_io_apdu_buffer[OFFSET_P1],
                        G_io_apdu_buffer[OFFSET_P2],
                        G_io_apdu_buffer + OFFSET_CDATA,
                        data_length,
                        flags,
                        tx);
                    break;

                case APDU_DERIVE_SECRET_KEY:
                    handle_derive_secret_key(
                        G_io_apdu_buffer[OFFSET_P1],
                        G_io_apdu_buffer[OFFSET_P2],
                        G_io_apdu_buffer + OFFSET_CDATA,
                        data_length,
                        flags,
                        tx);
                    break;

                case APDU_TX_STATE:
                    handle_tx_state();
                    break;

                case APDU_TX_START:
                    handle_tx_start(
                        G_io_apdu_buffer[OFFSET_P1],
                        G_io_apdu_buffer[OFFSET_P2],
                        G_io_apdu_buffer + OFFSET_CDATA,
                        data_length,
                        flags,
                        tx);
                    break;

                case APDU_TX_START_INPUT_LOAD:
                    handle_tx_start_input_load(flags);
                    break;

                case APDU_TX_LOAD_INPUT:
                    handle_tx_input_load(
                        G_io_apdu_buffer[OFFSET_P1],
                        G_io_apdu_buffer[OFFSET_P2],
                        G_io_apdu_buffer + OFFSET_CDATA,
                        data_length,
                        flags,
                        tx);
                    break;

                case APDU_TX_START_OUTPUT_LOAD:
                    handle_tx_start_output_load(flags);
                    break;

                case APDU_TX_LOAD_OUTPUT:
                    handle_tx_output_load(
                        G_io_apdu_buffer[OFFSET_P1],
                        G_io_apdu_buffer[OFFSET_P2],
                        G_io_apdu_buffer + OFFSET_CDATA,
                        data_length,
                        flags,
                        tx);
                    break;

                case APDU_TX_FINALIZE_PREFIX:
                    handle_tx_finalize_prefix(flags);
                    break;

                case APDU_TX_SIGN:
                    handle_tx_sign(G_io_apdu_buffer[OFFSET_P1], G_io_apdu_buffer[OFFSET_P2], flags, tx);
                    break;

                case APDU_TX_DUMP:
                    handle_tx_dump(
                        G_io_apdu_buffer[OFFSET_P1],
                        G_io_apdu_buffer[OFFSET_P2],
                        G_io_apdu_buffer + OFFSET_CDATA,
                        data_length,
                        flags,
                        tx);
                    break;

                case APDU_TX_RESET:
                    handle_tx_reset(G_io_apdu_buffer[OFFSET_P1], G_io_apdu_buffer[OFFSET_P2], flags, tx);
                    break;

                case APDU_RESET_KEYS:
                    handle_reset(G_io_apdu_buffer[OFFSET_P1], G_io_apdu_buffer[OFFSET_P2], flags, tx);
                    break;

                default:
                    THROW(0x6D00);
                    break;
            }
        }
        CATCH(EXCEPTION_IO_RESET)
        {
            THROW(EXCEPTION_IO_RESET);
        }
        CATCH_OTHER(e)
        {
            switch (e & 0xF000)
            {
                case 0x6000:
                    sw = e;
                    break;
                case 0x9000:
                    // All is well
                    sw = e;
                    break;
                default:
                    // Internal error
                    sw = 0x6800 | (e & 0x7FF);
                    break;
            }

            // Unexpected exception => report
            G_io_apdu_buffer[*tx] = sw >> 8;

            G_io_apdu_buffer[*tx + 1] = sw;

            *tx += 2;
        }
        FINALLY {}
    }
    END_TRY;
}

void app_main(void)
{
    volatile unsigned int rx = 0;
    volatile unsigned int tx = 0;
    volatile unsigned int flags = 0;

    // DESIGN NOTE: the bootloader ignores the way APDU are fetched. The only
    // goal is to retrieve APDU.
    // When APDU are to be fetched from multiple IOs, like NFC+USB+BLE, make
    // sure the io_event is called with a
    // switch event, before the apdu is replied to the bootloader. This avoid
    // APDU injection faults.
    for (;;)
    {
        volatile unsigned short sw = 0;

        BEGIN_TRY
        {
            TRY
            {
                rx = tx;

                tx = 0; // ensure no race in catch_other if io_exchange throws
                        // an error
                rx = io_exchange(CHANNEL_APDU | flags, rx);

                flags = 0;

                // no apdu received, well, reset the session, and reset the
                // bootloader configuration
                if (rx == 0)
                {
                    THROW(0x6982);
                }

                PRINTF("New APDU received:\n%.*H\n", rx, G_io_apdu_buffer);

                handleApdu(&flags, &tx);
            }
            CATCH(EXCEPTION_IO_RESET)
            {
                THROW(EXCEPTION_IO_RESET);
            }
            CATCH_OTHER(e)
            {
                switch (e & 0xF000)
                {
                    case 0x6000:
                        sw = e;
                        break;
                    case 0x9000:
                        // All is well
                        sw = e;
                        break;
                    default:
                        // Internal error
                        sw = 0x6800 | (e & 0x7FF);
                        break;
                }

                if (e != 0x9000)
                {
                    flags &= ~IO_ASYNCH_REPLY;
                }

                // Unexpected exception => report
                G_io_apdu_buffer[tx] = sw >> 8;

                G_io_apdu_buffer[tx + 1] = sw;

                tx += 2;
            }
            FINALLY {}
        }
        END_TRY;
    }
    // return_to_dashboard:
    return;
}

// override point, but nothing more to do
void io_seproxyhal_display(const bagl_element_t *element)
{
    io_seproxyhal_display_default((bagl_element_t *)element);
}

unsigned char io_event(unsigned char channel)
{
    // nothing done with the event, throw an error on the transport layer if
    // needed

    // can't have more than one tag in the reply, not supported yet.
    switch (G_io_seproxyhal_spi_buffer[0])
    {
        case SEPROXYHAL_TAG_FINGER_EVENT:
            UX_FINGER_EVENT(G_io_seproxyhal_spi_buffer);
            break;

        case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT:
            UX_BUTTON_PUSH_EVENT(G_io_seproxyhal_spi_buffer);
            break;

        case SEPROXYHAL_TAG_STATUS_EVENT:
            if (G_io_apdu_media == IO_APDU_MEDIA_USB_HID
                && !(U4BE(G_io_seproxyhal_spi_buffer, 3) & SEPROXYHAL_TAG_STATUS_EVENT_FLAG_USB_POWERED))
            {
                THROW(EXCEPTION_IO_RESET);
            }
            // no break is intentional

        default:
            UX_DEFAULT_EVENT();
            break;

        case SEPROXYHAL_TAG_DISPLAY_PROCESSED_EVENT:
            UX_DISPLAYED_EVENT({});
            break;

        case SEPROXYHAL_TAG_TICKER_EVENT:
            UX_TICKER_EVENT(G_io_seproxyhal_spi_buffer, {
#ifndef TARGET_NANOX
                if (UX_ALLOWED)
                {
                    if (ux_step_count)
                    {
                        // prepare next screen
                        ux_step = (ux_step + 1) % ux_step_count;

                        // redisplay screen
                        UX_REDISPLAY();
                    }
                }
#endif // TARGET_NANOX
            });
            break;
    }

    // close the event if not done previously (by a display or whatever)
    if (!io_seproxyhal_spi_is_status_sent())
    {
        io_seproxyhal_general_status();
    }

    // command has been processed, DO NOT reset the current APDU transport
    return 1;
}

unsigned short io_exchange_al(unsigned char channel, unsigned short tx_len)
{
    switch (channel & ~(IO_FLAGS))
    {
        case CHANNEL_KEYBOARD:
            break;

        // multiplexed io exchange over a SPI channel and TLV encapsulated protocol
        case CHANNEL_SPI:
            if (tx_len)
            {
                io_seproxyhal_spi_send(G_io_apdu_buffer, tx_len);

                if (channel & IO_RESET_AFTER_REPLIED)
                {
                    reset();
                }

                return 0; // nothing received from the master so far (it's a tx
                          // transaction)
            }
            else
            {
                return io_seproxyhal_spi_recv(G_io_apdu_buffer, sizeof(G_io_apdu_buffer), 0);
            }

        default:
            THROW(INVALID_PARAMETER);
    }

    return 0;
}

void app_exit(void)
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

__attribute__((section(".boot"))) int main(void)
{
    // exit critical section
    __asm volatile("cpsie i");

    // ensure exception will work as planned
    os_boot();

    // Explicitly clear the working memory
    explicit_bzero(WORKING_SET, WORKING_SET_SIZE);

    for (;;)
    {
        UX_INIT();

        BEGIN_TRY
        {
            TRY
            {
                io_seproxyhal_init();

                USB_power(0);

                USB_power(1);

#ifdef HAVE_BLE
                BLE_power(0, NULL);

                BLE_power(1, "Nano X - TurtleCoin");
#endif // HAVE_BLE

                ui_splash();

                app_main();
            }
            CATCH(EXCEPTION_IO_RESET)
            {
                // reset IO and UX before continuing
                continue;
            }
            CATCH_ALL
            {
                break;
            }
            FINALLY {}
        }
        END_TRY;
    }

    app_exit();

    return 0;
}