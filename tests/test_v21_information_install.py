"""Version-upgrade and ownership fixtures, never connects to a headunit."""
from pathlib import Path
import os
import subprocess
import tempfile
from test_toolbox_install import PROFILES,package,fixture,digest,run,navigation_payload,REPO

# Exact public G11-beta baseline, commit 90751cd7ce8db751083e403d1fa4b79b8c0241b4.
# Vendored fixture avoids changing Git safe.directory on a user's machine.
old_install=(REPO/'src/information/tests/pre-v21-installer.sh').read_bytes()
for mu,cluster,size in PROFILES:
 pkg=package(mu,cluster);payload=pkg/'payload';nav=navigation_payload(payload,mu,cluster)
 with tempfile.TemporaryDirectory(prefix='cpmap-v21-upgrade-') as tmp:
  root=Path(tmp);dio,lsd,jars=fixture(root,size,'stock',nav)
  ipod=root/'mnt/app/armle/usr/sbin/mm-ipod';original=(digest(dio),digest(lsd),digest(ipod))
  unrelated=root/'mnt/app/other-tool/keep';unrelated.parent.mkdir();unrelated.write_bytes(b'untouched')
  old=root/'old-v20.sh';old.write_bytes(old_install)
  assert 'CPMAP_INSTALL_OK' in run(old,root,payload,mu,cluster)
  assert not (root/'mnt/app/carplaymaps/stock_first/mm-ipod.real').exists()
  for iteration in range(2):
   assert 'CPMAP_INSTALL_OK' in run(pkg/'install.sh',root,payload,mu,cluster)
   saved=root/'mnt/app/carplaymaps/stock_first/mm-ipod.real'
   assert digest(saved)==original[2]
   assert 'CPMAP_PUBLIC_V21_IPOD_WRAPPER' in ipod.read_text()
   assert digest(jars/'VCInformation.jar')==digest(payload/'profiles'/mu/cluster/'VCInformation.jar')
   assert lsd.read_text().count('VCInformation.jar')==1
  run(pkg/'restore-carplay-maps.sh',root)
  assert original==(digest(dio),digest(lsd),digest(ipod))
  assert not (jars/'VCInformation.jar').exists()
  assert unrelated.read_bytes()==b'untouched'
 print('V21_OLD_VERSION_UPGRADE_REPEAT_RESTORE_PASS',mu,cluster,flush=True)
 with tempfile.TemporaryDirectory(prefix='cpmap-v21-foreign-') as tmp:
  root=Path(tmp);dio,lsd,jars=fixture(root,size,'stock',nav)
  ipod=root/'mnt/app/armle/usr/sbin/mm-ipod';ipod.write_bytes(b'#!/bin/sh\nexec other-private-helper "$@"\n')
  before=(digest(dio),digest(lsd),digest(ipod))
  output=run(pkg/'install.sh',root,payload,mu,cluster,ok=False)
  assert 'ORIGINAL_MM_IPOD_MISSING_OR_UNKNOWN_WRAPPER' in output
  assert before==(digest(dio),digest(lsd),digest(ipod))
  assert not (root/'mnt/app/carplaymaps').exists()
 print('V21_FOREIGN_MM_IPOD_PROTECTED_PASS',mu,cluster,flush=True)
print('V21_INFORMATION_INSTALL_OWNERSHIP_PASS profiles=7')
