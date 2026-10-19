#!/bin/sh
# Builds libcardshim.so for MHI2 (QNX 6.5, ARMv7) without the QNX SDK licence:
# compile with clang on the host, link with ntoarmv7-ld from
# https://github.com/luka-dev/qnx65-armv7-toolchain (an i386 Linux binary, run in Docker).
#
#   QNX_TC=/path/to/qnx65-armv7-toolchain ./build.sh
set -e

HERE=$(cd "$(dirname "$0")" && pwd)
OUT="$HERE/build"
: "${QNX_TC:?set QNX_TC to a clone of luka-dev/qnx65-armv7-toolchain}"
mkdir -p "$OUT"

clang -target armv7a-none-eabi -mcpu=cortex-a9 -marm -mfloat-abi=softfp -mfpu=vfpv3-d16 \
    -fPIC -O2 -ffreestanding -fno-builtin -fno-stack-protector \
    -fno-unwind-tables -fno-asynchronous-unwind-tables -Wall \
    -c "$HERE/cardshim.c" -o "$OUT/cardshim.o"

docker run --rm --platform linux/386 -v "$QNX_TC/sdp:/sdp:ro" -v "$OUT:/w" debian:bullseye-slim \
    /sdp/host/linux/x86/usr/bin/ntoarmv7-ld -shared -soname libcardshim.so --hash-style=sysv \
    -o /w/libcardshim.so /w/cardshim.o -L/sdp/target/qnx6/armle-v7/lib -lc

shasum -a 256 "$OUT/libcardshim.so"
