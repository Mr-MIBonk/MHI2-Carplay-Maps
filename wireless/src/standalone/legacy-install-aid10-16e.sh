#!/bin/ksh
# Compatibility filename required by the R1 management updater's fixed list.
# There is no 16e declaration or selectable phone profile in this package.
ROOT=/mnt/app/eso/hmi/engdefs/scripts/wireless-carplay-maps
[ -f "$ROOT/entry.sh" ] || ROOT=/net/mmx/mnt/app/eso/hmi/engdefs/scripts/wireless-carplay-maps
echo 'Legacy entry redirected to the single iPhone 17 Pro Max declaration.'
exec /bin/ksh "$ROOT/entry.sh" install-790
