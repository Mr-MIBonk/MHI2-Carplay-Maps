"""Run historical public installers and R2 only in synthetic host filesystems.
No headunit, QNX runtime, host-global sync, service signal or network is used.
Historical sources/payloads are caller-supplied private test inputs, not bundled.
"""
from pathlib import Path
import hashlib,json,os,shutil,stat,subprocess,sys,tempfile
import zipfile
REPO=Path(__file__).resolve().parents[1];SD=Path(sys.argv[1]);OUT=Path(sys.argv[2]);OUT.mkdir()
CHECK=OUT/'file-check';CONFIG=OUT/'config-patch';NAV=OUT/'cpmap-nav-check'
sizes={'MU1102':55915411,'MU1367':55840541,'MU1440':55840933,'MU1433':52751492,'MU1447':55712574}
checks=0;cases=[];logs=0
def check(value,label):
    global checks
    assert value,label
    checks+=1
def run(args,environment=None,ok=True):
    global logs
    logs+=1;p=subprocess.run(list(map(str,args)),capture_output=True,text=True,env=environment,timeout=60)
    (OUT/('%04d.out'%logs)).write_text(p.stdout);(OUT/('%04d.err'%logs)).write_text(p.stderr)
    check((p.returncode==0)==ok,(list(map(str,args)),p.returncode,p.stdout[-6000:],p.stderr[-3000:]))
    return p.stdout
for source,file in [('file-check.c',CHECK),('config-patch.c',CONFIG),('cpmap-nav-check.c',NAV)]:
    run(['/usr/bin/gcc','-B/usr/bin/','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation',REPO/'src/standalone'/source,'-o',file])
def put(file,data,mode=0o644):
    file.parent.mkdir(parents=True,exist_ok=True);file.write_bytes(data.encode() if isinstance(data,str) else data);file.chmod(mode)
def fixture(root,mu):
    put(root/'ifs/lsd.jxe',b'')
    with (root/'ifs/lsd.jxe').open('wb') as f:f.truncate(sizes[mu])
    elf=bytearray(52);elf[:6]=b'\x7fELF\x01\x01';elf[18]=40
    put(root/'mnt/app/eso/bin/apps/dio_manager',bytes(elf)+b'factory-dio'*65000,0o755)
    put(root/'mnt/app/armle/usr/sbin/mm-ipod',bytes(elf)+b'factory-ipod',0o755)
    put(root/'mnt/app/eso/bin/apps/connectionmanager',bytes(elf)+b'factory-cm',0o755)
    put(root/'mnt/app/eso/hmi/lsd/lsd.sh','#!/bin/sh\nBASE_DIR=/mnt/app/eso/hmi\nBOOTCLASSPATH=""\n$J9 $BOOTCLASSPATH Main\n',0o755)
    (root/'mnt/app/eso/hmi/lsd/jars').mkdir()
    (root/'mnt/app/eso/hmi/engdefs/scripts').mkdir(parents=True)
    put(root/'mnt/app/eso/hmi/lsd/config/bundles.properties','Bundle.Startup.Lateload = 61,62,88\nBundle.Name.98 = OtherProject\n')
    put(root/'mnt/system/etc/eso/production/connectivity.json','{# preserved comment\n"btstack":{"exec":"/eso/bin/apps/btstack","otherProject":true},"outside":"unchanged"}\n')
    put(root/'mnt/system/etc/eso/production/smartphone_integrator.json','{"carplay":{"exec":"dio_manager","path":"/eso/bin/apps","envs":["KEEP_OTHER_PROJECT=1"]}}\n')
    for d in ('ramdisk','dev/shmem','mnt/ota','mnt/persist'):(root/d).mkdir(parents=True,exist_ok=True)
    for name in ('sync','mount','slay','on','reboot','shutdown','killall'):
        put(root/'commands'/name,'#!/bin/sh\n'+('exit 0\n' if name=='sync' else 'echo UNEXPECTED_LIVE_COMMAND >&2\nexit 99\n'),0o755)
    for name,data in {'mnt/app/other-project/keep':'keep','mnt/app/eso/hmi/engdefs/mqb.esd':'existing toolbox',
                      'etc/shadow':'synthetic-password-unchanged','mnt/app/sshd_test':'SSH unchanged',
                      'net/rcc/mnt/efs-system/opt/audio/tones/systemtone15.wav':'RCC untouched',
                      'mnt/icab/persistent/cpmap-ui/user-test':'user-preference-unchanged'}.items():put(root/name,data)
