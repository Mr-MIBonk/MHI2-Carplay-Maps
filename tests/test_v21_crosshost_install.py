"""Run installers extracted from the candidate under a host POSIX shell.

Works with Windows Git Bash + a checker compiled from the real C source, or
Linux sh + its native checker. This is filesystem/transaction evidence only,
not a substitute for QNX/emulator or real phone/cluster acceptance.
"""
from pathlib import Path,PurePosixPath
import argparse,hashlib,os,shlex,subprocess,tempfile,zipfile
from test_toolbox_install import PROFILES,fixture,digest,navigation_name
from test_information_wrapper import shell_path
from host_sync_fixture import scoped_sync_commands

def main():
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('--shell',default='sh');parser.add_argument('--checker',type=Path,required=True)
 parser.add_argument('--candidate',type=Path,required=True)
 args=parser.parse_args();checks=0
 subprocess.run([str(args.checker.resolve()),'--self-test'],check=True,timeout=10)
 with tempfile.TemporaryDirectory(prefix='cpmap-v21-archive-') as tmp:
  media=Path(tmp)
  with zipfile.ZipFile(args.candidate) as archive:
   assert archive.testzip() is None
   for entry in archive.infolist():
    parts=PurePosixPath(entry.filename)
    assert not parts.is_absolute() and '..' not in parts.parts
    if entry.filename.startswith('Custom/carplay_maps_vc/payload/'):
     target=media.joinpath(*parts.parts)
     if entry.is_dir():target.mkdir(parents=True,exist_ok=True)
     else:target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(archive.read(entry))
  payload=media/'Custom/carplay_maps_vc/payload'
  installer=payload/'common/install-carplay-maps.sh';restore=payload/'common/restore-carplay-maps.sh'
  def run(script,root,*options,success=True,commands=None):
   nonlocal checks
   environment={**os.environ,'CPMAP_TEST_ROOT':shell_path(root),
    'CPMAP_TEST_NAV_CHECK':shell_path(args.checker)}
   # Restrict only this child shell. Do not modify the user's PATH/config.
   child_path=(shell_path(commands)+':' if commands else '')+'/usr/bin:/bin'
   scoped_sync=scoped_sync_commands(root)
   if scoped_sync:child_path=shell_path(scoped_sync)+':'+child_path
   # Set PATH inside the shell as well: a Windows Git launcher may normalize
   # the inherited Windows PATH before the requested script starts.
   result=subprocess.run([args.shell,'-c','PATH="$1"; export PATH; shift; exec /bin/sh "$@"',
       'cpmap-fixture',child_path,shell_path(script),*map(str,options)],env=environment,
       capture_output=True,text=True,timeout=30)
   checks+=1
   assert (result.returncode==0)==success,result.stdout+'\n'+result.stderr
   return result.stdout
  for mu,cluster,size in PROFILES:
   nav=payload/(('profiles/'+mu+'/'+cluster+'/') if mu=='MU1433' else 'common/')/navigation_name(mu)
   for mode in ('stock','existing','merged','missing-file','partial','bad-startup'):
    with tempfile.TemporaryDirectory(prefix='cpmap-v21-host-') as tmpunit:
     root=Path(tmpunit);dio,lsd,jars=fixture(root,size,mode,nav)
     lsd.write_bytes(lsd.read_bytes().replace(b'\r\n',b'\n'))
     ipod=root/'mnt/app/armle/usr/sbin/mm-ipod'
     before=(digest(dio),digest(lsd),digest(ipod))
     preferences=root/'mnt/icab/persistent/cpmap-ui/settings.state'
     preferences.parent.mkdir(parents=True);preferences.write_bytes(b'user-preferences-are-not-reset')
     options=[shell_path(payload),mu,cluster]
     if mode=='bad-startup':
      run(installer,root,*options,success=False)
      assert before==(digest(dio),digest(lsd),digest(ipod));continue
     assert 'CPMAP_INSTALL_OK' in run(installer,root,*options)
     for name in ('VCMenu.jar','VCMirrorRate.jar','CarPlayClusterControls.jar','VCInformation.jar'):
      assert digest(jars/name)==digest(payload/'profiles'/mu/cluster/name),(mu,mode,name)
      assert lsd.read_text().count(name)==1
     saved=root/'mnt/app/carplaymaps/stock_first/mm-ipod.real'
     assert digest(saved)==before[2]
     assert 'CPMAP_PUBLIC_V21_IPOD_WRAPPER' in ipod.read_text()
     installed=digest(lsd)
     run(installer,root,*options)
     assert digest(lsd)==installed and digest(saved)==before[2]
     run(restore,root)
     assert before==(digest(dio),digest(lsd),digest(ipod))
     assert not (jars/'VCInformation.jar').exists()
     assert preferences.read_bytes()==b'user-preferences-are-not-reset'
   for suffix in ('VCMenu.jar.new','VCInformation.jar.new','mm-ipod.cpmap-new','lsd.sh.cpmap-new','dio_manager.new'):
    with tempfile.TemporaryDirectory(prefix='cpmap-v21-failure-') as tmpunit:
     root=Path(tmpunit);dio,lsd,jars=fixture(root,size,'stock',nav)
     lsd.write_bytes(lsd.read_bytes().replace(b'\r\n',b'\n'))
     ipod=root/'mnt/app/armle/usr/sbin/mm-ipod';before=(digest(dio),digest(lsd),digest(ipod))
     commands=root/'commands';commands.mkdir();mv=commands/'mv'
     mv.write_bytes(('#!/bin/sh\ncase "$1" in */'+suffix+') echo INJECTED_COMMIT_FAILURE >&2; exit 91;; esac\nexec /bin/mv "$@"\n').encode('utf-8'));mv.chmod(0o755)
     output=run(installer,root,shell_path(payload),mu,cluster,success=False,commands=commands)
     assert 'CPMAP_INSTALL_FAIL=' in output
     assert before==(digest(dio),digest(lsd),digest(ipod)),(mu,suffix,'rollback mismatch')
     assert not (jars/'VCInformation.jar').exists()
   print('V21_EXTRACTED_ARCHIVE_CROSSHOST_PASS',mu,cluster,'clean_update_restore=5','no_write_rejection=1','commit_failure_targets=5',flush=True)
 print('V21_EXTRACTED_ARCHIVE_ALL_PROFILES_PASS profiles=7 shell_operations='+str(checks)+' QNX_or_vehicle=not_claimed')
if __name__=='__main__':main()
