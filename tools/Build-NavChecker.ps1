[CmdletBinding()]
param([string]$Distribution = 'Ubuntu-20.04')
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$wslRepo = '/mnt/' + $repo.Substring(0,1).ToLowerInvariant() + $repo.Substring(2).Replace('\','/')
# Approved open GCC 4.9 toolchain image. Never use the original SDK/qcc.
$compile = 'mkdir -p /src/src/toolbox/build && arm-unknown-nto-qnx6.5.0eabi-gcc -std=c99 -O2 -Wall -Wextra -Werror -march=armv7-a -mfloat-abi=soft -o /tmp/cpmap-nav-check /src/src/toolbox/cpmap-nav-check.c && arm-unknown-nto-qnx6.5.0eabi-readelf -h -l -d /tmp/cpmap-nav-check && cp /tmp/cpmap-nav-check /src/src/toolbox/build/cpmap-nav-check'
& wsl.exe -d $Distribution -u root -- docker run --rm -v "${wslRepo}:/src" --entrypoint /bin/sh qnx65-armv7:4.9 -c $compile
if ($LASTEXITCODE -ne 0) { throw 'Navigation checker build failed.' }
$binary = Join-Path $repo 'src/toolbox/build/cpmap-nav-check'
foreach ($relative in @('MU1102/791_AID12.3','MU1102/790_AID10.5','MU1367/791_AID12.3','MU1367/790_AID10.5','MU1440_SKODA/790_AID10.5')) {
    $profile = Join-Path $repo $relative
    Copy-Item -LiteralPath $binary -Destination (Join-Path $profile 'payload/common/cpmap-nav-check')
    Copy-Item -LiteralPath (Join-Path $repo 'src/toolbox/install-carplay-maps.sh') -Destination (Join-Path $profile 'install.sh')
    Copy-Item -LiteralPath (Join-Path $repo 'src/toolbox/install-carplay-maps.sh') -Destination (Join-Path $profile 'payload/common/install-carplay-maps.sh')
}
Get-FileHash -LiteralPath $binary -Algorithm SHA256
