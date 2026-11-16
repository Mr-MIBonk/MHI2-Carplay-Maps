"""Synthetic host filesystems only. No QNX program, network, mount or reboot.

The production scripts are executed, with exactly three native helpers replaced
by host builds. Fixture-local sync is inert: never flush the host/other VM.
"""
from pathlib import Path
import hashlib,json,os,shutil,subprocess,sys,tempfile
REPO=Path(__file__).resolve().parents[1]
SD=Path(sys.argv[1]).resolve();OUT=Path(sys.argv[2]);OUT.mkdir()
PAYLOAD=SD/'carplay/payload';CHECK=OUT/'file-check';CONFIG=OUT/'config-patch';NAV=OUT/'cpmap-nav-check'
PROFILES=[('MU1102','790',55915411),('MU1102','791',55915411),('MU1367','790',55840541),('MU1367','791',55840541),('MU1440','790',55840933),('MU1433','790',52751492),('MU1447','790',55712574)]
checks=[];runs=[]
def check(value,label):
    assert value,label
    checks.append(label)
def command(name,args,env=None,ok=True):
    p=subprocess.run(list(map(str,args)),capture_output=True,text=True,env=env,timeout=60)
    (OUT/(name+'.out')).write_text(p.stdout);(OUT/(name+'.err')).write_text(p.stderr)
    check((p.returncode==0)==ok,(name,p.returncode,p.stdout[-5000:],p.stderr[-3000:]))
    return p
for source,target in [('standalone/file-check.c',CHECK),('standalone/config-patch.c',CONFIG),('standalone/cpmap-nav-check.c',NAV)]:
    command('compile-'+target.name,['/usr/bin/gcc','-B/usr/bin/','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation',REPO/'src'/source,'-o',target])
command('integrity',['sh','-c','"$1" --self-test && "$1" --manifest "$2" "$2/CONTENTS.sha256"','check',CHECK,SD])

def put(path,data):
    path.parent.mkdir(parents=True,exist_ok=True)
    path.write_bytes(data.encode() if isinstance(data,str) else data)
def fixture(root,size):
    put(root/'ifs/lsd.jxe',b'')
    with (root/'ifs/lsd.jxe').open('wb') as f:f.truncate(size)
    elf=bytearray(52);elf[:6]=b'\x7fELF\x01\x01';elf[18]=40
    put(root/'mnt/app/eso/bin/apps/dio_manager',bytes(elf)+b'factory-dio'*65000)
    put(root/'mnt/app/armle/usr/sbin/mm-ipod',bytes(elf)+b'factory-mm-ipod-fixture')
    put(root/'mnt/app/eso/bin/apps/connectionmanager',b'factory-cm-fixture')
    put(root/'mnt/app/eso/hmi/lsd/lsd.sh','#!/bin/sh\nBASE_DIR=/mnt/app/eso/hmi\nBOOTCLASSPATH=""\n$J9 $BOOTCLASSPATH Main\n')
    (root/'mnt/app/eso/hmi/lsd/jars').mkdir()
    (root/'mnt/app/eso/hmi/engdefs/scripts').mkdir(parents=True)
    put(root/'mnt/app/eso/hmi/lsd/config/bundles.properties','# original\nBundle.Startup.Lateload = 61,62,88\nBundle.Name.61 = Original\nBundle.Activator.61 = original.Activator\nBundle.Name.98 = OtherProject\n')
    put(root/'mnt/system/etc/eso/production/connectivity.json','{# OEM comment\n"manager":{"btstack":{"exec":"/eso/bin/apps/btstack","customer":{"exec":"/elsewhere"}}},"other":42}\n')
    put(root/'mnt/system/etc/eso/production/smartphone_integrator.json','{"roles":{"carplay":{"exec":"dio_manager","path":"/eso/bin/apps","envs":["LD_LIBRARY_PATH=/lib","KEEP_OTHER_PROJECT=1"]}},"another":{"exec":"not-a-carplay-role"}}\n')
    for d in ('ramdisk','dev/shmem','mnt/ota','mnt/persist'):(root/d).mkdir(parents=True,exist_ok=True)
    protected={'mnt/app/eso/hmi/engdefs/mqb.esd':b'mqb untouched','mnt/app/other-project/VehicleDataTap':b'another project',
               'mnt/app/sshd_test':b'SSH original','etc/shadow':b'synthetic-password-must-not-change',
               'mnt/icab/persistent/cpmap-ui/user-test':b'preserve-settings',
               'net/rcc/mnt/efs-system/opt/audio/tones/systemtone15.wav':b'do-not-touch-rcc',
               'net/rcc/mnt/efs-persist/SWDL/service.log':b'do-not-clear-history'}
    for path,b in protected.items():put(root/path,b)
    commands=root/'commands';commands.mkdir();put(commands/'sync','#!/bin/sh\nexit 0\n');(commands/'sync').chmod(0o755)
    return protected
