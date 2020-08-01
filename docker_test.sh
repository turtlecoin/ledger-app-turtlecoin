#!/bin/bash

CURRENTDIR=`pwd`

if [[ -v OS ]]; then
  if [[ "${OS}" = "Windows_NT" ]]; then
    CURRENTDIR=`pwd -W`
  fi
fi

NEW_UUID=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 32 | head -n 1)

echo "Launching docker instance: $NEW_UUID for $CURRENTDIR"

docker run --workdir //builder -it -d --rm --name $NEW_UUID --entrypoint //bin/bash -v $CURRENTDIR:/builder -p 9999:9999 -p 42000:42000 -p 43000:43000 ledgerhq/speculos:latest
docker exec -d $NEW_UUID python //speculos/speculos.py bin/app.elf --sdk 1.6 --seed "stunning cool hexagon criminal dabbing dads magically enjoy ourselves ingested ongoing southern greater cigar serving pheasants pawnshop mystery powder sizes randomly melting custom kernels hexagon" --apdu-port 9999 --button-port 42000 --automation-port 43000 --display headless &
cd tests && npm ci && npm test && cd $CURRENTDIR
docker kill $NEW_UUID