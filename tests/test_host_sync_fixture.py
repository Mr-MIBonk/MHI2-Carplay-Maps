"""Test-only scope and fail-closed checks, not vehicle durability tests."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile
from host_sync_fixture import scoped_sync_commands

if sys.platform=='win32':
    print('HOST_SYNC_SCOPE_NOT_USED_ON_WINDOWS')
else:
    with tempfile.TemporaryDirectory(prefix='cpmap sync scope ') as tmp:
        root=Path(tmp)
        commands=scoped_sync_commands(root)
        script=commands/'sync'
        (root/'pending-data').write_bytes(b'host fixture flush')
        environment={**os.environ,'CPMAP_TEST_ROOT':str(root)}
        assert subprocess.run([str(script)],env=environment,timeout=10).returncode==0
        assert subprocess.run([str(script),'unsupported'],env=environment,timeout=10).returncode==92
        assert subprocess.run([str(script)],env={**environment,'CPMAP_TEST_ROOT':''},timeout=10).returncode==93
    print('HOST_SYNC_FIXTURE_SCOPE_PASS real_syncfs=1 no_vehicle_claim=1')
