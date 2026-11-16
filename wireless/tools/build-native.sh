#!/bin/sh
# Wireless CarPlay Maps: build owned native overlays, never install them.
set -eu
case "$0" in */*)HERE=${0%/*};;*)exit 2;;esac
SRC=`cd "$HERE/.." && pwd -P`
FROZEN=$1;DEST=$2
CC=${QNX_CC:?Set QNX_CC to your licensed/locally rebuilt ARM QNX EABI5 compiler}
test ! -e "$DEST" || { echo 'Use a new output directory';exit 2; }
mkdir -p "$DEST"
# The reconstructed QNX nm/collect2 pair cannot reliably scan object inputs on
# WSL DrvFS/bind mounts. Build intermediate objects on native Linux /tmp, then
# copy completed outputs. Never delete or reuse the supplied source/SD tree.
WORK=`mktemp -d /tmp/wmaps-native-XXXXXXXX`
case "$WORK" in /tmp/wmaps-native-????????);;*)exit 2;;esac
cleanup(){ code=$?;trap - 0 1 2 15;rm -rf -- "$WORK";exit "$code"; }
trap cleanup 0 1 2 15
OUT="$WORK/artifacts";mkdir "$OUT"
FLAGS='-std=gnu99 -D_QNX_SOURCE -D_POSIX_C_SOURCE=200809L -O2 -Wall -Wextra -Werror -Wno-unused-function -Wno-unused-but-set-variable -Wno-misleading-indentation -mno-unaligned-access -fno-builtin'
"$CC" $FLAGS "$SRC/src/standalone/file-check.c" -o "$OUT/file-check"
"$CC" $FLAGS "$SRC/src/standalone/config-patch.c" -o "$OUT/config-patch"
"$CC" $FLAGS -fPIC -shared -Wl,-z,defs "$SRC/src/primary30/primary30.c" "$SRC/src/primary30/primary30_hook.c" -lc -o "$OUT/libomonob-primary30.so"
"$CC" $FLAGS "$SRC/src/vc-runtime/runtime/carplay_cluster_supervisor.c" -lsocket -o "$OUT/carplay-cluster-supervisor"
for cluster in 790 791;do
 "${CC}" $FLAGS -DCPVC_SHM_ONLY=1 -DCPVC_BUILD_PROFILE=$cluster "$SRC/src/vc-runtime/mirror/carplay_mirror_receiver.c" -Wl,-E -lEGL -lGLESv2 -lsocket -o "$OUT/cpvc-mirror-canvas-$cluster"
 mkdir "$OUT/$cluster"
 set -- "$OUT/$cluster/receiver_hooks.o"
 for name in receiver_interface interface_core map_zoom card_protocol card_channel card_runtime;do
  "$CC" $FLAGS -fPIC -fvisibility=hidden -DV75_CARD_COMPILED_CLUSTER=$cluster -I"$SRC/src/wireless-maps" -c "$SRC/src/wireless-maps/$name.c" -o "$OUT/$cluster/$name.o"
  set -- "$@" "$OUT/$cluster/$name.o"
 done
 "$CC" -fPIC -c "$SRC/src/wireless-maps/receiver_hooks.S" -o "$OUT/$cluster/receiver_hooks.o"
 "$CC" -shared -Wl,-z,defs -Wl,-soname,libomonob-wireless-maps.so "$@" -lc -lsocket -o "$OUT/$cluster/libomonob-wireless-maps.so"
done
for interval in 5 180;do
 "$CC" $FLAGS -DCPMAP_IDR_FRAMES=$interval -I"$SRC/src/wireless-entry" "$SRC/src/wireless-entry/wireless_791_entry.c" -lsocket -o "$OUT/wireless-entry-idr$interval"
done
# Hash headers are generated from THIS build, not hardcoded to another car or
# another compiler output. The frozen receiver itself has no complete C source.
python3 "$HERE/build_hashes.py" "$FROZEN" "$OUT"
for profile in AID12.3 AID10.5-17PM;do
 "$CC" $FLAGS -I"$SRC/src/wireless-boot" -I"$OUT/$profile" "$SRC/src/wireless-boot/receiver_exec.c" -o "$OUT/$profile/wireless-receiver-exec"
done
cp -pR "$OUT"/. "$DEST"/
echo 'Native overlays built only. Do not replace an SD payload without repackaging its manifests and re-running tests.'
