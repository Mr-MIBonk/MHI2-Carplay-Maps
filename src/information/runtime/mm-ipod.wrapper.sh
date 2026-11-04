#!/bin/ksh
# CPMAP_PUBLIC_V21_IPOD_WRAPPER -- original bytes kept in stock_first.
ROOT=/mnt/app/carplaymaps
INFO=$ROOT/information
REAL=$ROOT/stock_first/mm-ipod.real
RUNTIME=/ramdisk/cpmap-info/runtime
CFG=/etc/mm/iap2.cfg
TMP=/ramdisk/cpmap-info/iap2.cfg
[ -x "$REAL" ] || exit 127
[ -f "$INFO/ENABLED" ] || exec "$REAL" "$@"
mkdir -p "$RUNTIME" || exec "$REAL" "$@"
rm -f "$TMP.new"
if [ -s "$CFG" ]; then
    if grep -q '5204' "$CFG"; then
        cp "$CFG" "$TMP.new"
    elif grep -q '^recvmsgs=' "$CFG"; then
        sed '/^recvmsgs=/s/$/,0x5204/' "$CFG" > "$TMP.new"
    else
        cp "$CFG" "$TMP.new"
        printf '\n[ident]\nsendmsgs=0x5200,0x5203\nrecvmsgs=0x5201,0x5202,0x5204\n' >> "$TMP.new"
    fi
fi
if [ -s "$TMP.new" ] && grep -q '5204' "$TMP.new"; then
    mv "$TMP.new" "$TMP" || exec "$REAL" "$@"
else
    rm -f "$TMP.new" "$TMP"
fi
touch "$RUNTIME/v49-rgd-start" "$RUNTIME/v49-rgd-send-enable" "$RUNTIME/v532-rgd-clear-request"
if [ -x "$INFO/cpvc-rgd-bootstrap.sh" ]; then
    "$INFO/cpvc-rgd-bootstrap.sh" </dev/null >/dev/null 2>&1 &
fi
if [ -r "$INFO/libcpvc-rgd.so" ]; then
    LD_PRELOAD="${LD_PRELOAD:+$LD_PRELOAD:}$INFO/libcpvc-rgd.so"
    export LD_PRELOAD
fi
# Check the argument before shifting. Unknown driver configurations retain
# every original argument, including -d and its operand.
if [ -s "$TMP" ] && [ "${1:-}" = '-d' ]; then
    case "${2:-}" in
        *config=/etc/mm/iap2.cfg*)
            DRIVER=$(printf '%s\n' "$2" | sed 's#config=/etc/mm/iap2.cfg#config=/ramdisk/cpmap-info/iap2.cfg#')
            shift 2
            exec "$REAL" -d "$DRIVER" "$@"
            ;;
    esac
fi
exec "$REAL" "$@"
