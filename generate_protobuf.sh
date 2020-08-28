#!/bin/bash
CURDIR=$(pwd)
DESTDIR="$CURDIR/src/trezor/proto"
SRCDIR=${SRCDIR:-"$CURDIR/src/trezor/trezor-common/protob"}
PROTOC="${PROTOC_PATH}protoc"

mkdir -p "$DESTDIR"
cd "$SRCDIR"

for i in messages messages-common messages-management messages-ethereum ; do
    $PROTOC --cpp_out="$DESTDIR" -I/usr/include -I. $i.proto
done
