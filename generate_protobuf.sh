#!/bin/bash
CURDIR=$(pwd)
DESTDIR="$CURDIR/src/trezor/proto"
SRCDIR="$CURDIR/src/trezor/trezor-common/protob"

mkdir -p "$DESTDIR"
cd "$SRCDIR"

for i in messages types config storage ; do
    protoc --cpp_out="$DESTDIR" -I/usr/include -I. $i.proto
done