def env(root,media=SD):return {**os.environ,'PATH':str(root/'commands')+':'+os.environ['PATH'],
    'CPMAP_TEST_ROOT':str(root),'CPMAP_TEST_FILE_CHECK':str(CHECK),'CPMAP_TEST_NAV_CHECK':str(NAV),
    'CPMAP_TEST_CONFIG_PATCH':str(CONFIG),'CPMAP_TEST_MEDIA':str(media)}
def entry(root):return root/'mnt/app/eso/hmi/engdefs/scripts/wireless-carplay-maps/entry.sh'
def invoke(root,action,media=SD,ok=True):
    script=entry(root) if entry(root).exists() else media/'install/scripts/entry.sh'
    return run(['sh',script,action,media],env(root,media),ok)
def snapshot(root,exclude_manager=True):
    result={}
    for p in root.rglob('*'):
        if not p.is_file():continue
        rel=p.relative_to(root).as_posix()
        if rel.startswith(('ifs/','ramdisk/','commands/')):continue
        if exclude_manager and rel.startswith('mnt/app/eso/hmi/engdefs/scripts/wireless-carplay-maps/'):continue
        result[rel]=(hashlib.sha256(p.read_bytes()).hexdigest(),stat.S_IMODE(p.stat().st_mode))
    return result
def delta(a,b):return [p for p in set(a)|set(b) if a.get(p)!=b.get(p)]
history=json.loads(Path('/history/HISTORY_FIXTURES.json').read_text())
matrix=[]
for record in history:
    base=Path('/inputs')/Path(record.get('tree',record.get('payload'))).relative_to('inputs')
    profiles={}
    for core in sorted(base.rglob('core.so')):
        if core.parent.parent.parent.name!='profiles':continue
        mu=core.parent.parent.name;cluster=core.parent.name
        if mu in sizes and cluster in ('790','791'):profiles.setdefault((mu,cluster),core.parents[3])
    for (mu,cluster),payload in sorted(profiles.items()):
        installer=payload/'common/install-carplay-maps.sh'
        if not installer.exists():installer=base/'src/toolbox/install-carplay-maps.sh'
        check(installer.is_file(),'historical installer found '+record['tag'])
        text=installer.read_text('utf-8-sig')
        check('CPMAP_TEST_ROOT' in text,'historical target override audited '+record['tag'])
        matrix.append((record,mu,cluster,payload,installer))
focus=os.environ.get('WMAPS_TEST_FOCUS')
if focus=='smoke':matrix=[r for r in matrix if r[0]['tag'] in ('toolbox-plaintext-v1.0.0','toolbox-plaintext-v2.1.0-rc.7') and r[1:] and r[1:3]==('MU1102','790')]
for record,mu,cluster,payload,installer in matrix:
    with tempfile.TemporaryDirectory(prefix='wmaps-public-upgrade-') as td:
        root=Path(td);fixture(root,mu)
        if record['tag']=='toolbox-plaintext-v1.0.0':
            # v1.0 required a separately installed coexistence patch and an
            # existing prepend line; it did not install that prerequisite.
            jars=root/'mnt/app/eso/hmi/lsd/jars'
            shutil.copyfile(SD/'carplay/payload/common/CarPlayNavigationPatch.jar',jars/'NavActiveIgnore.jar')
            put(root/'mnt/app/eso/hmi/lsd/lsd.sh','#!/bin/sh\nBASE_DIR=/mnt/app/eso/hmi\nBOOTCLASSPATH=""\nBOOTCLASSPATH="$BOOTCLASSPATH -Xbootclasspath/p:$BASE_DIR/lsd/jars/NavActiveIgnore.jar"\n$J9 $BOOTCLASSPATH Main\n',0o755)
        run(['sh',installer,payload,mu,cluster],env(root))
        initial=snapshot(root);ipod=root/'mnt/app/armle/usr/sbin/mm-ipod';ipod_before=ipod.read_bytes()
        has_info=(root/'mnt/app/carplaymaps/information/ENABLED').exists()
        invoke(root,'install-entry');old=snapshot(root)
        invoke(root,'install-'+cluster)
        check((root/'mnt/app/carplaymaps/VERSION').read_text().strip()=='WIRELESS-CARPLAY-MAPS-3.0','new version exact')
        check(ipod.read_bytes().startswith(b'\x7fELF'),'native mm-ipod restored/preserved')
        check(not (root/'mnt/app/carplaymaps/information').exists(),'old information runtime removed')
        check(not (root/'mnt/app/eso/hmi/lsd/jars/VCInformation.jar').exists(),'known information jar removed')
        check('VCInformation.jar' not in (root/'mnt/app/eso/hmi/lsd/lsd.sh').read_text(),'old information classpath removed')
        invoke(root,'update-feature')
        invoke(root,'restore-feature')
        restored=snapshot(root);check(restored==old,('previous public version exact restore',record['tag'],mu,cluster,delta(old,restored)))
        check(ipod.read_bytes()==ipod_before,'old driver/wrapper exact on undo')
        invoke(root,'uninstall');restored=snapshot(root)
        check(restored==initial,('uninstall own entry only',record['tag'],mu,cluster,delta(initial,restored)))
        cases.append({'version':record['tag'],'profile':mu+'/'+cluster,'had_information':has_info,'result':'OLD_INSTALL_UPGRADE_REPEAT_RESTORE_UNINSTALL_PASS'})
        print('PUBLIC_UPGRADE_PASS',record['tag'],mu,cluster,flush=True)

