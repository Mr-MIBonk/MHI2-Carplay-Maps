[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$PreviousArchive,
    [Parameter(Mandatory=$true)][string]$BuildRoot
)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.IO.Compression.FileSystem
Add-Type -AssemblyName System.IO.Compression
$repo = Split-Path -Parent $PSScriptRoot
$previous = [IO.Path]::GetFullPath($PreviousArchive)
$build = [IO.Path]::GetFullPath($BuildRoot)
$name = 'VW_SKODA_ALL_CarPlay_Maps_VC_FULL_MQB_Toolbox_V4.2A_v1.2.2.zip'
if (Test-Path -LiteralPath $build) { throw 'BuildRoot must be a new directory.' }
if ((Get-FileHash -LiteralPath $previous -Algorithm SHA256).Hash.ToLowerInvariant() -ne 'f72d14a82518c1a27754c842d362fc29c9df1e8c355ed4b6acd4e8bb73b0dd21') {
    throw 'Expected the unchanged published v1.2.1 MQB archive.'
}
$checker = Join-Path $repo 'src/toolbox/build/cpmap-nav-check'
if (!(Test-Path -LiteralPath $checker)) { throw 'Run tools/Build-NavChecker.ps1 first.' }
$assets = Join-Path $build 'release-assets'
New-Item -ItemType Directory -Path $assets | Out-Null
$output = Join-Path $assets $name
Copy-Item -LiteralPath $previous -Destination $output
$archive = [IO.Compression.ZipFile]::Open($output, [IO.Compression.ZipArchiveMode]::Update)
try {
    $entryName = 'Custom/carplay_maps_vc/payload/common/install-carplay-maps.sh'
    $entry = $archive.GetEntry($entryName)
    if (!$entry) { throw 'Expected MQB installer path is missing.' }
    $entry.Delete()
    [IO.Compression.ZipFileExtensions]::CreateEntryFromFile($archive, (Join-Path $repo 'src\toolbox\install-carplay-maps.sh'), $entryName, [IO.Compression.CompressionLevel]::Optimal) | Out-Null
    $checkerEntry = 'Custom/carplay_maps_vc/payload/common/cpmap-nav-check'
    if ($archive.GetEntry($checkerEntry)) { throw 'Baseline unexpectedly already contains the new checker.' }
    [IO.Compression.ZipFileExtensions]::CreateEntryFromFile($archive, $checker, $checkerEntry, [IO.Compression.CompressionLevel]::Optimal) | Out-Null
} finally { $archive.Dispose() }
$hash = (Get-FileHash -LiteralPath $output -Algorithm SHA256).Hash.ToLowerInvariant()
[IO.File]::WriteAllText((Join-Path $assets 'FULL_TOOLBOX_SHA256.txt'), "$hash  $name`n", [Text.UTF8Encoding]::new($false))
Copy-Item -LiteralPath (Join-Path $repo 'releases\toolbox-plaintext-v1.2.2.md') -Destination (Join-Path $assets 'GITHUB_RELEASE_NOTES.md')
if (@(Get-ChildItem -LiteralPath $assets -File).Count -ne 3) { throw 'MQB release must contain exactly one ZIP, one checksum and one notes file.' }
Write-Host "MQB_ONLY_RELEASE_BUILD_OK $assets"
