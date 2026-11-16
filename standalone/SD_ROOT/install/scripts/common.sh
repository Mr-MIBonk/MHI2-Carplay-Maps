#!/bin/ksh
# Local prototype. No dependency on MQB, no SWDL history cleanup, no reboot.
entry_fail(){ echo "CPMAPVC_LOCAL_FAIL=$1";exit 1; }
entry_paths(){
 if [ -n "${CPMAP_TEST_ROOT:-}" ] && [ -s "$CPMAP_TEST_ROOT/ifs/lsd.jxe" ];then MMX="$CPMAP_TEST_ROOT"
 elif [ -s /ifs/lsd.jxe ];then MMX=""
 elif [ -s /net/mmx/ifs/lsd.jxe ];then MMX=/net/mmx
 else entry_fail MMX_NOT_FOUND;fi
 APP="$MMX/mnt/app"
 ENTRY="$APP/eso/hmi/engdefs/scripts/wireless-carplay-maps"
 ESD="$APP/eso/hmi/engdefs/WirelessCarPlayMaps.esd"
 RUNTIME="$APP/carplaymaps"
 OWNER='omonob:WirelessCarPlayMaps:LOCAL-ONLY:1'
 SHELL_BIN=/bin/ksh
 [ -n "${CPMAP_TEST_ROOT:-}" ] && SHELL_BIN=/bin/sh
}
detect_mu(){
 line=`ls -ln "$MMX/ifs/lsd.jxe"` || entry_fail JXE_STAT_FAILED
 set -- $line
 case "$5" in
  55915411) MU=MU1102;;55840541) MU=MU1367;;55840933) MU=MU1440;;
  52751492) MU=MU1433;;55712574) MU=MU1447;;
  *) entry_fail UNSUPPORTED_FIRMWARE;;
 esac
}
find_media(){
 MEDIA="";found=0
 if [ -n "${1:-}" ];then search="$1"
 elif [ -n "${CPMAP_TEST_MEDIA:-}" ] && [ -n "${CPMAP_TEST_ROOT:-}" ];then search="$CPMAP_TEST_MEDIA"
 elif [ "$MMX" = /net/mmx ];then search='/net/mmx/fs/sda0 /net/mmx/fs/sdb0 /net/mmx/media/mp000 /net/mmx/media/mp001 /net/mmx/media/mp002'
 else search='/fs/sda0 /fs/sdb0 /media/mp000 /media/mp001 /media/mp002';fi
 # Known mount paths contain no spaces. An explicitly supplied SWDL path is
 # treated as one path (including spaces), not word-split into alternatives.
 if [ -n "${1:-}" ] || [ -n "${CPMAP_TEST_ROOT:-}" ];then
  [ -s "$search/LOCAL_ONLY" ] && [ -s "$search/carplay/payload/common/install-carplay-maps.sh" ] || entry_fail MEDIA_NOT_FOUND
  MEDIA="$search"
 else
  for candidate in $search;do
   if [ -s "$candidate/LOCAL_ONLY" ] && [ -s "$candidate/carplay/payload/common/install-carplay-maps.sh" ];then
    candidate=`cd "$candidate" 2>/dev/null && pwd -P` || entry_fail MEDIA_PATH_FAILED
    if [ "$MEDIA" != "$candidate" ];then MEDIA="$candidate";found=`expr "$found" + 1`;fi
   fi
  done
  [ "$found" -eq 1 ] || entry_fail MEDIA_MISSING_OR_AMBIGUOUS
 fi
 case "$MEDIA" in /*) ;;*) entry_fail ABSOLUTE_MEDIA_PATH_REQUIRED;;esac
 case "$MEDIA" in */../*|*/..|*/./*) entry_fail INVALID_MEDIA_PATH;;esac
 [ ! -L "$MEDIA" ] || entry_fail SYMLINK_MEDIA_REFUSED
 CHECK="$MEDIA/install/file-check"
 [ -n "${CPMAP_TEST_ROOT:-}" ] && [ -n "${CPMAP_TEST_FILE_CHECK:-}" ] && CHECK="$CPMAP_TEST_FILE_CHECK"
 run_checker --self-test || entry_fail CHECKER_UNAVAILABLE
 run_checker --manifest "$MEDIA" "$MEDIA/CONTENTS.sha256" || entry_fail MEDIA_INTEGRITY_FAILED
}
require_phone_off(){ [ ! -e "$MMX/dev/ipod0" ] && [ ! -e "$MMX/dev/shmem/wcp_cp_session" ] || entry_fail DISCONNECT_CARPLAY_FIRST; }
require_public_runtime(){
 [ ! -s "$RUNTIME/cpmap-auth" ] || entry_fail ENCRYPTED_VERSION_REQUIRES_OWN_MIGRATION
 if [ -s "$APP/eso/hmi/lsd/lsd.sh" ] && grep -q 'VCMenu-V73.jar' "$APP/eso/hmi/lsd/lsd.sh";then
  entry_fail ENCRYPTED_VERSION_REQUIRES_OWN_MIGRATION
 fi
}
owned_entry(){ [ -s "$ENTRY/OWNER" ] && [ "`cat "$ENTRY/OWNER"`" = "$OWNER" ]; }
run_checker(){
 tool="${CHECK:-$ENTRY/file-check}"
 [ -n "${CPMAP_TEST_ROOT:-}" ] && [ -n "${CPMAP_TEST_FILE_CHECK:-}" ] && tool="$CPMAP_TEST_FILE_CHECK"
 if [ "$MMX" = /net/mmx ] && [ -z "${CPMAP_TEST_ROOT:-}" ];then
  # The ARM helper and statvfs probe belong to MMX. Running a binary via
  # its Qnet pathname does not select its execution processor. Normalize
  # only this known node prefix, preserving quoting/order for spaces.
  case "$tool" in /net/mmx/*) tool=${tool#/net/mmx};;esac
  for checker_arg in "$@";do
   shift
   case "$checker_arg" in /net/mmx/*) checker_arg=${checker_arg#/net/mmx};;esac
   set -- "$@" "$checker_arg"
  done
  on -f mmx "$tool" "$@"
 else
  "$tool" "$@"
 fi
}
same_file(){
 left=`run_checker --hash "$1"` || return 1
 right=`run_checker --hash "$2"` || return 1
 [ "$left" = "$right" ]
}
restore_app_mount(){
 rc=$?
 if [ "${RESTORE_APP_RO:-no}" = yes ];then
  sync
  if [ "$MMX" = /net/mmx ];then on -f mmx mount -ur /mnt/app || rc=1
  else mount -ur /mnt/app || rc=1;fi
 fi
 trap - 0
 exit "$rc"
}
ensure_app_writable(){
 [ -z "${CPMAP_TEST_ROOT:-}" ] || return 0
 mode=`run_checker --filesystem "$APP"` || entry_fail APP_MOUNT_STATE_UNAVAILABLE
 case "$mode" in
  read-write) return 0;;
  read-only)
   if [ "$MMX" = /net/mmx ];then on -f mmx mount -uw /mnt/app || entry_fail APP_REMOUNT_FAILED
   else mount -uw /mnt/app || entry_fail APP_REMOUNT_FAILED;fi
   RESTORE_APP_RO=yes;trap restore_app_mount 0
   mode=`run_checker --filesystem "$APP"` || entry_fail APP_MOUNT_RECHECK_FAILED
   [ "$mode" = read-write ] || entry_fail APP_STILL_READ_ONLY;;
  *) entry_fail APP_UNKNOWN_MOUNT_STATE;;
 esac
}

# Execute native helpers and the whole feature transaction on MMX, not RCC.
run_feature_transaction(){
 script=$1;payload=$2;mu=$3;cluster=$4;variant=$5;action=$6
 if [ "$MMX" = /net/mmx ] && [ -z "${CPMAP_TEST_ROOT:-}" ];then
  case "$script" in /net/mmx/*) script=${script#/net/mmx};;esac
  case "$payload" in /net/mmx/*) payload=${payload#/net/mmx};;esac
  on -f mmx /bin/sh "$script" "$payload" "$mu" "$cluster" "$variant" "${ENTRY#/net/mmx}" "$action"
 else
  /bin/sh "$script" "$payload" "$mu" "$cluster" "$variant" "$ENTRY" "$action"
 fi
}