def env(root):return {**os.environ,'PATH':str(root/'commands')+':'+os.environ['PATH'],
    'CPMAP_TEST_ROOT':str(root),'CPMAP_TEST_MEDIA':str(SD),'CPMAP_TEST_FILE_CHECK':str(CHECK),
    'CPMAP_TEST_CONFIG_PATCH':str(CONFIG),'CPMAP_TEST_NAV_CHECK':str(NAV)}
def entry(root):return root/'mnt/app/eso/hmi/engdefs/scripts/wireless-carplay-maps/entry.sh'
def invoke(root,action,*,ok=True,media=SD,label=None):
    script=entry(root) if entry(root).exists() else SD/'install/scripts/entry.sh'
    result=command(label or str(len(checks))+'-'+action,['sh',script,action,media],env(root),ok)
    return result.stdout
def baseline(root):
    # Only target system files; test command files and the owned entry are not
    # another project's data. No host root/path is ever a fixture delete target.
    exclude='/mnt/app/eso/hmi/engdefs/scripts/wireless-carplay-maps/'
    return {p.relative_to(root).as_posix():p.read_bytes() for p in root.rglob('*') if p.is_file()
            and exclude not in '/'+p.relative_to(root).as_posix() and not p.relative_to(root).as_posix().startswith(('commands/','ramdisk/'))}
def protected_ok(root,protected):
    for path,b in protected.items():check((root/path).read_bytes()==b,'unchanged '+path)

for index,(mu,cluster,size) in enumerate(PROFILES):
    with tempfile.TemporaryDirectory(prefix='wmaps-profile-') as td:
        root=Path(td);protected=fixture(root,size);original=baseline(root)
        invoke(root,'install-entry');before=baseline(root);invoke(root,'install-'+cluster)
        owner=entry(root).parent
        first={p.relative_to(owner/'feature-backup').as_posix():p.read_bytes() for p in (owner/'feature-backup').rglob('*') if p.is_file()}
        startup=(root/'mnt/app/eso/hmi/lsd/lsd.sh').read_text()
        check(startup.count('-Xbootclasspath/p:')==1,'single classpath '+mu+cluster)
        if mu=='MU1102':
            check(startup.count('WirelessMapsBootstrap.jar')==1,'connection-only bundle '+cluster)
            check('WirelessInfo' not in startup and 'VCInformation' not in startup,'no information bootstrap')
            check('Bundle.Name.98 = OtherProject' in (root/'mnt/app/eso/hmi/lsd/config/bundles.properties').read_text(),'other bundle preserved')
            expect=40 if cluster=='791' else 60
            check('maxfps=%d\n'%expect in (root/'mnt/persist/altscreen.conf').read_text(),'cluster policy '+cluster)
            check('WCP_FPS=30\nWCP_VIDEO_FPS=30' in (root/'mnt/persist/v75-wireless-display.conf').read_text(),'primary30 config')
            core=root/'mnt/app/carplaymaps'
            check((core/'wireless-entry').read_bytes()==(PAYLOAD/'wireless/profiles'/('AID12.3' if cluster=='791' else 'AID10.5-17PM')/'wireless-entry').read_bytes(),'entry hash exact')
            check(not any(any(s in p.name for s in ('cover','Information','normaliz','intro.eft','welcome','systemtone')) for p in core.rglob('*')),'no removed runtime module/assets')
        else:check(not (root/'mnt/ota/wcp_receiver').exists(),'unported wireless not installed '+mu)
        invoke(root,'update-feature');invoke(root,'update-entry');invoke(root,'update-feature')
        afterfirst={p.relative_to(owner/'feature-backup').as_posix():p.read_bytes() for p in (owner/'feature-backup').rglob('*') if p.is_file()}
        check(first==afterfirst,'first baseline survives feature and entry updates '+mu+cluster)
        invoke(root,'restore-feature');check(baseline(root)==before,'full first baseline restored '+mu+cluster)
        invoke(root,'uninstall');check(not entry(root).exists(),'manager removed '+mu+cluster)
        check(baseline(root)==original,'uninstall returns original system files '+mu+cluster)
        protected_ok(root,protected)
        runs.append({'profile':mu+'/'+cluster,'result':'INSTALL_UPDATE_ENTRY_UPDATE_RESTORE_UNINSTALL_PASS'})
        print('STANDALONE_FULL_LIFECYCLE_PASS',mu,cluster,flush=True)

