#!/bin/bash

CURRENTDIR=`pwd`

if [[ -v OS ]]; then
  if [[ "${OS}" = "Windows_NT" ]]; then
    CURRENTDIR=`pwd -W`
  fi
fi

NEW_UUID=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 32 | head -n 1)

echo "Launching docker instance: $NEW_UUID for $CURRENTDIR"

docker run --workdir //builder -it -d --rm --name $NEW_UUID -v $CURRENTDIR:/builder ledgerhq/ledger-app-builder:1.6.0
docker exec -it -e BOLOS_SDK=//sdk $NEW_UUID bash -c "make clean && make DEBUG=1"
docker kill $NEW_UUID

