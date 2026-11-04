# V2.1 CarPlay information (experimental)

This module adapts the author's existing V53.3 information branch. It is **not**
the video renderer, not Android Auto information, and not the private V7.x
installer. A successful offline test is not physical phone/cluster acceptance.

## Runtime and switches

The native iAP2 hook subscribes to route/maneuver/lane information, preserving
the stock identification callback. Its indexed cache retains the existing
disconnect replay guard and lane-association behaviour. The artwork consumer
reads the stock artwork-transfer interface, bounds JPEG input before decoding,
and atomically publishes one 128×128 cover plus its track signature.

`VCInformation.jar` sends information through the existing BAP/cluster path.
The menu's four remembered choices are All enabled, Navigation off, Music off
and All off. Navigation/video ownership and music-information permissions are
separate. Navigation shutdown clears once; re-enabling triggers current data.
Music shutdown clears CarPlay artwork once, and re-enabling permits the same
unchanged song cover to be emitted again. OEM radio/BT artwork paths are not
filtered. The optional historical local-Bluetooth listener is not started.

DSI/BAP dispatch uses the original ASL thread queue, coalesced to at most five
pending jobs. The fifth slot registers the music-information listener without
blocking ASL or reporting a queued registration as successful. Its real result
is consumed only by the matching original HsmTarget. This corrects wrong-thread
diagnostics independently observed at startup and while switching the new menu
in the real G11 firmware emulator. It does not
claim an unmeasured improvement in physical playback latency.

The public helper directory is `/mnt/app/carplaymaps/information`. Logs,
cover and fixed-size caches are under `/ramdisk/cpmap-info`, not a growing
persistent log directory. The stock mm-ipod executable is saved exactly before
its owned wrapper is installed. Restore/reinstall/commit failure preserve that
first original. An unknown pre-existing mm-ipod wrapper is rejected rather than
silently replacing another project's modifications.

## Exact target adaptation

Five builds use the exact original MU1102, MU1367, MU1440, MU1433 and MU1447 JARs.
G11 BAPString void/fluent differences are adapted with preserved operand-stack
results. MU1433 additionally needs the older nested types and navigation API.
European builds use the unit's default timezone, including its DST rules, for
both ETA-display and BAP ETA calculations; MU1102 retains its CN timezone.
The installer does not change the unit's clock or timezone.

Eight overlapping map/coexistence classes are excluded: the separately audited
map JAR retains ownership. DSIHandler is not in the historical information
patch, so the menu's existing navigation-ownership hook is retained separately.
No MU1409 binary or full OEM JAR is shipped.

## Reproducible source and attribution

`instruction-source` is compilable ASM instruction source exported from the
author-supplied historical V53.3 MU1102 patch, SHA-256
`efd234cc94cafb41f0614a8f9344013dbf3a8758871f093b0679c4edb9e01861`.
It preserves the historical branch's class names and instructions rather than
pretending a decompiler's invalid Java output is buildable source. There are
49 historical classes, 41 after map isolation, plus two new dispatch helpers
and five byte-identical shared menu-policy/persistence classes in the result.
`VerifyHistoricalSource` can compare the instruction trace with the privately
supplied historical patch; debug attributes and stack-map serialization alone
are ignored. The historical binary is not required to build from this source.

Native replay/cache sources retain the existing V53.2.2 branch. The RGB contain
resize kernel retains its luka-dev attribution. The bundled stb headers use
the MIT option, with upstream hashes and the small unsigned-JPEG-bit-buffer
fix recorded in [vendor provenance](native/vendor/PROVENANCE.md). The surrounding
project remains GPLv3. See also the [community acknowledgements](../../docs/COMMUNITY_V2_1_CREDITS.md).

## Build and offline checks

1. Build all five exact menu targets with `tools/Build-V2Menu.ps1`.
2. Run `tools/Build-V21Information.ps1` with all five `-MUxxxxStockJar` paths,
   `-MenuBuild` and a fresh `-Output`. JDK 17 is the build tool, not the target VM.
   Each resulting JAR is API-audited, loaded with verification, tested against
   winter/summer ETA cases and injected hot-switch fixtures. The same build
   also executes the concurrent dispatcher test with held/active consumers,
   simultaneous producers and injected enqueue failures. These fixture-backed
   checks do not replace real QNX/phone/MOST acceptance.
3. Run `tools/Build-InformationNative.sh` in the existing Neutrino GNU container.
   Mount this repository as `/src`, and private extracted iAP2 ABI fixtures for
   the five exact firmwares as `/fixtures:ro`. Only the generated helpers are
   output; original client/stream libraries are used for linking/audit but are
   never copied into the release. Host sanitizers exercise decoder bounds,
   failed writes, crash-safe locking and four synthetic association/replay logs.
4. `tools/Stage-V21Profiles.ps1` stages seven supported combinations. Linux
   fixtures `test_v2_install.py`, `test_v21_information_install.py`,
   `test_toolbox_install.py` and `test_navigation_repair.py` check install,
   upgrade, missing navigation repair, failure rollback, restore and ownership.

Use a new build folder; never overwrite an installation package being used by
another window. Keep real G11 phone, day/night and restart-card acceptance
separate from these offline tests.