# Reproduce the released R1 regression and prove that R2 can recover using R1's
# retained pre-upgrade snapshot rather than guessing a stock driver.
rc=next(r for r in matrix if r[0]['tag']=='toolbox-plaintext-v2.1.0-rc.7' and r[1:3]==('MU1102','790'))
with tempfile.TemporaryDirectory(prefix='wmaps-r1-regression-') as td:
    root=Path(td);fixture(root,'MU1102');run(['sh',rc[4],rc[3],'MU1102','790'],env(root))
    old_ipod=(root/'mnt/app/armle/usr/sbin/mm-ipod').read_bytes()
    invoke(root,'install-entry',Path('/r1'));invoke(root,'install-790',Path('/r1'))
    check((root/'mnt/app/armle/usr/sbin/mm-ipod').read_bytes()==old_ipod,'R1 leaves old wrapper')
    check(not (root/'mnt/app/carplaymaps/stock_first/mm-ipod.real').exists(),'R1 removes its dependency: reproduced')
    invoke(root,'update-entry');invoke(root,'update-feature')
    check((root/'mnt/app/armle/usr/sbin/mm-ipod').read_bytes().startswith(b'\x7fELF'),'R2 recovers native from first snapshot')
    invoke(root,'restore-feature')
    check((root/'mnt/app/carplaymaps/information/ENABLED').exists(),'first snapshot restores old version information')
    check((root/'mnt/app/armle/usr/sbin/mm-ipod').read_bytes()==old_ipod,'first snapshot wrapper restore covered after journal expansion')
    print('R1_V21_REGRESSION_REPRODUCED_AND_R2_RECOVERY_PASS',flush=True)

for action in ('install-790','install-790-16e','install-791'):
    with tempfile.TemporaryDirectory(prefix='wmaps-r1-upgrade-') as td:
        root=Path(td);fixture(root,'MU1102');invoke(root,'install-entry',Path('/r1'));invoke(root,action,Path('/r1'))
        invoke(root,'update-entry');invoke(root,'update-feature')
        selected=(entry(root).parent/'INSTALLED_PROFILE').read_text().strip()
        check(selected.endswith('/791/791' if action=='install-791' else '/790/17PM'),'R1 saved profile migration '+action)
        check(not (SD/'carplay/payload/wireless/profiles/AID10.5-16e').exists(),'no second declaration payload')
        invoke(root,'restore-feature')
        check(not (root/'mnt/app/carplaymaps').exists(),'R1 first factory snapshot preserved '+action)
    print('R1_PROFILE_UPGRADE_PASS',action,flush=True)

with tempfile.TemporaryDirectory(prefix='wmaps-old-management-') as td:
    root=Path(td);fixture(root,'MU1102');run(['sh',rc[4],rc[3],'MU1102','790'],env(root))
    run(['sh','/old-manager/install/scripts/entry.sh','install-entry','/old-manager'],env(root,Path('/old-manager')))
    legacy=root/'mnt/app/eso/hmi/engdefs/scripts/cpmapvc-local/entry.sh'
    legacy_bytes=legacy.read_bytes()
    invoke(root,'install-entry');before=snapshot(root)
    invoke(root,'install-790')
    check(b'migrated to Wireless CarPlay Maps' in legacy.read_bytes(),'known legacy manager safely retired')
    invoke(root,'restore-feature')
    check(snapshot(root)==before,'old module and independent management entry restored exactly')
    check(legacy.read_bytes()==legacy_bytes,'legacy script bytes exact')
    print('OLD_STANDALONE_MANAGEMENT_MIGRATION_PASS',flush=True)

