#!/bin/sh
# Wireless CarPlay Maps. Maintainer: omonob. Source date: 2026-08-29.
# No private guard, information parser, welcome sound or startup animation.
ROOT=/mnt/app/carplaymaps
unset LD_PRELOAD CARPLAY_CLUSTER_INTRO CARPLAY_CLUSTER_INTRO_FPS
export IPL_CONFIG_DIR=/etc/eso/production
export LD_LIBRARY_PATH=/mnt/app/root/lib-target:/eso/lib:/mnt/app/usr/lib:/mnt/app/armle/lib:/mnt/app/armle/lib/dll:/mnt/app/armle/usr/lib
test -x "$ROOT/wireless-entry" || exit 1
p=`cat /ramdisk/omonob-791-input.pid 2>/dev/null`
case "$p" in ''|*[!0-9]*)p=;;esac
if test -z "$p" || ! kill -0 "$p" 2>/dev/null;then
 "$ROOT/wireless-entry" --serve </dev/null >/ramdisk/omonob-791-input.log 2>&1 &
 echo $! > /ramdisk/omonob-791-input.pid
fi
# This existing supervisor has an fcntl singleton; no extra MOST writer.
"$ROOT/carplay-cluster-supervisor" </dev/null >/dev/null 2>&1 &
exit 0
