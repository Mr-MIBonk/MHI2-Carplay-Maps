#!/bin/sh
# Read-only classification + own RAM staging. Never starts/stops any service.
set -e
P=$1;MU=$2;COMMON=$3;ENTRY=$4;S=$5;CHECK=$6;NAV=$7
ROOT="$P/mnt/app/carplaymaps";FIRST="$ENTRY/feature-backup"
fail(){ echo "WMAPS_LEGACY_FAIL=$1";exit 1; }
IPOD="$P/mnt/app/armle/usr/sbin/mm-ipod"
INFO_JAR="$P/mnt/app/eso/hmi/lsd/jars/VCInformation.jar"
if "$NAV" --elf-arm "$IPOD";then
 echo WMAPS_MM_IPOD_NATIVE_PRESERVED
else
 test -f "$IPOD" && test ! -L "$IPOD" || fail MM_IPOD_MISSING_OR_SYMLINK
 digest=`"$CHECK" --hash "$IPOD"`
 awk -F '\t' -v hash="$digest" '$1==hash{found=1}END{exit !found}' "$COMMON/known-public-ipod-wrappers.tsv" || fail UNKNOWN_MM_IPOD_WRAPPER_PRESERVED
 chosen="";proof=""
 for file in "$ROOT/stock_first/mm-ipod.real" "$FIRST/runtime/stock_first/mm-ipod.real";do
  if test -f "$file" && test ! -L "$file" && "$NAV" --elf-arm "$file";then
   next=`"$CHECK" --hash "$file"`
   test -z "$proof" || test "$proof" = "$next" || fail CONFLICTING_NATIVE_IPOD_BACKUPS
   chosen=$file;proof=$next
  fi
 done
 test -n "$chosen" || fail ORIGINAL_MM_IPOD_BACKUP_MISSING
 cp -p "$chosen" "$S/generated/mm-ipod.stock"
 test "`"$CHECK" --hash "$S/generated/mm-ipod.stock"`" = "$proof" || fail IPOD_BACKUP_COMPARE
 touch "$S/generated/restore-native-ipod"
 echo WMAPS_PUBLIC_INFORMATION_IPOD_MIGRATION_PREPARED
fi
if test -e "$INFO_JAR";then
 test -f "$INFO_JAR" && test ! -L "$INFO_JAR" || fail INFORMATION_JAR_NOT_REGULAR
 digest=`"$CHECK" --hash "$INFO_JAR"`
 awk -F '\t' -v hash="$digest" -v mu="$MU" '$1==mu&&$2==hash{found=1}END{exit !found}' "$COMMON/known-public-information.tsv" || fail UNKNOWN_INFORMATION_JAR_PRESERVED
 touch "$S/generated/remove-public-information-jar"
 echo WMAPS_KNOWN_PUBLIC_INFORMATION_JAR_RETIREMENT_PREPARED
fi
OLD="$P/mnt/app/eso/hmi/engdefs/scripts/cpmapvc-local"
ESD="$P/mnt/app/eso/hmi/engdefs/CarPlayMapsVCStandalone.esd"
if test -e "$OLD" || test -e "$ESD";then
 test -d "$OLD" && test ! -L "$OLD" && test ! -L "$ESD" || fail FOREIGN_LEGACY_MANAGEMENT_PRESERVED
 test -f "$OLD/OWNER" && test "`cat "$OLD/OWNER"`" = 'omonob:CarPlayMapsVC:LOCAL-ONLY:1' || fail FOREIGN_LEGACY_MANAGEMENT_PRESERVED
 for name in entry.sh common.sh;do
  test -f "$OLD/$name" && test ! -L "$OLD/$name" || fail LEGACY_MANAGEMENT_FILE_MISSING
  digest=`"$CHECK" --hash "$OLD/$name"`
  awk -F '\t' -v name="$name" -v hash="$digest" '$1==name&&$2==hash{found=1}END{exit !found}' "$COMMON/known-legacy-management.tsv" || fail CUSTOM_LEGACY_MANAGEMENT_PRESERVED
 done
 test -f "$ESD" && test ! -L "$ESD" || fail LEGACY_ESD_MISSING
 digest=`"$CHECK" --hash "$ESD"`
 awk -F '\t' -v hash="$digest" '$1=="entry.esd"&&$2==hash{found=1}END{exit !found}' "$COMMON/known-legacy-management.tsv" || fail CUSTOM_LEGACY_ESD_PRESERVED
 touch "$S/generated/retire-legacy-management"
 echo WMAPS_OLD_PROJECT_ENTRY_RETIREMENT_PREPARED
fi