for mode in ('unknown','phone-usb','phone-wireless','private','foreign-entry','foreign-esd','wrong-cluster','foreign-ota','json-owner','bundle-full','symlink-target'):
    with tempfile.TemporaryDirectory(prefix='wmaps-negative-') as td:
        root=Path(td);protected=fixture(root,99 if mode=='unknown' else 55840933 if mode=='wrong-cluster' else 55915411)
        if mode=='phone-usb':put(root/'dev/ipod0','1')
        if mode=='phone-wireless':put(root/'dev/shmem/wcp_cp_session','1')
        if mode=='private':put(root/'mnt/app/carplaymaps/cpmap-auth','synthetic-private-marker')
        if mode=='foreign-entry':(entry(root).parent).mkdir()
        if mode=='foreign-esd':put(root/'mnt/app/eso/hmi/engdefs/WirelessCarPlayMaps.esd','foreign')
        if mode=='foreign-ota':put(root/'mnt/ota/wcp_ctl.sh','foreign')
        if mode=='json-owner':put(root/'mnt/system/etc/eso/production/connectivity.json','{"btstack":{"exec":"/other-project/btstack"}}')
        if mode=='bundle-full':put(root/'mnt/app/eso/hmi/lsd/config/bundles.properties','Bundle.Startup.Lateload = 61,97,98,99\n')
        if mode=='symlink-target':
            (root/'mnt/app/eso/bin/apps/dio_manager').unlink();(root/'mnt/app/eso/bin/apps/dio_manager').symlink_to(root/'etc/shadow')
        before=baseline(root)
        if mode in ('unknown','phone-usb','phone-wireless','foreign-entry','foreign-esd'):invoke(root,'install-entry',ok=False)
        else:
            invoke(root,'install-entry');before=baseline(root)
            invoke(root,'install-791' if mode=='wrong-cluster' else 'install-790',ok=False)
        check(baseline(root)==before,'fail closed before mutation '+mode);protected_ok(root,protected)
        print('FAIL_CLOSED_PASS',mode,flush=True)

for failure in ('core-menu','wireless-file','generated-config'):
    with tempfile.TemporaryDirectory(prefix='wmaps-failure-') as td:
        root=Path(td);protected=fixture(root,55915411);invoke(root,'install-entry');before=baseline(root)
        patterns={'core-menu':'*/VCMenu.jar.new','wireless-file':'*/wireless-receiver-exec.wmaps-new','generated-config':'*/connectivity.json.wmaps-new'}
        put(root/'commands/mv','#!/bin/sh\nsource=$1\n[ "$source" != -f ] || source=$2\ncase "$source" in '+patterns[failure]+')exit 99;;esac\nexec /bin/mv "$@"\n');(root/'commands/mv').chmod(0o755)
        out=invoke(root,'install-790',ok=False)
        check('WMAPS_ROLLED_BACK' in out,'rollback receipt '+failure)
        after=baseline(root)
        check(after==before,('rollback exact '+failure,sorted(p for p in set(before)|set(after) if before.get(p)!=after.get(p))))
        check(not (entry(root).parent/'feature-backup').exists(),'failed first install does not retain committed baseline '+failure)
        protected_ok(root,protected);print('FAULT_ROLLBACK_PASS',failure,flush=True)

