#!/bin/ksh
# V53.2.2 REPLAY GUARD TEST bootstrap for the inherited Java RGI watchdog.
# Publish an inactive atomic snapshot before starting the indexed cache, and
# never create a second cache service when the watchdog probes again.

SYNC=/mnt/app/carplaymaps/information/cpvc-rgd-sync
PPS_DIR=/ramdisk/cpmap-info/runtime
PPS=$PPS_DIR/v532-rgd-cache.pps
TMP=$PPS.tmp.$$
RESYNC=$PPS_DIR/v513-rgd-resync-request
LOCK=$PPS_DIR/v532-rgd-sync.lock
HEARTBEAT=$PPS_DIR/v532-rgd-heartbeat
HEARTBEAT_PROBE=$PPS_DIR/v532-rgd-heartbeat.probe
HEARTBEAT_STALE=$PPS_DIR/v532-rgd-heartbeat.stale

# Public V2.1 owns a bounded volatile runtime; no media remount is needed.
mkdir -p "$PPS_DIR" 2>/dev/null
chmod 777 "$PPS_DIR" 2>/dev/null
if [ ! -f "$PPS" ]; then
    rm -f "$TMP" 2>/dev/null
    {
        echo "state::-1"
        echo "man_state::-1"
        echo "road::"
        echo "dest::"
        echo "eta::0"
        echo "rtt::0"
        echo "dist_dest::-1"
        echo "dist_man::-1"
        echo "dist_man_str::"
        echo "dist_man_units::-1"
        echo "man_index::-1"
        echo "man_type::-1"
        echo "turn_to::"
        echo "man_desc::"
        echo "driving_side::-1"
        echo "junction_type::-1"
        echo "exit_angle::1000"
        echo "man_count::-1"
        echo "lane_idx::-1"
        echo "lane_count::0"
        echo "lane_status::"
        echo "lane_dir::"
        echo "lane_angles::"
        echo "lane_active_scr::0"
        echo "seq::0"
    } > "$TMP"
    chmod 666 "$TMP" 2>/dev/null
    mv "$TMP" "$PPS" 2>/dev/null || { rm -f "$TMP" 2>/dev/null; exit 1; }
fi

# Do not use `pidin | grep` here: on QNX the grep command can appear in the
# process list and make the watchdog believe a non-existent service is alive.
if [ -f "$LOCK" ]; then
    SYNC_PID=$(cat "$LOCK" 2>/dev/null)
    case "$SYNC_PID" in
        ''|*[!0-9]*) SYNC_PID="" ;;
        *)
            if kill -0 "$SYNC_PID" 2>/dev/null; then
                # A live, advancing heartbeat proves that the parser loop is
                # healthy.  A route snapshot is allowed to remain unchanged
                # for a long time (straight road, no lane event, or a map app
                # that sends no lane guidance), so a stalled cache sequence is
                # NOT a reason to send another 0x5200 subscription request.
                # Restart only after three probes see the exact same heartbeat
                # (or repeatedly see no heartbeat).  This extra latch avoids
                # killing a healthy loop when two Java probes happen inside
                # the native 5-second heartbeat interval.
                HEARTBEAT_NOW=$(cat "$HEARTBEAT" 2>/dev/null)
                HEARTBEAT_OLD=$(cat "$HEARTBEAT_PROBE" 2>/dev/null)
                [ -n "$HEARTBEAT_NOW" ] || HEARTBEAT_NOW=missing
                if [ -n "$HEARTBEAT_OLD" ] && [ "$HEARTBEAT_NOW" = "$HEARTBEAT_OLD" ]; then
                    if [ -f "$HEARTBEAT_STALE" ]; then
                        slay -f cpvc-rgd-sync 2>/dev/null
                        sleep 1
                        # Never unlink a lock pathname while its old inode may
                        # still be locked by a surviving process.
                        kill -0 "$SYNC_PID" 2>/dev/null && exit 1
                        rm -f "$LOCK" "$HEARTBEAT_PROBE" "$HEARTBEAT_STALE" 2>/dev/null
                    else
                        echo 1 > "$HEARTBEAT_STALE" 2>/dev/null
                        exit 0
                    fi
                else
                    rm -f "$HEARTBEAT_STALE" 2>/dev/null
                    echo "$HEARTBEAT_NOW" > "$HEARTBEAT_PROBE" 2>/dev/null
                    # Parser is alive.  Preserve the current route and do
                    # nothing; the native parser itself detects iap2.log inode
                    # replacement/truncation and requests a true resync.
                    exit 0
                fi
            fi
            ;;
    esac
fi
rm -f "$LOCK" "$HEARTBEAT_PROBE" "$HEARTBEAT_STALE" 2>/dev/null
[ -x "$SYNC" ] || exit 1
# A watchdog recovery starts at the end of the existing iAP2 log to avoid
# replaying a stale route.  Ask the sender to request a fresh phone snapshot.
echo 1 > "$RESYNC" 2>/dev/null
exec "$SYNC"