for hazard in ['unknown-ipod-wrapper','missing-ipod-stock','unknown-info-jar','wrong-previous-firmware',
               'modified-modern-startup','unexpected-recreated-jar','corrupt-runtime-backup',
               'extra-backup-file','unowned-recovery-stage','unknown-legacy-manager','corrupt-journal-map','missing-journal-seal',
               'truncated-journal-index','missing-runtime-backup']:
    with tempfile.TemporaryDirectory(prefix='wmaps-risk-') as td:
        root=Path(td);fixture(root,'MU1102')
        modern=hazard in ['modified-modern-startup','unexpected-recreated-jar','corrupt-runtime-backup','extra-backup-file','unowned-recovery-stage','corrupt-journal-map','missing-journal-seal','truncated-journal-index','missing-runtime-backup']
        if not modern or hazard in ['corrupt-runtime-backup','extra-backup-file','missing-runtime-backup']:
            run(['sh',rc[4],rc[3],'MU1102','790'],env(root))
        invoke(root,'install-entry')
        if modern:invoke(root,'install-790')
        owner=entry(root).parent
        if hazard=='unknown-ipod-wrapper':
            p=root/'mnt/app/armle/usr/sbin/mm-ipod';put(p,p.read_bytes()+b'\n# another project hook\n',0o755)
        if hazard=='missing-ipod-stock':(root/'mnt/app/carplaymaps/stock_first/mm-ipod.real').unlink()
        if hazard=='unknown-info-jar':put(root/'mnt/app/eso/hmi/lsd/jars/VCInformation.jar',b'unknown-other-build')
        if hazard=='wrong-previous-firmware':put(root/'mnt/app/carplaymaps/PROFILE','MU1367/790\n')
        if hazard=='modified-modern-startup':
            p=root/'mnt/app/eso/hmi/lsd/lsd.sh';put(p,p.read_text()+'# another project startup\n',0o755)
        if hazard=='unexpected-recreated-jar':put(root/'mnt/app/eso/hmi/lsd/jars/VCInformation.jar',b'another project created this later')
        if hazard=='corrupt-runtime-backup':put(owner/'feature-backup/runtime/core.so',b'corrupted old snapshot')
        if hazard=='extra-backup-file':put(owner/'feature-backup/runtime/unknown-extra',b'not in sealed manifest')
        if hazard=='unowned-recovery-stage':put(owner/'.feature-backup-new/KEEP_ME',b'unfinished recovery must survive')
        if hazard=='unknown-legacy-manager':put(root/'mnt/app/eso/hmi/engdefs/scripts/cpmapvc-local/OWNER','another project')
        if hazard=='corrupt-journal-map':
            p=owner/'feature-backup/files.tsv';put(p,p.read_text().replace('/mnt/app/eso/bin/apps/dio_manager','/mnt/app/eso/bin/apps/connectionmanager',1))
        if hazard=='missing-journal-seal':(owner/'feature-backup/INDEX_SHA256').unlink()
        if hazard=='truncated-journal-index':
            p=owner/'feature-backup/INDEX_SHA256';put(p,p.read_text().splitlines()[0]+'\n')
        if hazard=='missing-runtime-backup':shutil.rmtree(owner/'feature-backup/runtime')
        before=snapshot(root,False)
        action='restore-feature' if hazard in ['unexpected-recreated-jar','corrupt-runtime-backup','extra-backup-file','corrupt-journal-map','missing-journal-seal','truncated-journal-index','missing-runtime-backup'] else 'update-feature' if modern else 'install-790'
        invoke(root,action,ok=False)
        after=snapshot(root,False);check(after==before,('risk fails without target mutation',hazard,delta(before,after)))
        if hazard=='unexpected-recreated-jar':invoke(root,'update-feature',ok=False);check(snapshot(root,False)==before,'missing-file receipt also protects updates')
        print('UPGRADE_RISK_FAIL_CLOSED_PASS',hazard,flush=True)

