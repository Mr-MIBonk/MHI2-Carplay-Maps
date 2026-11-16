#!/bin/ksh
ROOT=/mnt/app/eso/hmi/engdefs/scripts/wireless-carplay-maps
[ -f "$ROOT/entry.sh" ] || ROOT=/net/mmx/mnt/app/eso/hmi/engdefs/scripts/wireless-carplay-maps
exec /bin/ksh "$ROOT/entry.sh" status
