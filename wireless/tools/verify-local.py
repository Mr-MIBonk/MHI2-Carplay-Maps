"""Read-only manifest/SWDL binding verifier. Does not execute target code."""
from pathlib import Path,PurePosixPath
import hashlib,re,sys,zipfile
root=Path(sys.argv[1]).resolve();seen=set()
for line in (root/'CONTENTS.sha256').read_text('utf-8').splitlines():
    digest,relative=line.split('  ',1);p=PurePosixPath(relative)
    assert re.fullmatch('[a-f0-9]{64}',digest) and not p.is_absolute() and '..' not in p.parts and '.' not in p.parts
    assert relative not in seen;seen.add(relative)
    file=root/relative;assert not file.is_symlink() and file.is_file() and root in file.resolve().parents
    assert hashlib.sha256(file.read_bytes()).hexdigest()==digest,relative
    if file.suffix=='.jar':
        with zipfile.ZipFile(file) as jar:assert jar.testzip() is None,relative
actual=set()
for file in root.rglob('*'):
    assert not file.is_symlink(),str(file)
    if file.is_file() and file!=root/'CONTENTS.sha256':actual.add(file.relative_to(root).as_posix())
assert actual==seen,('manifest coverage',sorted(actual-seen),sorted(seen-actual))
if (root/'metainfo2.txt').exists():
    meta=(root/'metainfo2.txt').read_text('ascii')
    final=root/'install/scripts/final/finalScript.sh';hashes=final.parent/'hashes.txt';esd=root/'install/esd/WirelessCarPlayMaps.esd'
    assert 'FinalScriptChecksum = "'+hashlib.sha1(final.read_bytes()).hexdigest()+'"' in meta
    assert 'CheckSum = "'+hashlib.sha1(esd.read_bytes()).hexdigest()+'"' in meta
    assert 'FileSize = "'+str(esd.stat().st_size)+'"' in meta
    assert 'CheckSum = "'+hashlib.sha1(hashes.read_bytes()).hexdigest()+'"' in meta
    assert 'FileSize = "'+str(final.stat().st_size+hashes.stat().st_size)+'"' in meta
print('LOCAL_READ_ONLY_MANIFEST_PASS files=%d'%len(seen))
