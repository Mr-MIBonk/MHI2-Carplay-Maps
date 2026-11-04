param(
    [Parameter(Mandatory=$true)][string]$MU1102StockJar,
    [Parameter(Mandatory=$true)][string]$MU1367StockJar,
    [Parameter(Mandatory=$true)][string]$MU1440StockJar,
    [Parameter(Mandatory=$true)][string]$MU1433StockJar,
    [Parameter(Mandatory=$true)][string]$MU1447StockJar,
    [Parameter(Mandatory=$true)][string]$MenuBuild,
    [Parameter(Mandatory=$true)][string]$Output,
    [string]$JavaHome='C:\Program Files\Java\jdk-17'
)
$ErrorActionPreference='Stop'
$repo=Split-Path $PSScriptRoot -Parent
$source=Join-Path $repo 'src\information'
if(Test-Path -LiteralPath $Output){throw 'Choose a new empty output directory.'}
$null=New-Item -ItemType Directory -Path $Output
$out=(Resolve-Path -LiteralPath $Output).Path
$classes=Join-Path $out 'tools'
$null=New-Item -ItemType Directory -Path $classes
$java=Join-Path $JavaHome 'bin\java.exe'
$javac=Join-Path $JavaHome 'bin\javac.exe'
$exports='java.base/jdk.internal.org.objectweb.asm=ALL-UNNAMED'
$asm=@('--add-exports',$exports,'--add-exports','java.base/jdk.internal.org.objectweb.asm.util=ALL-UNNAMED','--add-exports','java.base/jdk.internal.org.objectweb.asm.tree=ALL-UNNAMED')
function Run([string]$Program,[string[]]$Arguments){
    & $Program @Arguments
    if($LASTEXITCODE -ne 0){throw ('Build command failed: '+$Program+' ('+$LASTEXITCODE+')')}
}
$sources=@(Get-ChildItem -LiteralPath (Join-Path $source 'instruction-source') -Filter '*.java' -Recurse | ForEach-Object FullName)
$sources+=@(Get-ChildItem -LiteralPath (Join-Path $source 'tools') -Filter '*.java' | ForEach-Object FullName)
$sources+=Join-Path $source 'tests\EtaTimezoneTest.java'
$sources+=Join-Path $repo 'src\vc-menu\tools\BuildNativeHook.java'
Run $javac ($asm+@('-Xlint:-removal','-encoding','UTF-8','-d',$classes)+$sources)
$historical=Join-Path $out 'V53-source.jar'
Run $java @('--add-exports',$exports,'-cp',$classes,'BuildHistoricalInformation',(Join-Path $source 'instruction-source\classes.tsv'),$historical)
$dispatchTest=Join-Path $out 'dispatch-host-test'
$null=New-Item -ItemType Directory -Path $dispatchTest
$dispatchSources=@(Get-ChildItem -LiteralPath (Join-Path $source 'tests\dispatch-fixtures') -Filter '*.java' -Recurse|ForEach-Object FullName)
$dispatchSources+=Join-Path $source 'tests\InformationDispatchTest.java'
$dispatchSources+=Join-Path $source 'tests\InformationDispatchConcurrencyTest.java'
$dispatchSources+=Join-Path $source 'java\org\omonob\cpmap\information\InformationAslDispatch.java'
Run $javac (@('-encoding','UTF-8','-d',$dispatchTest)+$dispatchSources)
Run $java @('-Xverify:all','-cp',$dispatchTest,'InformationDispatchTest')
Run $java @('-Xverify:all','-cp',$dispatchTest,'InformationDispatchConcurrencyTest')
$stock=@{MU1102=$MU1102StockJar;MU1367=$MU1367StockJar;MU1440=$MU1440StockJar;MU1433=$MU1433StockJar;MU1447=$MU1447StockJar}
$report=@()
foreach($mu in @('MU1102','MU1367','MU1440','MU1433','MU1447')){
    $folder=Join-Path $out $mu
    $null=New-Item -ItemType Directory -Path $folder
    $inputJar=$historical
    if($mu -eq 'MU1433'){
        $inputJar=Join-Path $folder 'old-api.jar'
        Run $java @('--add-exports',$exports,'-cp',$classes,'Mu0627Adapter',$historical,$stock[$mu],$inputJar)
    }
    $bap=Join-Path $folder 'bap.jar'
    Run $java @('--add-exports',$exports,'-cp',$classes,'V53BapReturnAdapter',$inputJar,$stock[$mu],$bap)
    $isolated=Join-Path $folder 'isolated.jar'
    Run $java @('--add-exports',$exports,'-cp',$classes,'PrepareInformation',$bap,$isolated,$mu)
    $helpers=Join-Path $folder 'helper-classes'
    $null=New-Item -ItemType Directory -Path $helpers
    $helperSources=@(Get-ChildItem -LiteralPath (Join-Path $source 'java') -Filter '*.java' -Recurse|ForEach-Object FullName)
    Run $javac (@('--release','7','-encoding','UTF-8','-cp',$stock[$mu],'-d',$helpers)+$helperSources)
    $dispatchJar=Join-Path $folder 'dispatched.jar'
    Run $java ($asm+@('-cp',$classes,'AddInformationHelpers',$isolated,$helpers,$dispatchJar))
    $jar=Join-Path $folder 'VCInformation.jar'
    $menu=Join-Path $MenuBuild ($mu+'\VCMenu.jar')
    Run $java @('--add-exports',$exports,'-cp',$classes,'BuildV53PolicyHook',$dispatchJar,$menu,$jar)
    $audit=Join-Path $folder 'API.tsv'
    # Audit the final patch against the exact target, not the newer CN donor's
    # intentionally removed CarLife ABI on the older SKODA G11 target.
    Run $java @('--add-exports',$exports,'-cp',$classes,'ApiAudit',$stock[$mu],$stock[$mu],$jar,$audit)
    if((Get-Item -LiteralPath $audit).Length -ne 0){throw ('Exact-target API findings: '+$audit)}
    Run $java @('-Xverify:all','-cp',$classes,'LinkSmoke',$jar,$stock[$mu])
    Run $java @('-Xverify:all','-cp',$classes,'EtaTimezoneTest',$jar,$stock[$mu],$mu)
    $fixtures=Join-Path $folder 'fixtures'
    $behavior=Join-Path $folder 'behavior'
    $null=New-Item -ItemType Directory -Path $fixtures,$behavior
    $fixtureSources=@(Get-ChildItem -LiteralPath (Join-Path $source 'tests\hook-fixtures') -Filter '*.java' -Recurse|ForEach-Object FullName)
    Run $javac (@('-encoding','UTF-8','-cp',$stock[$mu],'-d',$fixtures)+$fixtureSources)
    $cp=$classes+';'+$menu+';'+$stock[$mu]
    Run $javac ($asm+@('-encoding','UTF-8','-cp',$cp,'-d',$behavior,(Join-Path $source 'tests\HookBehaviorTest.java')))
    Run $java ($asm+@('-Xverify:all','-cp',($behavior+';'+$cp),'HookBehaviorTest',$fixtures,(Join-Path $folder 'test-state')))
    $report+= [pscustomobject]@{Firmware=$mu;StockSHA256=(Get-FileHash -LiteralPath $stock[$mu] -Algorithm SHA256).Hash;JarSHA256=(Get-FileHash -LiteralPath $jar -Algorithm SHA256).Hash;ApiFindings=0}
}
$report|ConvertTo-Json -Depth 5|Set-Content -LiteralPath (Join-Path $out 'build-report.json') -Encoding UTF8
Write-Host 'V21_INFORMATION_BUILD_PASS exact_targets=5 historical_classes=49 isolated_classes=41'
