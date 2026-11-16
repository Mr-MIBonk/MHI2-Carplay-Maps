#!/bin/ksh
# Local experiment bootstrap. Failure must propagate; no developer-mode writes.
VOLUME="${2:-}"
[ -n "$VOLUME" ] || { echo 'CPMAPVC_LOCAL_FAIL=SWDL_MEDIA_ARGUMENT_REQUIRED';exit 1; }
/bin/ksh "$VOLUME/install/scripts/entry.sh" install-entry "$VOLUME" || exit 1
touch /tmp/SWDLScript.Result || exit 1
echo 'LOCAL_ONLY_ENTRY_READY: install Wireless CarPlay Maps from its GEM entry; no reboot issued.'
exit 0
