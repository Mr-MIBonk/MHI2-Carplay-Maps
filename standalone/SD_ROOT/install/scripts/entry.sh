#!/bin/ksh
# Only files in wireless-carplay-maps and our unique GEM ESD belong to this manager.
case "$0" in */*) SELF_DIR=${0%/*};;*) exit 2;;esac
. "$SELF_DIR/common.sh" || exit 2
entry_paths
action="${1:-status}"
case "$action" in
 install-entry|update-entry)
  detect_mu;require_phone_off;find_media "${2:-}"
  parent="$APP/eso/hmi/engdefs/scripts"
  [ -d "$parent" ] || entry_fail GEM_SCRIPT_DIRECTORY_MISSING
  if [ -e "$ENTRY" ] && ! owned_entry;then entry_fail FOREIGN_ENTRY_DIRECTORY;fi
  if [ -e "$ESD" ] && ! owned_entry;then
   # SWDL may already have copied this exact component before FinalScript.
   same_file "$ESD" "$MEDIA/install/esd/WirelessCarPlayMaps.esd" || entry_fail FOREIGN_ESD_FILE
  fi
  [ ! -e "$ENTRY/install.lock" ] || entry_fail FEATURE_TRANSACTION_RUNNING
  ensure_app_writable
  next="$parent/.wireless-carplay-maps-new-$$";old="$parent/.wireless-carplay-maps-old-$$"
  [ ! -e "$next" ] && [ ! -e "$old" ] || entry_fail TRANSACTION_PATH_EXISTS
  mkdir "$next" || entry_fail APP_NOT_WRITABLE
  for name in common.sh entry.sh install-aid10.sh install-aid12.sh update-feature.sh update-entry.sh status.sh restore-feature.sh uninstall.sh;do
   cp "$MEDIA/install/scripts/$name" "$next/$name" || entry_fail ENTRY_STAGE_FAILED
   same_file "$MEDIA/install/scripts/$name" "$next/$name" || entry_fail ENTRY_STAGE_COMPARE_FAILED
   chmod 755 "$next/$name" || entry_fail ENTRY_MODE_FAILED
  done
  cp "$MEDIA/carplay/payload/common/feature-transaction.sh" "$next/feature-transaction.sh" || entry_fail RESTORE_STAGE_FAILED
  chmod 755 "$next/feature-transaction.sh"
  cp "$MEDIA/install/file-check" "$next/file-check" || entry_fail CHECKER_STAGE_FAILED
  cp "$MEDIA/install/esd/WirelessCarPlayMaps.esd" "$next/entry.esd" || entry_fail ESD_STAGE_FAILED
  echo "$OWNER" > "$next/OWNER" || entry_fail OWNER_STAGE_FAILED
  chmod 644 "$next/OWNER" "$next/entry.esd"
  chmod 755 "$next/file-check"
  if owned_entry;then
   for saved in feature-backup installed.tsv INSTALLED_PROFILE;do
    [ ! -e "$ENTRY/$saved" ] || cp -pR "$ENTRY/$saved" "$next/$saved" || entry_fail JOURNAL_TRANSFER_FAILED
   done
  fi
  if owned_entry;then cp "$ESD" "$next/esd.previous" || entry_fail ESD_BACKUP_FAILED;mv "$ENTRY" "$old" || entry_fail ENTRY_BACKUP_FAILED;fi
  mv "$next" "$ENTRY" || { [ -d "$old" ] && mv "$old" "$ENTRY";entry_fail ENTRY_COMMIT_FAILED; }
  if ! cp "$ENTRY/entry.esd" "$ESD.cpmapvc-new" || ! mv "$ESD.cpmapvc-new" "$ESD" || ! same_file "$ENTRY/entry.esd" "$ESD";then
   mv "$ENTRY" "$next"
   if [ -d "$old" ];then mv "$old" "$ENTRY";cp "$ENTRY/entry.esd" "$ESD";else rm -f "$ESD";fi
   entry_fail ESD_COMMIT_FAILED_ROLLED_BACK
  fi
  # Each target is a checked, fixed child of our own entry namespace.
  [ ! -d "$old" ] || rm -rf "$old"
  echo 'CPMAPVC_LOCAL_ENTRY_OK no_MQB_changes=1 no_reboot=1';;
 install-790|install-791|update-feature)
  require_phone_off;detect_mu;require_public_runtime;find_media "${2:-}"
  owned_entry || entry_fail INSTALL_MANAGEMENT_ENTRY_FIRST
  if [ "$action" = update-feature ];then
   [ -s "$ENTRY/INSTALLED_PROFILE" ] || entry_fail NO_INSTALLED_PROFILE
   selected=`cat "$ENTRY/INSTALLED_PROFILE"`
   case "$selected" in "$MU/790/17PM")cluster=790;variant=17PM;;"$MU/790/16e")cluster=790;variant=17PM;echo WMAPS_LEGACY_PHONE_PROFILE_UPGRADED_TO_17PM;;"$MU/791/791")cluster=791;variant=791;;"$MU/790/wired")cluster=790;variant=wired;;"$MU/791/wired")cluster=791;variant=wired;;*)entry_fail SAME_UNIT_PROFILE_MISMATCH;;esac
  else
   case "$action" in install-791)cluster=791;variant=791;;*)cluster=790;variant=17PM;;esac
   case "$MU" in MU1102|MU1367|MU1440);;*)variant=wired;;esac
  fi
  # A saved wired-only predecessor becomes the exact new wireless profile.
  case "$MU:$cluster:$variant" in
   MU1367:790:wired|MU1440:790:wired)variant=17PM;echo WMAPS_WIRED_PROFILE_UPGRADED_TO_WIRELESS;;
   MU1367:791:wired)variant=791;echo WMAPS_WIRED_PROFILE_UPGRADED_TO_WIRELESS;;
  esac
  case "$MU:$cluster" in MU1433:791|MU1447:791|MU1440:791)entry_fail SKODA_SEAT_AID12_UNSUPPORTED;;esac
  ensure_app_writable
  run_feature_transaction "$MEDIA/carplay/payload/common/feature-transaction.sh" "$MEDIA/carplay/payload" "$MU" "$cluster" "$variant" install || entry_fail FEATURE_INSTALL_FAILED
  echo WMAPS_STANDALONE_FEATURE_OK;;
 status)
  detect_mu;echo "WMAPS_MU=$MU"
  if owned_entry;then echo ENTRY=installed;else echo ENTRY=absent;fi
  if [ -s "$ENTRY/INSTALLED_PROFILE" ];then echo "PROFILE=`cat "$ENTRY/INSTALLED_PROFILE"`";else echo FEATURE=absent;fi
  echo "Wireless engine: exact MU1102, MU1367 and MU1440 profiles. New firmware ports require physical acceptance.";;
 restore-feature|uninstall)
  require_phone_off;detect_mu;require_public_runtime
  owned_entry || entry_fail OWNED_ENTRY_REQUIRED
  CHECK="$ENTRY/file-check";ensure_app_writable
  if [ -s "$ENTRY/INSTALLED_PROFILE" ];then
   selected=`cat "$ENTRY/INSTALLED_PROFILE"`
   case "$selected" in "$MU/790/"*)cluster=790;;"$MU/791/"*)cluster=791;;*)entry_fail SAME_UNIT_PROFILE_MISMATCH;;esac
   run_feature_transaction "$ENTRY/feature-transaction.sh" - "$MU" "$cluster" - restore || entry_fail FEATURE_RESTORE_FAILED
  fi
  if [ "$action" = uninstall ];then
   same_file "$ESD" "$ENTRY/entry.esd" || entry_fail ESD_CHANGED_PRESERVED
   rm -f "$ESD" || entry_fail ESD_REMOVE_FAILED
   rm -rf "$ENTRY" || entry_fail ENTRY_REMOVE_FAILED
  fi
  echo WMAPS_STANDALONE_RESTORE_OK;;
 *) entry_fail UNKNOWN_ACTION;;
esac
sync