with tempfile.TemporaryDirectory(prefix='wmaps-update-failure-') as td:
    root=Path(td);fixture(root,55915411);invoke(root,'install-entry');invoke(root,'install-790')
    before=baseline(root);owner=entry(root).parent
    metadata={n:(owner/n).read_bytes() for n in ['installed.tsv','INSTALLED_PROFILE']}
    put(root/'commands/mv','#!/bin/sh\nsource=$1\n[ "$source" != -f ] || source=$2\ncase "$source" in */wireless-receiver-exec.wmaps-new)exit 99;;esac\nexec /bin/mv "$@"\n');(root/'commands/mv').chmod(0o755)
    invoke(root,'update-feature',ok=False)
    check(baseline(root)==before,'failed update restores previously installed runtime exactly')
    check(all((owner/n).read_bytes()==b for n,b in metadata.items()),'failed update restores manager receipts')
    (root/'commands/mv').unlink()
    # A one-shot restore rename failure must restore the currently installed build.
    marker=root/'commands/failed-once'
    put(root/'commands/mv','#!/bin/sh\nsource=$1\n[ "$source" != -f ] || source=$2\ncase "$source" in */connectivity.json.wmaps-restore)if [ ! -e "'+str(marker)+'" ];then touch "'+str(marker)+'";exit 99;fi;;esac\nexec /bin/mv "$@"\n');(root/'commands/mv').chmod(0o755)
    invoke(root,'restore-feature',ok=False)
    check(baseline(root)==before,'failed restore returns to installed runtime exactly')
    check(all((owner/n).read_bytes()==b for n,b in metadata.items()),'failed restore retains receipts')
    print('UPDATE_AND_RESTORE_FAULT_RECOVERY_PASS',flush=True)

with tempfile.TemporaryDirectory(prefix='wmaps-single-phone-') as td:
    root=Path(td);fixture(root,55915411);invoke(root,'install-entry');invoke(root,'install-790')
    check(not (PAYLOAD/'wireless/profiles/AID10.5-16e').exists(),'no second active phone declaration')
    invoke(root,'install-790-16e',ok=False)
    put(root/'mnt/system/etc/eso/production/connectivity.json','another project changed this later')
    before=baseline(root);invoke(root,'restore-feature',ok=False)
    check(before==baseline(root),'do not overwrite post-install other-project change')

with tempfile.TemporaryDirectory(prefix='wmaps-config-') as td:
    t=Path(td);inp=t/'input';out=t/'output'
    for i,text in enumerate(['{}','{"btstack":{"exec":"bad"}}','{"btstack":{"exec":"/eso/bin/apps/btstack"},"nested":{"btstack":{"exec":"/eso/bin/apps/btstack"}}}','{"btstack":{"exec":"/eso/bin/apps/btstack","exec":"/eso/bin/apps/btstack"}}','{"broken":','{"btstack":{"exec":"/eso/bin/apps/btstack"},}']):
        put(inp,text);command('bad-config-'+str(i),[CONFIG,'connectivity',inp,out],ok=False);check(not out.exists(),'bad config never produces target '+str(i))
    put(inp,'{"roles":{"carplay":{"exec":"dio_manager","path":"/eso/bin/apps","envs":["KEEP=1","LD_PRELOAD=/other/project.so"]}}}')
    command('foreign-preload',[CONFIG,'si',inp,out],ok=False);check(not out.exists(),'foreign preload refused')

with tempfile.TemporaryDirectory(prefix='wmaps-manifest-') as td:
    t=Path(td);data=b'owned-test';put(t/'file',data);line=hashlib.sha256(data).hexdigest()+'  file\n'
    for i,value in enumerate([line+line,hashlib.sha256(data).hexdigest()+'  ../escape\n',line.replace('file','/absolute'),line.rstrip('\n'),line.replace('file','file\r')]):
        put(t/'manifest',value);command('bad-manifest-'+str(i),[CHECK,'--manifest',t,t/'manifest'],ok=False)
    (t/'alias').symlink_to(t/'file');put(t/'manifest',line.replace('file','alias'))
    command('symlink-manifest',[CHECK,'--manifest',t,t/'manifest'],ok=False)

for file in SD.rglob('*.sh'):command('syntax-'+str(len(checks)),['sh','-n',file])
report={'result':'OFFLINE_STANDALONE_SYNTHETIC_PASS','checks':len(checks),'profiles':runs,'headunit_accessed':False,
        'actual_QNX_loader':False,'phone_acceptance':False,'host_global_sync':False}
(OUT/'REPORT.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
