#!/bin/sh
# Existing GNU Neutrino image only; no proprietary SDK/compiler invocation.
set -eu
cc=arm-unknown-nto-qnx6.5.0eabi-gcc
re=arm-unknown-nto-qnx6.5.0eabi-readelf
test "$("$cc" -dumpversion)" = 4.9.4
publish=${CPMAP_INFORMATION_BUILD_OUT:-/src/build-information-native}
test ! -e "$publish"
out=$(mktemp -d /tmp/cpmap-information-XXXXXX)
flags='-std=gnu99 -O2 -Wall -Wextra -mno-unaligned-access'
src=/src/src/information
lib=$(mktemp -d /tmp/cpmap-private-link-XXXXXX)
private=$(mktemp -d /tmp/cpmap-private-symbols-XXXXXX)
# Old 32-bit binutils cannot stat some high-inode DrvFS files (EOVERFLOW).
# Copy read-only fixtures into the disposable container filesystem first.
cp -r /fixtures/. "$private/"
cp "$private/MU1447/armle/usr/lib/libiap2client.so.1" "$private/MU1447/armle/usr/lib/libstrm.so.1" "$lib/"
"$cc" $flags -fPIC -shared "$src/native/libcpvc-rgd-mhi2.c" -o "$out/libcpvc-rgd.so"
"$cc" $flags "$src/native/cpvc-rgd-sync.c" -o "$out/cpvc-rgd-sync"
"$cc" $flags /src/src/toolbox/cpmap-nav-check.c -o "$out/cpmap-nav-check"
# Only resolve against the user's original client library; it is NOT copied
# into the output or release. The target firmware provides this dependency.
"$cc" $flags "$src/native/carplay_artwork_helper.c" -L"$lib" -Wl,-rpath-link,"$lib" -l:libiap2client.so.1 -lm -o "$out/carplay_artwork_helper"
for binary in libcpvc-rgd.so cpvc-rgd-sync carplay_artwork_helper; do
 "$re" -W -h -A -d "$out/$binary" > "$out/$binary.readelf.txt"
 "$re" -W -s "$out/$binary" > "$out/$binary.symbols.txt"
done
for mu in MU1102 MU1367 MU1440 MU1433 MU1447; do
 "$re" -W -D -s "$private/$mu/armle/lib/dll/ipod-drvr-iap2.so" > "$out/$mu-driver-symbols.txt"
 for symbol in ident_info_funcs iap2_ctrl_msg_table packet_sendlist_pop packet_sendlist_push link_send_message packet_addparam_blob; do
  grep -Eq "[[:space:]]$symbol$" "$out/$mu-driver-symbols.txt" || { echo "Missing $mu $symbol";exit 1; }
 done
 "$re" -W -D -s "$private/$mu/armle/usr/lib/libiap2client.so.1" > "$out/$mu-client-symbols.txt"
 for symbol in iap2_connect iap2_disconnect iap2_filexfer_read; do
  grep -Eq "[[:space:]]$symbol$" "$out/$mu-client-symbols.txt" || exit 1
 done
 echo "INFORMATION_NATIVE_SYMBOLS_PASS=$mu exact_private_firmware=1 on_car_ABI=not_yet_tested"
done
host='/usr/bin/cc -B/usr/bin/ -std=gnu99 -O1 -g -Wall -Wextra -Wno-unused-function -fno-pie -no-pie -fsanitize=address,undefined'
$host "$src/tests/test_artwork.c" -lm -o "$out/artwork-host"
ASAN_OPTIONS=halt_on_error=1:detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 "$out/artwork-host"
$host -D_DEFAULT_SOURCE "$src/native/cpvc-rgd-sync.c" -o "$out/sync-host"
ASAN_OPTIONS=halt_on_error=1:detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 "$out/sync-host" --selftest-log-follow
python3 /src/tests/test_information_sync.py "$out/sync-host"
mkdir -p "$publish"
cp "$out/libcpvc-rgd.so" "$out/cpvc-rgd-sync" "$out/carplay_artwork_helper" "$out/cpmap-nav-check" "$publish/"
cp "$out/"*.txt "$publish/"
echo INFORMATION_NATIVE_BUILD_HOST_PASS_REAL_VEHICLE_TEST_REQUIRED
