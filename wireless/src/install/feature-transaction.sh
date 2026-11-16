#!/bin/sh
# Wireless CarPlay Maps, omonob. Local offline candidate.
# One owned transaction, readback, first-baseline recovery; never changes SSH,
# passwords, keys, FEC, coding, audio assets or an unrelated project namespace.
set -e
PAYLOAD=$1;MU=$2;CLUSTER=$3;VARIANT=$4;ENTRY=$5;ACTION=${6:-install}
fail(){ echo "WMAPS_TRANSACTION_FAIL=$1";exit 1; }
P=""
if test -n "${CPMAP_TEST_ROOT:-}";then
 test -s "$CPMAP_TEST_ROOT/ifs/lsd.jxe" || fail INVALID_FIXTURE
 P=$CPMAP_TEST_ROOT
fi
test "$ENTRY" = "$P/mnt/app/eso/hmi/engdefs/scripts/wireless-carplay-maps" || fail ENTRY_SCOPE
test ! -L "$ENTRY" && test -s "$ENTRY/OWNER" || fail ENTRY_OWNERSHIP
test "`cat "$ENTRY/OWNER"`" = 'omonob:WirelessCarPlayMaps:LOCAL-ONLY:1' || fail ENTRY_OWNERSHIP
ROOT="$P/mnt/app/carplaymaps";T="$ENTRY/feature-backup";S="$P/ramdisk/.wcmaps-install-$$"
FIRST_NEW="$ENTRY/.feature-backup-new";FIRST_OLD="$ENTRY/.feature-backup-old-$$"
CHECK="$ENTRY/file-check"
if test -n "$P";then CHECK=${CPMAP_TEST_FILE_CHECK:-$CHECK};fi
test -x "$CHECK" || fail CHECKER_MISSING
hash(){ "$CHECK" --hash "$1"; }
same(){ test "`hash "$1"`" = "`hash "$2"`"; }
seal_snapshot(){
 seal_dir=$1
 echo 2 > "$seal_dir/FORMAT"
 : > "$seal_dir/INDEX_SHA256"
 for seal_name in FORMAT files.tsv runtime.sha256;do
  if test -f "$seal_dir/$seal_name";then echo "`hash "$seal_dir/$seal_name"`  $seal_name" >> "$seal_dir/INDEX_SHA256";fi
 done
 "$CHECK" --manifest "$seal_dir" "$seal_dir/INDEX_SHA256"
}
line=`ls -ln "$P/ifs/lsd.jxe"`;set -- $line
case "$MU:$5:$CLUSTER" in MU1102:55915411:790|MU1102:55915411:791|MU1367:55840541:790|MU1367:55840541:791|MU1440:55840933:790|MU1433:52751492:790|MU1447:55712574:790);;*)fail SAME_UNIT_PROFILE;;esac
test ! -e "$P/dev/ipod0" && test ! -e "$P/dev/shmem/wcp_cp_session" || fail DISCONNECT_PHONE_FIRST
case "$ACTION" in install|restore);;*)fail UNKNOWN_ACTION;;esac
for required in awk cat chmod cp expr grep ls mkdir mv rm rmdir touch;do
 command -v "$required" >/dev/null 2>&1 || fail "REQUIRED_TOOL_MISSING_$required"
done
for path in "$ROOT" "$S" "$T" "$ENTRY/install.lock";do test ! -L "$path" || fail SYMLINK_TARGET;done
test ! -e "$S" || fail STAGE_ALREADY_EXISTS
for recovery in "$ENTRY"/.feature-backup-*;do
 test ! -e "$recovery" && test ! -L "$recovery" || fail FIRST_BACKUP_RECOVERY_REVIEW_REQUIRED
