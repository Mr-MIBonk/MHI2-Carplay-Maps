"""Independent final-asset inspection; no network or headunit operations."""
from pathlib import Path,PurePosixPath
import argparse,hashlib,io,json,struct,zipfile

REPO=Path(__file__).resolve().parents[1]
PROFILES=[('MU1102','791','MU1102'),('MU1102','790','MU1102'),
 ('MU1367','791','MU1367'),('MU1367','790','MU1367'),
 ('MU1440','790','MU1440_SKODA'),('MU1433','790','MU1433_SKODA'),('MU1447','790','MU1447_SEAT')]
MAPPED_CLASS_EXCLUSIONS=(
 'de/esolutions/fw/comm/dsi/carplay/impl/DSICarplayProxy.class',
 'de/esolutions/fw/dsi/androidauto2/DSIAndroidAuto2Dispatcher.class',
 'de/esolutions/fw/dsi/androidauto/DSIAndroidAutoDispatcher.class',
 'de/vw/mib/asl/internal/androidauto/target/NavigationListener.class',
 'de/vw/mib/asl/internal/androidauto/target/RequestHandler.class',
 'de/vw/mib/asl/internal/carplay/common/CarPlayModeHandling.class',
 'de/vw/mib/asl/internal/exboxm/api/impl/guidance/ExboxGuidanceManagerImpl.class',
 'de/vw/mib/asl/internal/navigation/api/impl/ASLNavigationServices.class')
def digest(data):return hashlib.sha256(data).hexdigest()
def archive(path):
 with zipfile.ZipFile(path) as z:
  assert z.testzip() is None
  assert len(z.namelist())==len(set(z.namelist()))
  for entry in z.infolist():
   p=PurePosixPath(entry.filename)
   assert not p.is_absolute() and '..' not in p.parts and '\\' not in entry.filename
   assert (entry.external_attr>>16)&0o170000!=0o120000
  return {n:z.read(n) for n in z.namelist() if not n.endswith('/')}
def jar(data):
 with zipfile.ZipFile(io.BytesIO(data)) as z:
  assert z.testzip() is None and len(z.namelist())==len(set(z.namelist()))
  classes={n:z.read(n) for n in z.namelist() if n.endswith('.class')}
  for n,b in classes.items():
   assert b[:4]==b'\xca\xfe\xba\xbe',n
   assert 46<=int.from_bytes(b[6:8],'big')<=48,(n,'unsupported target class version')
  return classes
def arm_elf(data):
 assert data[:7]==b'\x7fELF\x01\x01\x01' and int.from_bytes(data[18:20],'little')==40
 # No host temporary link path or private firmware path may be a target RPATH.
 assert b'/tmp/cpmap-private-' not in data and b'/fixtures/' not in data