# If committing the extended first-baseline journal fails, the previous
# journal and runtime must both survive, including old 16e selection metadata.
with tempfile.TemporaryDirectory(prefix='wmaps-first-journal-failure-') as td:
    root=Path(td);fixture(root,'MU1102');invoke(root,'install-entry',Path('/r1'));invoke(root,'install-790-16e',Path('/r1'))
    invoke(root,'update-entry');before=snapshot(root,False)
    put(root/'commands/mv','#!/bin/sh\nsource=$1\n[ "$source" != -f ] || source=$2\ncase "$source" in */.feature-backup-new)exit 99;;esac\nexec /bin/mv "$@"\n',0o755)
    invoke(root,'update-feature',ok=False)
    after=snapshot(root,False);check(after==before,('extended first journal rollback exact',delta(before,after)))
    print('FIRST_JOURNAL_EXTENSION_FAULT_ROLLBACK_PASS',flush=True)

r2_cases=[]
for mu,cluster in [('MU1102','790'),('MU1102','791'),('MU1367','790'),('MU1367','791'),('MU1440','790'),('MU1433','790'),('MU1447','790')]:
    with tempfile.TemporaryDirectory(prefix='wmaps-published-predecessor-') as td:
        root=Path(td);fixture(root,mu);original=snapshot(root)
        invoke(root,'install-entry',Path('/r2'));invoke(root,'install-'+cluster,Path('/r2'))
        check((root/'mnt/app/carplaymaps/VERSION').read_text().strip()=='WIRELESS-CARPLAY-MAPS-LOCAL-2','actual published predecessor installed')
        invoke(root,'update-entry');invoke(root,'update-feature');invoke(root,'update-feature')
        check((root/'mnt/app/carplaymaps/VERSION').read_text().strip()=='WIRELESS-CARPLAY-MAPS-3.0','V3.0 installed from predecessor')
        invoke(root,'restore-feature')
        restored=snapshot(root);restored.pop('mnt/app/eso/hmi/engdefs/WirelessCarPlayMaps.esd',None)
        check(restored==original,('published predecessor first baseline preserved',mu,cluster,delta(original,restored)))
        invoke(root,'uninstall');check(snapshot(root)==original,'published predecessor migration uninstall exact')
        r2_cases.append(mu+'/'+cluster)
        print('PUBLISHED_PREDECESSOR_TO_V3_UPGRADE_PASS',mu,cluster,flush=True)

with tempfile.TemporaryDirectory(prefix='wmaps-tree-checker-') as td:
    root=Path(td);tree=root/'tree';tree.mkdir();put(tree/'nested/file',b'\x00\xfftest')
    proof=root/'tree.sha256';run([CHECK,'--tree-hash',tree,proof]);run([CHECK,'--tree-check',tree,proof])
    put(tree/'extra',b'not listed');run([CHECK,'--tree-check',tree,proof],ok=False);(tree/'extra').unlink()
    (tree/'alias').symlink_to(tree/'nested/file');run([CHECK,'--tree-hash',tree,root/'bad.sha256'],ok=False)
    print('SEALED_RUNTIME_TREE_MALFORMATION_AND_EXTRA_FILE_GATES_PASS',flush=True)

# A synthetic POSIX host can have more commands than the real MMX image.
# An absent required command must be diagnosed before taking a write lock.
with tempfile.TemporaryDirectory(prefix='wmaps-command-preflight-') as td:
    root=Path(td);fixture(root,'MU1102');invoke(root,'install-entry')
    for name in ['sh','cat','chmod','cp','expr','grep','ls','mkdir','mv','rm','rmdir','touch','sed']:
        target=shutil.which(name);check(bool(target),'host preflight test command '+name)
        (root/'commands'/name).symlink_to(target)
    before=snapshot(root,False);limited=env(root);limited['PATH']=str(root/'commands')
    output=run(['sh',entry(root),'install-790',SD],limited,ok=False)
    check('REQUIRED_TOOL_MISSING_awk' in output,'clear missing utility error')
    check(snapshot(root,False)==before,'missing utility makes no installed-file change')
    print('REQUIRED_COMMAND_PREFLIGHT_BEFORE_WRITE_PASS',flush=True)
report={'result':'OFFLINE_PUBLIC_HISTORY_UPGRADE_PASS','checks':checks,'cases':cases,'case_count':len(cases),
        'published_predecessor_upgrade_profiles':r2_cases,
        'headunit_accessed':False,'QNX_runtime_or_phone_tested':False,'R1_regression_reproduced_and_recovered':True,
        'old_16e_selection_migrated_to_only_17PM':True,'scope':'synthetic install/rollback filesystem, not visual or performance acceptance'}
(OUT/'REPORT.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
