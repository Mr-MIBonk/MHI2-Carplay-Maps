#!/bin/sh
# Wireless CarPlay Maps, omonob. Source date: 2026-08-29.
# Read-only transformation; output is a RAM staging file. No service control.
set -e
kind=$1;input=$2;output=$3
test "$input" != "$output"
case "$kind" in
lsd)
 awk '
 BEGIN {prefix="$BASE_DIR/lsd/jars/";extra=prefix "CarPlayWireless-CP-55915411.jar:" prefix "CarPlayWireless-SI-55915411.jar:" prefix "CarPlayWireless-Touch-55915411.jar:" prefix "WirelessMapsBootstrap.jar"}
 {
  lines[NR]=$0
  if($0=="# OMONOB_WMAPS_WIRELESS_BEGIN"){if(block||beg++)exit 9;block=1;remove[NR]=1;next}
  if(block){remove[NR]=1;if($0=="# OMONOB_WMAPS_WIRELESS_END"){block=0;ends++}else if($0!="[ -x /mnt/app/carplaymaps/start-wireless-maps.sh ] && /mnt/app/carplaymaps/start-wireless-maps.sh </dev/null >/dev/null 2>&1 &")exit 9;next}
  if($0 ~ /^[ \t]*#/)next
  if($0 ~ /-Xbootclasspath\/p:/){
   if($0 !~ /^[ \t]*BOOTCLASSPATH="\$BOOTCLASSPATH -Xbootclasspath\/p:[^" \t]+"[ \t]*$/){bad=1;next}
   remove[NR]=1;if(!insert)insert=NR
   match($0,/-Xbootclasspath\/p:[^" \t]+/);value=substr($0,RSTART+18,RLENGTH-18);n=split(value,items,":")
   for(i=1;i<=n;i++){
    if(items[i] ~ /\/(CarPlayWireless-(CP|SI|Touch)-55915411|WirelessMapsBootstrap)\.jar$/)continue
    if(length(items[i])&&!seen[items[i]]++)kept=kept ":" items[i]
   }
  }
  if($0 ~ /^[ \t]*\$J9[ \t]/){j9=NR;launches++}
 }
 END {
  if(bad||block||beg!=ends||launches!=1)exit 9
  if(!insert)insert=j9
  if(insert>j9)exit 9
  for(row=1;row<=NR;row++){
   if(row==insert)print "BOOTCLASSPATH=\"$BOOTCLASSPATH -Xbootclasspath/p:" extra kept "\""
   if(!remove[row])print lines[row]
  }
  print "# OMONOB_WMAPS_WIRELESS_BEGIN"
  print "[ -x /mnt/app/carplaymaps/start-wireless-maps.sh ] && /mnt/app/carplaymaps/start-wireless-maps.sh </dev/null >/dev/null 2>&1 &"
  print "# OMONOB_WMAPS_WIRELESS_END"
 }' "$input" > "$output";;
bundles)
 awk '
 {
  lines[NR]=$0
  if($0=="# OMONOB_WMAPS_BUNDLE_BEGIN"){if(block||beg++)bad=1;block=1;remove[NR]=1;next}
  if(block){
   remove[NR]=1
   if($0=="# OMONOB_WMAPS_BUNDLE_END"){block=0;ends++;next}
   if($0 ~ /^Bundle.Name.[0-9][0-9] = WirelessCarPlayMaps$/){s=$0;sub(/^Bundle.Name./,"",s);sub(/ =.*$/,"",s);old=s+0;ownname++}
   else if($0 ~ /^Bundle.Activator.[0-9][0-9] = org.omonob.cpmap.wireless.WirelessMapsBootstrap$/){s=$0;sub(/^Bundle.Activator./,"",s);sub(/ =.*$/,"",s);act=s+0;ownact++}
   else bad=1
   next
  }
  if($0 ~ /^[ \t]*#/){next}
  if($0 ~ /WirelessMapsBootstrap|WirelessCarPlayMaps/){bad=1;next}
  if($0 ~ /^Bundle\.(Name|Activator)\.[0-9]+[ \t]*=/){s=$0;sub(/^Bundle\.(Name|Activator)\./,"",s);sub(/[ \t]*=.*/,"",s);used[s+0]=1}
  if($0 ~ /^Bundle.Startup.Lateload[ \t]*=/){late=NR;lates++;s=$0;sub(/^[^=]*=[ \t]*/,"",s);n=split(s,ids,",")}
  else if($0 ~ /^Bundle.Startup.[^=]*=/){s=$0;sub(/^[^=]*=[ \t]*/,"",s);gsub(/,/," ",s);m=split(s,a,/[ \t]+/);for(i=1;i<=m;i++)if(a[i] ~ /^[0-9]+$/)used[a[i]+0]=1}
 }
 END {
  if(bad||block||beg!=ends||lates!=1||(beg&&(ownname!=1||ownact!=1||old!=act)))exit 9
  for(i=1;i<=n;i++){gsub(/[ \t]/,"",ids[i]);if(ids[i]!~/^[0-9][0-9]?$/||seen[ids[i]+0]++)exit 9;if(!beg||ids[i]+0!=old){used[ids[i]+0]=1;keep=keep sep (ids[i]+0);sep=","}}
  for(slot=97;slot<=99;slot++)if(!used[slot])break
  if(slot>99)exit 9
  for(row=1;row<=NR;row++){
   if(row==late)print "Bundle.Startup.Lateload = " keep sep slot
   else if(!remove[row])print lines[row]
  }
  print "# OMONOB_WMAPS_BUNDLE_BEGIN"
  print "Bundle.Name." slot " = WirelessCarPlayMaps"
  print "Bundle.Activator." slot " = org.omonob.cpmap.wireless.WirelessMapsBootstrap"
  print "# OMONOB_WMAPS_BUNDLE_END"
 }' "$input" > "$output";;
*)exit 2;;
esac
test -s "$output"