def main():
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('--candidate',type=Path,required=True);p.add_argument('--baseline',type=Path,required=True)
 a=p.parse_args();old=archive(a.baseline);new=archive(a.candidate)
 assert digest(a.baseline.read_bytes())=='86aada968b035710696f7e4a7cc7a2e13bb83fb4e71ad3c4374e40ecb193cbae'
 assert set(old)<=set(new),'Unrelated historical MQB entries removed'
 owned_prefix='Custom/carplay_maps_vc/'
 owned_names={'Toolbox/scripts/cpmap_common.sh','Toolbox/scripts/cpmap_install_790.sh',
  'Toolbox/scripts/cpmap_restore_core.sh','Toolbox/GEM/carplay-maps-vc.esd','CARPLAY_MAPS_VC_README.md'}
 preserved=0
 for name,b in old.items():
  if not name.startswith(owned_prefix) and name not in owned_names:
   assert new[name]==b,name;preserved+=1
 for name in set(new)-set(old):assert name.startswith(owned_prefix),name
 assert not any('Standalone' in n or 'MU0033' in n for n in new)
 common='Custom/carplay_maps_vc/payload/common/'
 assert new[common+'install-carplay-maps.sh']==(REPO/'src/toolbox/install-carplay-maps.sh').read_bytes()
 assert new[common+'restore-carplay-maps.sh']==(REPO/'src/toolbox/restore-carplay-maps.sh').read_bytes()
 assert new['Toolbox/scripts/cpmap_restore_core.sh']==new[common+'restore-carplay-maps.sh']
 assert new['Toolbox/GEM/carplay-maps-vc.esd']==(REPO/'src/toolbox/carplay-maps-vc.esd').read_bytes()
 assert b'CarPlay Maps VC 2.1' in new['Toolbox/GEM/carplay-maps-vc.esd']
 assert b'olli991' in new['CARPLAY_MAPS_VC_README.md'] and b'octacon84' in new['CARPLAY_MAPS_VC_README.md']
 assert b'(../docs/' not in new['CARPLAY_MAPS_VC_README.md'],'SD-root README must not link outside the archive'
 assert b'https://github.com/omonob/MHI2-Carplay-Maps/blob/main/docs/COMMUNITY_V2_1_CREDITS.md' in new['CARPLAY_MAPS_VC_README.md']
 for filename in ('mm-ipod.wrapper.sh','cpvc-rgd-bootstrap.sh','artwork-launcher.sh'):
  assert new[common+'information/'+filename]==(REPO/'src/information/runtime'/filename).read_bytes()
 for filename in ('libcpvc-rgd.so','cpvc-rgd-sync','carplay_artwork_helper'):
  data=new[common+'information/'+filename];arm_elf(data)
  assert data==(REPO/'build-information-native'/filename).read_bytes()
 for name,b in new.items():
  if name.startswith(common) and name.endswith('.sh'):assert b'\r' not in b,name
  if name.startswith(owned_prefix):
   for private in (b'BEGIN PRIVATE KEY',b'BEGIN RSA PRIVATE KEY',b'C:\\Users\\omonob',b'D:\\VW\\'):
    assert private not in b,(name,'private material detected')
 results=[]
 for mu,cluster,brand in PROFILES:
  directory=REPO/brand/('791_AID12.3' if cluster=='791' else '790_AID10.5')
  target=directory/'payload/profiles'/mu/cluster
  prefix=owned_prefix+'payload/profiles/'+mu+'/'+cluster+'/'
  for f in target.iterdir():
   if f.is_file():assert new[prefix+f.name]==f.read_bytes(),prefix+f.name
  assert (directory/'install.sh').read_bytes()==new[common+'install-carplay-maps.sh']
  classes=jar(new[prefix+'VCInformation.jar']);assert len(classes)==48
  assert not set(MAPPED_CLASS_EXCLUSIONS)&set(classes)
  assert not any(n.endswith('/DSIHandler.class') for n in classes)
  menu=jar(new[prefix+'VCMenu.jar'])
  assert 'org/omonob/cpmap/ui/NativeSelectorGeometry.class' in menu,'Native selector bounds fix missing'
  assert b'native_padding=' in menu['org/omonob/cpmap/ui/IndependentMenuViewBridge.class'],'Stale compact-menu layout'
  assert b'native_option_item_gap=' in menu['org/omonob/cpmap/ui/IndependentMenuViewBridge.class'],'Native G11 item-gap audit missing'
  shared={n for n in classes if n.startswith('org/omonob/cpmap/ui/')}
  assert len(shared)==5
  for name in shared:assert classes[name]==menu[name],(mu,name)
  assert 'org/omonob/cpmap/information/InformationAslDispatch.class' in classes
  arm_elf(new[prefix+'core.so'])
  for line in (directory/'SHA256SUMS.txt').read_text().splitlines():
   checksum,relative=line.split('  ',1)
   assert digest((directory/relative).read_bytes())==checksum,(mu,relative)
  results.append(mu+'/'+cluster)
 aiddirs={n.split('/')[4:6][0]+'/'+n.split('/')[4:6][1] for n in new if n.startswith(owned_prefix+'payload/profiles/') and n.endswith('/core.so')}
 assert aiddirs==set(results),aiddirs
 record=a.candidate.parent/'FULL_TOOLBOX_SHA256.txt'
 assert record.read_text().strip()==digest(a.candidate.read_bytes())+'  '+a.candidate.name
 print('V21_CANDIDATE_ARCHIVE_PASS',json.dumps(dict(profiles=results,unrelated_mqb_files_preserved=preserved,information_classes_per_target=48,matching_shared_policy_classes=5,crc=True,sha=True,privacy_scan=True,vehicle_acceptance=False)))
if __name__=='__main__':main()
