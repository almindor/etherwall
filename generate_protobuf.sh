#!/bin/bash
CURDIR=$(pwd)
DESTDIR="$CURDIR/src/trezor/proto"
SRCDIR=${SRCDIR:-"$CURDIR/src/trezor/trezor-common/protob"}

mkdir -p "$DESTDIR"
cd "$SRCDIR"

for i in messages messages-common messages-management messages-ethereum ; do
    protoc --cpp_out="$DESTDIR" -I/usr/include -I. $i.proto
done
