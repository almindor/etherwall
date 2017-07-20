#!/bin/bash
CURDIR=$(pwd)

cd $CURDIR/src/trezor/trezor-common/protob

for i in messages types config storage ; do
    protoc --cpp_out=$CURDIR/src/trezor/proto/ -I/usr/include -I. $i.proto
done