done
mkdir "$ENTRY/install.lock" || fail INSTALL_ALREADY_RUNNING
WRITING=no;STAGED=no;FIRST_SWAPPED=no;FIRST_MOVED=no;FIRST_CANDIDATE_OWNED=no;READ_ONLY=""
prepare_mounts(){
 test -z "$P" || return 0
 mounts=/mnt/app
 if test "$MU" = MU1102;then mounts="$mounts /mnt/ota /mnt/system /mnt/persist";fi
 for path in $mounts;do
  mode=`"$CHECK" --filesystem "$path"` || return 1
  case "$mode" in read-write);;read-only)mount -uw "$path" || return 1;READ_ONLY="$path $READ_ONLY";;*)return 1;;esac
 done
}
TAB='	'
allow(){
 case "$1" in
 /mnt/app/eso/bin/apps/dio_manager|/mnt/app/eso/bin/apps/connectionmanager|/mnt/app/eso/hmi/lsd/lsd.sh|/mnt/app/eso/hmi/lsd/config/bundles.properties);;
 /mnt/app/eso/hmi/lsd/jars/CarPlayClusterControls.jar|/mnt/app/eso/hmi/lsd/jars/CarPlayNavigationPatch.jar|/mnt/app/eso/hmi/lsd/jars/CarPlayMapsG11Navigation.jar|/mnt/app/eso/hmi/lsd/jars/VCMenu.jar|/mnt/app/eso/hmi/lsd/jars/VCMirrorRate.jar);;
 /mnt/app/armle/usr/sbin/mm-ipod|/mnt/app/eso/hmi/lsd/jars/VCInformation.jar);;
 /mnt/app/eso/hmi/engdefs/scripts/cpmapvc-local/entry.sh|/mnt/app/eso/hmi/engdefs/CarPlayMapsVCStandalone.esd);;
 /mnt/app/eso/hmi/lsd/jars/CarPlayWireless-CP-55915411.jar|/mnt/app/eso/hmi/lsd/jars/CarPlayWireless-SI-55915411.jar|/mnt/app/eso/hmi/lsd/jars/CarPlayWireless-Touch-55915411.jar|/mnt/app/eso/hmi/lsd/jars/WirelessMapsBootstrap.jar);;
 /mnt/ota/btstack_wrap.sh|/mnt/ota/libbtwcp.so|/mnt/ota/wcp_autostart.sh|/mnt/ota/wcp_chansync.sh|/mnt/ota/wcp_ctl.sh|/mnt/ota/wcp_pf.sh|/mnt/ota/wcp_pfwatch.sh|/mnt/ota/wcp_receiver|/mnt/ota/wcp_video);;
 /mnt/system/etc/eso/production/connectivity.json|/mnt/system/etc/eso/production/smartphone_integrator.json);;
 /mnt/persist/altscreen.conf|/mnt/persist/altscreen.geom|/mnt/persist/v75-wireless-display.conf|/mnt/persist/omonob-phone/cluster-profile|/mnt/persist/btwcp_enable|/mnt/persist/btwcp_present_target|/mnt/persist/wcp_autostart|/mnt/persist/wcp_cp_stage);;
 *)return 1;;esac
}
safe_target(){
 allow "$1" || return 1
 parent="$P$1"
 while test "$parent" != "$P" && test "$parent" != / && test -n "$parent";do
  test ! -L "$parent" || return 1;parent=${parent%/*}
 done
}
validate_snapshot(){
 backup=$1
 test -s "$backup/files.tsv" || return 1
 if test -e "$backup/FORMAT";then
  test "`cat "$backup/FORMAT"`" = 2 && test -s "$backup/INDEX_SHA256" || return 1
  index_format=no;index_files=no;index_runtime=no
  while read -r index_hash index_name index_extra;do
   test -z "$index_extra" || return 1
   case "$index_name" in
    FORMAT)test "$index_format" = no || return 1;index_format=yes;;
    files.tsv)test "$index_files" = no || return 1;index_files=yes;;
    runtime.sha256)test "$index_runtime" = no || return 1;index_runtime=yes;;
    *)return 1;;
   esac
  done < "$backup/INDEX_SHA256"
  test "$index_format:$index_files" = yes:yes || return 1
  if test -d "$backup/runtime";then
   test "$index_runtime" = yes && test -s "$backup/runtime.sha256" || return 1
  else test "$index_runtime" = no && test ! -e "$backup/runtime.sha256" || return 1;fi
  "$CHECK" --manifest "$backup" "$backup/INDEX_SHA256" || return 1
 elif test "$backup" = "$T" && test -s "$ROOT/VERSION" && test "`cat "$ROOT/VERSION"`" = WIRELESS-CARPLAY-MAPS-LOCAL-2;then
  # A completed R2 always seals its first journal. Do not silently downgrade
  # a damaged R2 journal into the legacy R1 compatibility path.
  return 1
 fi
 # Validate the entire journal before restoring any destination.
 : > "$S/validate-seen"
 while IFS="$TAB" read -r id path before;do
  case "$id" in f[0-9][0-9][0-9]);;*)return 1;;esac
  safe_target "$path" || return 1
  grep -Fx "$id" "$S/validate-seen" >/dev/null 2>&1 && return 1
  grep -Fx "$path" "$S/validate-seen" >/dev/null 2>&1 && return 1
  echo "$id" >> "$S/validate-seen";echo "$path" >> "$S/validate-seen"
  if test "$before" != MISSING;then test "`hash "$backup/$id"`" = "$before" || return 1;fi
 done < "$backup/files.tsv"
 test ! -e "$backup/runtime.sha256" || test -d "$backup/runtime" || return 1
 if test -d "$backup/runtime";then
  if test -s "$backup/runtime.sha256";then
   "$CHECK" --tree-check "$backup/runtime" "$backup/runtime.sha256" || return 1
  else
   # R1 did not seal directory backups. Check its current structure, but do not
   # pretend a newly generated hash proves its historical original bytes.
   "$CHECK" --tree-hash "$backup/runtime" "$S/legacy-backup-current.sha256" || return 1
   echo WMAPS_LEGACY_BACKUP_STRUCTURE_CHECKED_NOT_HISTORICALLY_SEALED
  fi
 fi
}
restore_snapshot(){
 backup=$1
 validate_snapshot "$backup" || return 1
 while IFS="$TAB" read -r id path before;do
  rm -f "$P$path.wmaps-new" "$P$path.wmaps-restore" || return 1
  if test "$before" = MISSING;then rm -f "$P$path" || return 1
  else
   cp -p "$backup/$id" "$P$path.wmaps-restore" && mv -f "$P$path.wmaps-restore" "$P$path" || return 1
   test "`hash "$P$path"`" = "$before" || return 1
  fi
 done < "$backup/files.tsv"
 # ROOT is a fixed project child, never a computed mount or workspace root.
 test ! -L "$ROOT" || return 1
 rm -rf "$ROOT" || return 1
 if test -d "$backup/runtime";then cp -pR "$backup/runtime" "$ROOT" || return 1;fi
}
finish(){
 code=$?;trap - 0 1 2 15
 if test "$WRITING" = yes;then
  if restore_snapshot "$S/before";then
   for name in installed.tsv INSTALLED_PROFILE;do
    if test -f "$S/$name.previous";then cp -p "$S/$name.previous" "$ENTRY/$name" || exit 76
    else rm -f "$ENTRY/$name" || exit 76;fi
   done
   echo WMAPS_ROLLED_BACK
  else echo "WMAPS_ROLLBACK_INCOMPLETE=$S";rmdir "$ENTRY/install.lock" 2>/dev/null;exit 76;fi
 fi
 if test "$WRITING" = yes;then
  if test "$FIRST_SWAPPED" = yes;then rm -rf "$T" || exit 76;fi
  if test "$FIRST_MOVED" = yes;then mv "$FIRST_OLD" "$T" || exit 76;fi
 fi
 if test "$FIRST_CANDIDATE_OWNED" = yes && test -e "$FIRST_NEW";then rm -rf "$FIRST_NEW";fi
 if test "$WRITING" = no && test "$FIRST_MOVED" = yes && test -e "$FIRST_OLD";then rm -rf "$FIRST_OLD";fi
 if test "$STAGED" = yes;then rm -rf "$S";fi
 for path in $READ_ONLY;do mount -ur "$path" || code=1;done
 rmdir "$ENTRY/install.lock" || code=1
 exit "$code"
}
trap finish 0
trap 'exit 129' 1
trap 'exit 130' 2
trap 'exit 143' 15
if test "$ACTION" = restore;then
 test -s "$T/files.tsv" && test -s "$ENTRY/installed.tsv" || fail FIRST_BACKUP_MISSING
 while IFS="$TAB" read -r path wanted;do
  safe_target "$path" || fail JOURNAL_PATH
  test ! -e "$P$path.wmaps-new" && test ! -L "$P$path.wmaps-new" && test ! -e "$P$path.wmaps-restore" && test ! -L "$P$path.wmaps-restore" || fail STALE_ACTIVATION_FILE
  if test "$wanted" = MISSING;then test ! -e "$P$path" && test ! -L "$P$path" || fail POST_INSTALL_CHANGED_PRESERVED
  else test "`hash "$P$path"`" = "$wanted" || fail POST_INSTALL_CHANGED_PRESERVED;fi
 done < "$ENTRY/installed.tsv"
 mkdir -m 700 "$S";STAGED=yes;mkdir -m 700 "$S/before"
 validate_snapshot "$T" || fail FIRST_BACKUP_VALIDATION_FAILED
 cp "$T/files.tsv" "$S/before/files.tsv"
 : > "$S/before/current.tsv"
 while IFS="$TAB" read -r id path before;do
  if test -e "$P$path";then cp -p "$P$path" "$S/before/$id";before=`hash "$P$path"`
  else before=MISSING;fi
  echo "$id$TAB$path$TAB$before" >> "$S/before/current.tsv"
 done < "$T/files.tsv"
 mv "$S/before/current.tsv" "$S/before/files.tsv"
 if test -d "$ROOT";then
  "$CHECK" --tree-hash "$ROOT" "$S/before/runtime.sha256"
  cp -pR "$ROOT" "$S/before/runtime"
  "$CHECK" --tree-check "$S/before/runtime" "$S/before/runtime.sha256"
 fi
 seal_snapshot "$S/before"
 for name in installed.tsv INSTALLED_PROFILE;do
  test ! -e "$ENTRY/$name" || cp -p "$ENTRY/$name" "$S/$name.previous"
 done
 prepare_mounts || fail REMOUNT_FAILED
 WRITING=yes
 restore_snapshot "$T" || fail FIRST_BASELINE_RESTORE_FAILED
 WRITING=no
 rm -rf "$T";rm -f "$ENTRY/installed.tsv" "$ENTRY/INSTALLED_PROFILE"
 echo WMAPS_FIRST_BASELINE_RESTORED
 exit 0
fi
case "$MU:$CLUSTER:$VARIANT" in MU1102:790:17PM|MU1102:791:791);;MU1102:*)fail UNKNOWN_WIRELESS_PROFILE;;*:790:wired|*:791:wired);;*)fail UNKNOWN_PROFILE;;esac
if test -d "$ROOT";then
 test -s "$ROOT/VERSION" && test -s "$ROOT/PROFILE" || fail FOREIGN_RUNTIME_PRESERVED
 grep -Fx "`cat "$ROOT/VERSION"`" "$PAYLOAD/common/known-public-versions.txt" >/dev/null 2>&1 || fail UNKNOWN_PUBLIC_OR_PRIVATE_VERSION_PRESERVED
 case "$MU:`cat "$ROOT/PROFILE"`" in MU1102:MU1102/790|MU1102:MU1102/791|MU1367:MU1367/790|MU1367:MU1367/791|MU1440:MU1440/790|MU1433:MU1433/790|MU1447:MU1447/790);;*)fail PREVIOUS_PROFILE_WRONG_FIRMWARE;;esac
fi
test ! -s "$ROOT/cpmap-auth" || fail PRIVATE_VERSION_REQUIRES_OWN_MIGRATION
test ! -e "$FIRST_NEW" && test ! -L "$FIRST_NEW" && test ! -e "$FIRST_OLD" && test ! -L "$FIRST_OLD" || fail BACKUP_STAGE_PRESENT
if test -s "$ENTRY/installed.tsv";then
 while IFS="$TAB" read -r path wanted;do
  safe_target "$path" || fail PRIOR_RECEIPT_SCOPE
  if test "$wanted" = MISSING;then test ! -e "$P$path" && test ! -L "$P$path" || fail PREVIOUS_INSTALL_CHANGED_PRESERVED
  else test "`hash "$P$path"`" = "$wanted" || fail PREVIOUS_INSTALL_CHANGED_PRESERVED;fi
 done < "$ENTRY/installed.tsv"
fi
# Check conservative RAM and partition headroom before stopping project children.
for path in "$P/ramdisk" "$P/mnt/app";do
 free=`"$CHECK" --free-bytes "$path"`;test "$free" -ge 100000000 || fail INSUFFICIENT_HEADROOM
done
if test "$MU" = MU1102;then
 for path in "$P/mnt/ota" "$P/mnt/system" "$P/mnt/persist";do
  test -d "$path" || fail REQUIRED_PARTITION_MISSING
  free=`"$CHECK" --free-bytes "$path"`;test "$free" -ge 10000000 || fail INSUFFICIENT_HEADROOM
 done
fi
mkdir -m 700 "$S";STAGED=yes;mkdir -m 700 "$S/before" "$S/generated"
for name in installed.tsv INSTALLED_PROFILE;do
 test ! -e "$ENTRY/$name" || cp -p "$ENTRY/$name" "$S/$name.previous"
done
cp "$PAYLOAD/common/transaction-core-paths.txt" "$S/paths"
if test "$MU" = MU1102;then cat "$PAYLOAD/wireless/transaction-paths.txt" >> "$S/paths";fi
: > "$S/seen";: > "$S/before/files.tsv";n=0
while read -r path;do
 safe_target "$path" || fail UNSAFE_TARGET
 test ! -e "$P$path.wmaps-new" && test ! -L "$P$path.wmaps-new" && test ! -e "$P$path.wmaps-restore" && test ! -L "$P$path.wmaps-restore" || fail STALE_ACTIVATION_FILE
 grep -Fx "$path" "$S/seen" >/dev/null 2>&1 && fail DUPLICATE_TARGET
 echo "$path" >> "$S/seen";n=`expr "$n" + 1`;id=`awk -v n="$n" 'BEGIN{printf "f%03d",n}'`
 if test -e "$P$path";then
  test -f "$P$path" || fail TARGET_NOT_REGULAR
  cp -p "$P$path" "$S/before/$id";before=`hash "$P$path"`
  test "`hash "$S/before/$id"`" = "$before" || fail BACKUP_COMPARE
 else before=MISSING;fi
 echo "$id$TAB$path$TAB$before" >> "$S/before/files.tsv"
done < "$S/paths"
if test -d "$ROOT";then
 "$CHECK" --tree-hash "$ROOT" "$S/before/runtime.sha256"
 cp -pR "$ROOT" "$S/before/runtime"
 "$CHECK" --tree-check "$S/before/runtime" "$S/before/runtime.sha256"
fi
seal_snapshot "$S/before"
NAV="$PAYLOAD/common/cpmap-nav-check"
test -z "$P" || NAV=${CPMAP_TEST_NAV_CHECK:-$NAV}
sh "$PAYLOAD/common/prepare-legacy.sh" "$P" "$MU" "$PAYLOAD/common" "$ENTRY" "$S" "$CHECK" "$NAV"
if test "$MU" = MU1102;then
 W="$PAYLOAD/wireless";C="$P/mnt/system/etc/eso/production"
 PATCH="$W/config-patch";test -z "$P" || PATCH=${CPMAP_TEST_CONFIG_PATCH:-$PATCH}
 "$PATCH" connectivity "$C/connectivity.json" "$S/generated/connectivity.json"
 "$PATCH" si "$C/smartphone_integrator.json" "$S/generated/smartphone_integrator.json"
 sh "$PAYLOAD/common/merge-wireless.sh" bundles "$P/mnt/app/eso/hmi/lsd/config/bundles.properties" "$S/generated/bundles.properties"
 # Never claim files belonging to an unknown wireless implementation.
 if test ! -s "$ENTRY/installed.tsv";then
  while IFS="$TAB" read -r source target mode;do
   case "$target" in /mnt/ota/*|/mnt/app/eso/hmi/lsd/jars/CarPlayWireless-*.jar|/mnt/app/eso/hmi/lsd/jars/WirelessMapsBootstrap.jar)
    test ! -e "$P$target" || fail EXISTING_WIRELESS_REQUIRES_EXPLICIT_MIGRATION;;esac
  done < "$W/common-files.tsv"
 fi
fi
FIRST_CANDIDATE_OWNED=yes
if test -d "$T";then
 validate_snapshot "$T" || fail FIRST_BACKUP_CORRUPT
 cp -pR "$T" "$FIRST_NEW";chmod 700 "$FIRST_NEW"
 # Expand only newly tracked paths. R1 did not modify these legacy targets;
 # current bytes are preserved, not guessed to be an earlier factory version.
 : > "$S/first-seen";count=0
 while IFS="$TAB" read -r old_id old_path old_proof;do
  case "$old_id" in f[0-9][0-9][0-9]);;*)fail FIRST_BACKUP_ID;;esac
  safe_target "$old_path" || fail FIRST_BACKUP_SCOPE
  grep -Fx "$old_path" "$S/first-seen" >/dev/null 2>&1 && fail FIRST_BACKUP_DUPLICATE
  grep -Fx "$old_id" "$S/first-seen" >/dev/null 2>&1 && fail FIRST_BACKUP_DUPLICATE_ID
  echo "$old_path" >> "$S/first-seen"
  echo "$old_id" >> "$S/first-seen"
  if test "$old_proof" != MISSING;then
   test -f "$FIRST_NEW/$old_id" && test ! -L "$FIRST_NEW/$old_id" || fail FIRST_BACKUP_NOT_REGULAR
   test "`hash "$FIRST_NEW/$old_id"`" = "$old_proof" || fail FIRST_BACKUP_CORRUPT
  fi
  numeric=`awk -v id="$old_id" 'BEGIN{sub(/^f/,"",id);print id+0}'`
  test "$count" -ge "$numeric" || count=$numeric
 done < "$FIRST_NEW/files.tsv"
 while IFS="$TAB" read -r old_id old_path old_proof;do
  if ! grep -Fx "$old_path" "$S/first-seen" >/dev/null 2>&1;then
   count=`expr "$count" + 1`;test "$count" -lt 1000 || fail FIRST_BACKUP_CAPACITY
   next_id=`awk -v n="$count" 'BEGIN{printf "f%03d",n}'`
   if test "$old_proof" != MISSING;then cp -p "$S/before/$old_id" "$FIRST_NEW/$next_id";fi
   echo "$next_id$TAB$old_path$TAB$old_proof" >> "$FIRST_NEW/files.tsv"
  fi
 done < "$S/before/files.tsv"
 if test -d "$FIRST_NEW/runtime";then
  if test -s "$FIRST_NEW/runtime.sha256";then "$CHECK" --tree-check "$FIRST_NEW/runtime" "$FIRST_NEW/runtime.sha256"
  else "$CHECK" --tree-hash "$FIRST_NEW/runtime" "$FIRST_NEW/runtime.sha256";fi
 fi
else cp -pR "$S/before" "$FIRST_NEW";chmod 700 "$FIRST_NEW";fi
seal_snapshot "$FIRST_NEW"
prepare_mounts || fail REMOUNT_FAILED
# Detect another writer after staging, before changing a single target.
while IFS="$TAB" read -r id path before;do
 if test "$before" = MISSING;then test ! -e "$P$path" && test ! -L "$P$path" || fail PREIMAGE_CHANGED
 else test "`hash "$P$path"`" = "$before" || fail PREIMAGE_CHANGED;fi
done < "$S/before/files.tsv"
copy_file(){
 source=$1;target=$2;mode=$3
 case "$target" in /mnt/app/carplaymaps/*)test ! -L "$P$target" || fail SYMLINK_TARGET;;*)safe_target "$target" || fail UNSAFE_TARGET;;esac
 mkdir -p "${P}${target%/*}"
 cp "$source" "$P$target.wmaps-new";chmod "$mode" "$P$target.wmaps-new"
 same "$source" "$P$target.wmaps-new"
 mv -f "$P$target.wmaps-new" "$P$target";same "$source" "$P$target"
}
WRITING=yes
if test -f "$S/generated/restore-native-ipod";then
 copy_file "$S/generated/mm-ipod.stock" /mnt/app/armle/usr/sbin/mm-ipod 755
fi
sh "$PAYLOAD/common/install-carplay-maps.sh" "$PAYLOAD" "$MU" "$CLUSTER"
if test -f "$S/generated/remove-public-information-jar";then rm -f "$P/mnt/app/eso/hmi/lsd/jars/VCInformation.jar";fi
if test -f "$S/generated/retire-legacy-management";then
 copy_file "$PAYLOAD/common/legacy-management-disabled.sh" /mnt/app/eso/hmi/engdefs/scripts/cpmapvc-local/entry.sh 755
 copy_file "$PAYLOAD/common/legacy-management-disabled.esd" /mnt/app/eso/hmi/engdefs/CarPlayMapsVCStandalone.esd 644
fi
if test "$MU" = MU1102;then
 case "$CLUSTER:$VARIANT" in 791:791)PROFILE=AID12.3;;790:17PM)PROFILE=AID10.5-17PM;;esac
 sh "$PAYLOAD/common/merge-wireless.sh" lsd "$P/mnt/app/eso/hmi/lsd/lsd.sh" "$S/generated/lsd.sh"
 cat "$W/common-files.tsv" "$W/profiles/$PROFILE/files.tsv" > "$S/copies.tsv"
 while IFS="$TAB" read -r source target mode;do
  case "$source" in common/*|profiles/*);;*)fail INVALID_SOURCE;;esac
  case "$source" in *..*)fail INVALID_SOURCE;;esac
  case "$mode" in 644|755);;*)fail INVALID_MODE;;esac
  copy_file "$W/$source" "$target" "$mode"
 done < "$S/copies.tsv"
 for pair in 'connectivity.json:/mnt/system/etc/eso/production/connectivity.json:644' 'smartphone_integrator.json:/mnt/system/etc/eso/production/smartphone_integrator.json:644' 'bundles.properties:/mnt/app/eso/hmi/lsd/config/bundles.properties:644' 'lsd.sh:/mnt/app/eso/hmi/lsd/lsd.sh:755';do
  oldIFS=$IFS;IFS=:;set -- $pair;IFS=$oldIFS
  copy_file "$S/generated/$1" "$2" "$3"
 done
 echo "WMAPS_WIRELESS_PROFILE=$PROFILE primary_request=30"
else echo WMAPS_WIRED_ONLY_WIRELESS_PORT_PENDING;fi
: > "$S/installed.tsv"
while read -r path;do
 if test -e "$P$path";then echo "$path$TAB`hash "$P$path"`" >> "$S/installed.tsv"
 else echo "$path${TAB}MISSING" >> "$S/installed.tsv";fi
done < "$S/paths"
cp "$S/installed.tsv" "$ENTRY/installed.tsv.new";mv "$ENTRY/installed.tsv.new" "$ENTRY/installed.tsv"
echo "$MU/$CLUSTER/$VARIANT" > "$ENTRY/INSTALLED_PROFILE"
if test -d "$T";then mv "$T" "$FIRST_OLD";FIRST_MOVED=yes;fi
mv "$FIRST_NEW" "$T";FIRST_SWAPPED=yes
WRITING=no
test ! -e "$FIRST_OLD" || rm -rf "$FIRST_OLD"
echo WMAPS_FEATURE_COMMIT_OK
echo 'Restart the unit manually before connecting a phone. No reboot issued.'
exit 0
