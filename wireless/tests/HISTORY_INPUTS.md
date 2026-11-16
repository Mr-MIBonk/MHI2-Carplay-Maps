# Reproducing the historical upgrade matrix

`test_upgrade_history.py` runs historical public installers before the new installer, then checks update, restore and entry removal. It intentionally does not contain the full old toolbox packages.

Required container paths:

- `/repo`: this source tree; `/sd`: the matching new SD_ROOT.
- `/history/HISTORY_FIXTURES.json`: `evidence/public-history-inputs.json` from this source tree.
- `/inputs/history/<tag>`: clean immutable Git archives for each tag listed in that JSON, preserving their paths. The first tag instead uses its verified release ZIP payload as `/inputs/history/toolbox-plaintext-v1.0.0/payload`.
- `/r1`: the prior local R1 SD_ROOT, used only to reproduce its migration defect and saved-phone conversion.
- `/old-manager`: the original independent CarPlay Maps SD_ROOT, used for exact old-manager retirement/restore tests.
- An empty output directory passed as the second argument. A host C compiler and standard POSIX commands are required.

The public repository is `https://github.com/omonob/VW-MHI2-Carplay-Maps`; the exact tag commits and first-release SHA-256 are recorded in the evidence JSON. Never substitute a moving branch. Obtain R1 and the old independent manager from their original handoffs if those private regression checks are needed.

Run with networking disabled and no vehicle paths or devices mounted. The tests build host helpers, use their own temporary filesystems, stub fixture `sync`, and reject unexpected live mount/service/reboot commands. Do not remove those boundaries to make a test pass.

The first public installer needed an existing navigation coexistence patch and classpath entry. The test explicitly prepares that historical prerequisite; this is not evidence that the first release supported a blank factory filesystem.

`WMAPS_TEST_FOCUS=smoke` selects two historical combinations but still exercises the R1 and risk cases. A full report must have the complete historical case count; a smoke result must not be relabeled as full coverage.
