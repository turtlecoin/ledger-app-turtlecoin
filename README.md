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

Please see the application process flow notes [here](https://hackmd.io/@ZL2uKk4cThC4TG0z7Wu7sg/ryg3Inbzw).