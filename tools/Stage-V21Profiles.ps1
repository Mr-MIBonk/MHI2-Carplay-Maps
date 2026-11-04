param(
 [Parameter(Mandatory=$true)][string]$MenuBuild,
 [Parameter(Mandatory=$true)][string]$InformationBuild,
 [Parameter(Mandatory=$true)][string]$InformationNative,
 [Parameter(Mandatory=$true)][string]$G11Native
)
$ErrorActionPreference='Stop'
$repo=Split-Path $PSScriptRoot -Parent
$profiles=@(@('MU1102','791'),@('MU1102','790'),@('MU1367','791'),@('MU1367','790'),@('MU1440','790'),@('MU1433','790'),@('MU1447','790'))
foreach($p in $profiles){
 $mu=$p[0];$cluster=$p[1]
 $brand=switch($mu){MU1440{'MU1440_SKODA'} MU1433{'MU1433_SKODA'} MU1447{'MU1447_SEAT'} default{$mu}}
 $clusterDir=if($cluster -eq '790'){'790_AID10.5'}else{'791_AID12.3'}
 $pkg=Join-Path $repo ($brand+'/'+$clusterDir)
 $common=Join-Path $pkg 'payload/common';$specific=Join-Path $pkg ('payload/profiles/'+$mu+'/'+$cluster)
 $info=Join-Path $common 'information'
 $null=New-Item -ItemType Directory -Path $info -Force
 foreach($name in @('install-carplay-maps.sh','restore-carplay-maps.sh','dio_manager.wrapper.sh')){
  Copy-Item -LiteralPath (Join-Path $repo ('src/toolbox/'+$name)) -Destination (Join-Path $common $name)
 }
 Copy-Item -LiteralPath (Join-Path $common 'install-carplay-maps.sh') -Destination (Join-Path $pkg 'install.sh')
 Copy-Item -LiteralPath (Join-Path $common 'restore-carplay-maps.sh') -Destination (Join-Path $pkg 'restore-carplay-maps.sh')
 Copy-Item -LiteralPath (Join-Path $InformationNative 'cpmap-nav-check') -Destination (Join-Path $common 'cpmap-nav-check')
 foreach($name in @('VCMenu.jar','VCMirrorRate.jar','CarPlayClusterControls.jar')){
  Copy-Item -LiteralPath (Join-Path $MenuBuild ($mu+'/'+$name)) -Destination (Join-Path $specific $name)
 }
 Copy-Item -LiteralPath (Join-Path $specific 'CarPlayClusterControls.jar') -Destination (Join-Path $common 'CarPlayClusterControls.jar')
 Copy-Item -LiteralPath (Join-Path $InformationBuild ($mu+'/VCInformation.jar')) -Destination (Join-Path $specific 'VCInformation.jar')
 foreach($name in @('mm-ipod.wrapper.sh','cpvc-rgd-bootstrap.sh','artwork-launcher.sh')){
  Copy-Item -LiteralPath (Join-Path $repo ('src/information/runtime/'+$name)) -Destination (Join-Path $info $name)
 }
 foreach($name in @('libcpvc-rgd.so','cpvc-rgd-sync','carplay_artwork_helper')){
  Copy-Item -LiteralPath (Join-Path $InformationNative $name) -Destination (Join-Path $info $name)
 }
 if($mu -in @('MU1433','MU1447')){
  Copy-Item -LiteralPath (Join-Path $G11Native ($mu+'/790/core.so')) -Destination (Join-Path $specific 'core.so')
 }
}
'V21_SEVEN_PROFILE_CANDIDATES_STAGED'
