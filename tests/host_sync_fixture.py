"""Flush only a host fixture filesystem, never every WSL/host mount.

The shipped QNX scripts retain their normal sync call. This test-only command
executes GNU sync -f on the temporary fixture filesystem (not a no-op), avoiding
unrelated FUSE/network filesystems in other WSL mount namespaces. It does not
model vehicle flash durability or count as QNX execution.
"""
from pathlib import Path
import sys


def scoped_sync_commands(root):
    if sys.platform == 'win32':
        return None  # Keep the already validated Git Bash path unchanged.
    root = Path(root).resolve(strict=True)
    commands = root / '.host-sync-commands'
    commands.mkdir(exist_ok=True)
    script = commands / 'sync'
    script.write_text('#!/bin/sh\n'
                      '[ "$#" -eq 0 ] || exit 92\n'
                      '[ -n "$CPMAP_TEST_ROOT" ] && [ -d "$CPMAP_TEST_ROOT" ] || exit 93\n'
                      'exec /bin/sync -f "$CPMAP_TEST_ROOT"\n')
    script.chmod(0o755)
    return commands
