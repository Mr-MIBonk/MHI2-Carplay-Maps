"""Create a LOCAL full-MQB validation candidate, never uploads anything."""
from pathlib import Path,PurePosixPath
import argparse,hashlib,json,shutil,zipfile

REPO=Path(__file__).resolve().parents[1]
BASE_SHA='86aada968b035710696f7e4a7cc7a2e13bb83fb4e71ad3c4374e40ecb193cbae'
PROFILES=[('MU1102','791','MU1102'),('MU1102','790','MU1102'),('MU1367','791','MU1367'),('MU1367','790','MU1367'),('MU1440','790','MU1440_SKODA'),('MU1433','790','MU1433_SKODA'),('MU1447','790','MU1447_SEAT')]
def sha(data):return hashlib.sha256(data).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--base-archive',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--revision',type=int,default=1);a=p.parse_args()
 assert 1<=a.revision<=99
 version='2.1.0-rc.'+str(a.revision)
 assert not a.output.exists(),'Use a new candidate folder'
 assert sha(a.base_archive.read_bytes())==BASE_SHA,'Exact published 2.0.1 base required'
 replacements={};common=None
 for mu,cluster,brand in PROFILES:
  pkg=REPO/brand/('790_AID10.5' if cluster=='790' else '791_AID12.3')
  here=pkg/'payload/common'
  if common is None:common=here
  for f in common.rglob('*'):
   if f.is_file():
    relative=f.relative_to(common)
    assert (here/relative).read_bytes()==f.read_bytes() or relative.as_posix()=='CarPlayClusterControls.jar',relative
  for f in (pkg/'payload/profiles'/mu/cluster).iterdir():
   if f.is_file():replacements['Custom/carplay_maps_vc/payload/profiles/'+mu+'/'+cluster+'/'+f.name]=f.read_bytes()
  # Generated manifest covers the complete integration file set, no private inputs.
  files=sorted(f for f in pkg.rglob('*') if f.is_file() and f.name!='SHA256SUMS.txt')
  (pkg/'SHA256SUMS.txt').write_bytes(''.join(sha(f.read_bytes())+'  '+f.relative_to(pkg).as_posix()+'\n' for f in files).encode('ascii'))
 for f in common.rglob('*'):
  if f.is_file():replacements['Custom/carplay_maps_vc/payload/common/'+f.relative_to(common).as_posix()]=f.read_bytes()
 for source,target in [('cpmap_common.sh','cpmap_common.sh'),('cpmap_install_790.sh','cpmap_install_790.sh'),('restore-carplay-maps.sh','cpmap_restore_core.sh')]:
  replacements['Toolbox/scripts/'+target]=(REPO/'src/toolbox'/source).read_bytes()
 replacements['Toolbox/GEM/carplay-maps-vc.esd']=(REPO/'src/toolbox/carplay-maps-vc.esd').read_bytes()
 notes=(REPO/'releases/toolbox-plaintext-v2.1.0.md').read_bytes()
 # This README sits at the SD root, not in the repository's releases folder.
 # Preserve usable documentation links instead of pointing outside the ZIP.
 external_notes=notes.replace(
  b'(../docs/',b'(https://github.com/omonob/MHI2-Carplay-Maps/blob/main/docs/')
 replacements['CARPLAY_MAPS_VC_README.md']=external_notes
 root=a.output/'release-assets';root.mkdir(parents=True)
 archive=root/('VW_SKODA_SEAT_CarPlay_Maps_VC_FULL_MQB_Toolbox_V4.2A_v'+version+'.zip')
 changed=[];kept=[]
 with zipfile.ZipFile(a.base_archive) as base,zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as out:
  assert len(base.namelist())==len(set(base.namelist()))
  for entry in base.infolist():
   path=PurePosixPath(entry.filename);assert not path.is_absolute() and '..' not in path.parts
   assert (entry.external_attr>>16)&0o170000!=0o120000
   before=base.read(entry);after=replacements.pop(entry.filename,before)
   if after!=before:
    assert entry.filename.startswith('Custom/carplay_maps_vc/') or entry.filename in ('Toolbox/scripts/cpmap_common.sh','Toolbox/scripts/cpmap_install_790.sh','Toolbox/scripts/cpmap_restore_core.sh','Toolbox/GEM/carplay-maps-vc.esd','CARPLAY_MAPS_VC_README.md'),entry.filename
    changed.append(entry.filename)
   else:kept.append(entry.filename)
   out.writestr(entry,after)
  for name,data in sorted(replacements.items()):
   assert name.startswith('Custom/carplay_maps_vc/payload/'),name
   out.writestr(name,data)
 with zipfile.ZipFile(a.base_archive) as base,zipfile.ZipFile(archive) as out:
  assert len(out.namelist())==len(set(out.namelist())) and out.testzip() is None
  assert not any('Standalone' in n or 'MU0033' in n for n in out.namelist())
  for name in kept:assert out.read(name)==base.read(name)
  for mu,cluster,brand in PROFILES:
   prefix='Custom/carplay_maps_vc/payload/profiles/'+mu+'/'+cluster+'/'
   for name in ('VCMenu.jar','VCInformation.jar','CarPlayClusterControls.jar','VCMirrorRate.jar','core.so'):assert out.read(prefix+name)
  files=len(out.namelist())
 digest=sha(archive.read_bytes())
 (root/'FULL_TOOLBOX_SHA256.txt').write_bytes((digest+'  '+archive.name+'\n').encode('ascii'))
 (root/'GITHUB_RELEASE_NOTES.md').write_bytes(external_notes)
 report=dict(version=version,published=False,base_sha256=BASE_SHA,archive_sha256=digest,profiles=7,files=files,unrelated_mqb_files='byte-identical',standalone_included=False,physical_information_acceptance=False,modified_owned_files=changed)
 (a.output/'package-audit.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
 print('V21_LOCAL_FULL_MQB_CANDIDATE_PASS',json.dumps(report),str(archive))
if __name__=='__main__':main()
