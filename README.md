# TurtleCoin® Ledger Application

## Overview

This application is designed to run on Ledger Nano S/X hardware wallets. It exposes just the necessary sensitive
operations for keeping your private keys safe while offloading as much of the wallet mechanics as possible to the
host system thereby keeping wallet processes as fast as possible.

## Building and installing

To build and install the app on your Ledger Nano S you must set up the Ledger Nano S build environments.

Please follow the Getting Started instructions via the
[Getting Started Guide](https://ledger.readthedocs.io/en/latest/userspace/getting_started.html).

##### Environment

If you don't want to setup a global environment, you can also setup one just for this app by performing the following:

**Note:** The Nano X SDK is not generally available at this time.

```bash
sudo apt install python3-venv python3-dev libudev-dev libusb-1.0-0-dev

# (s, x, or blue depending on your device)
source prepare-devenv.sh s
```

##### Compilation

Compile the application:
```bash
make
```

Refresh the repo (required after Makefile edits):
```bash
make clean
```

##### Loading the Application

Load the the app on the device

```bash
make load
```

Remove the app from the device:
```bash
make delete
```

## Host Application Flow

The following sequence diagrams depict a "normal" application flow when interacting with the TurtleCoin® Ledger application. As such, it makes the assumption that everything is 100% correct in the data supplied, generated, etc. However, errors may be encountered at any point in the process flow that the host application must account for and handle appropriately.

### Notes & Warnings

1)  While many other APDUs are exposed to developers not all are designed for standard operations from a user-experience perspective. The diagrams below are designed for the *best possible* user experience while managing the security of the private spend key.

2) Due to the relatively low CPU speed of the actual Ledger hardware, it is highly recommended that output scanning and other non-sensitive operations are performed by the host application on the host machine. Yes, the application on the Ledger will perform many of the same operations on the device itself but will do so at a substanially slower speed.

3)  It is considered ***bad form*** to export the private spend key from a device unless absolutely necessary. The keys are created in a deterministic way and can be restored on **any** Ledger device using the seed words for the Ledger device itself. This negates the need to backup the private spend key in this manner. ***Never allow the export of the private spend key on user/production device!***

4) A `*` after the APDU command indicates that the command must be sent with `P1` set to `0x01` on *Release* builds of the TurtleCoin® Ledger application. Any other values for `P1` will **fail**. If you are interacting with a *Debug* build of the application, you *may* specify `0x00` for `P1` and skip waiting for the user confirmation. 

### DO NOT RUN DEBUG BUILDS ON USER AND/OR PRODUCTION DEVICES

Due to #3 & #4 above, it's really easy to grab the private spend key out of a debug build. To avoid having your private key stolen and and all of your funds magically disappear, never run a debug build on your "normal" device.

We highly recommend that you work with a separate hardware device entirely when working with debug builds; however, at the very least, reset the device entirely and generate a **brand new ledger seed** whenever you move between Debug and Release versions of the application.

***You have been warned.***

### Initialization

```sequence
Participant Host
Participant Ledger
Host->Ledger: APDU_VERSION
Ledger->Host: {version}
Note left of Host: Confirm compatible\nLedger app version
Host->Ledger: APDU_IDENT
Ledger->Host: {ident}
Note left of Host: Confirm valid ident
Host->Ledger: APDU_PUBLIC_KEYS*
Note right of Ledger: User confirms\nor denies
Ledger->Host: OK\n{public_spend_key}\n{public_view_key}
Host->Ledger: APDU_VIEW_SECRET_KEY*
Note right of Ledger: User confirms\nor denies
Ledger->Host: OK\n{private_view_key}
```

### Output Scanning

```sequence
Participant Host
Participant Ledger
Note left of Host: Performs normal output\n scanning and when it finds\nan output belonging to this\nwallet, it will proceed to
Host->Ledger: APDU_GENERATE_KEYIMAGE*\n \n{tx_public_key}\n{output_index}\n{public_ephemeral}
Note right of Ledger: User confirms\nor denies
Ledger->Host: OK\n{key_image}
Note left of Host: Host saves the key image,\ntx_public_key, and output_index\nin the wallet to recognize \nspent funds and construct\ntransactions later
```

### Transaction Construction & Signing

```sequence
Participant Host
Participant Ledger
Note left of Host: Constructs high-level\ntransaction inputs\nand outputs using\nnormal means and methods
Host->Ledger: APDU_TX_START\n \n{unlock_time}\n{input_count}\n{output_count}\n{new_tx_public_key}\n{has_payment_id}\n<{payment_id}>
Note right of Ledger: Switches state\nto transaction\nmode
Ledger->Host: OK
Host->Ledger: APDU_TX_START_INPUT_LOAD
Note right of Ledger: Prepare to recieve\ninputs
Ledger->Host: OK
Note left of Host: Host loops through and\nloads as many inputs\nas necessary to create\nthe transaction
Host->Ledger: APDU_TX_LOAD_INPUT\n \n{input_tx_public_key}\n{input_output_index}\n{amount}\n{[public_keys]}\n{[relative_offsets]}\n{real_output_index}
Note right of Ledger: Processes the input and\nconfirms that the output\nbeing spent belongs to\nthe keys the app holds
Ledger->Host: OK
Note left of Host: After all inputs are\nloaded...
Host->Ledger: APDU_TX_START_OUTPUT_LOAD
Note right of Ledger: Prepare to receive\noutputs
Ledger->Host: OK
Note left of Host: Host loops through and\nloads as many outputs\nas necessary to create\nthe transaction\nincluding any\n"change" outputs
Host->Ledger: APDU_TX_LOAD_OUTPUT\n \n{amount}\n{output_key}
Note right of Ledger: Processes the\noutput and adds\nit to the\ntransaction
Ledger->Host: OK
Note left of Host: After all outputs are\nloaded...
Host->Ledger: APDU_TX_FINALIZE_PREFIX
Note right of Ledger: Complete the rest\nof the transaction\nprefix including\nTX_EXTRA
Ledger->Host: OK
Host->Ledger: APDU_TX_SIGN*
Note right of Ledger: User confirms\nor denies
Ledger->Host: OK\n \n{tx_hash}\n{tx_size}
Note left of Host: Loops through the\nthe following until\nthe full transaction\nhas been received
Host->Ledger: APDU_TX_DUMP\n \n{start_offset}
Note right of Ledger: Selects the next chunk\nof bytes from the\ntransaction
Ledger->Host: OK\n \n{raw_transaction_chunk}
Note left of Host: Sends the full raw\ntransaction to a daemon\nfor acceptance by the\nnetwork
Host->Ledger: APDU_TX_RESET
Note right of Ledger: User confirms\nor denies
Note right of Ledger: Clears transaction from\nmemory and returns the\ndevice to normal working\nmode
Ledger->Host: OK
```