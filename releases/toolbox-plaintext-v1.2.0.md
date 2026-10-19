# CarPlay Maps VC v1.2.0

> Historical record: the navigation prerequisite check is superseded by [the v1.2.1 MQB fix](./toolbox-plaintext-v1.2.1.md). M.I.B. automatic-flashing packages are withdrawn; the installation routes below are historical, not current instructions.

Author: [omonob（汽车大玩家）](https://github.com/omonob)

## English release notes

This release brings the previously exercised **791 40 FPS display optimization** into the free open-source toolbox distributions. The 800x480 canvas and navigation-centered layout remain unchanged. It applies to the existing MU1102 and MU1367 791/794 AID12.3 profiles; compatibility is not expanded.

### Changes

- Optimizes the 791 video display parameters while preserving resolution, navigation placement and controls.
- Corrects stale top-level 30 fps documentation: the existing 791 core already advertised 40 fps.
- Updates the two fixed 791 M.I.B. distributions and their two 791 cores in the universal MQB toolbox.
- Includes a reproducible parameter transformation, offline tests and SHA-256 checksums.

40 fps is a negotiated ceiling, not a measured display-consumption guarantee. This release does not promise zero latency or elimination of corruption in every condition.

### Unchanged

All three 790 M.I.B. archives are retained byte for byte from v1.1.0. The 790 cores/layout, video bridge, supervisor, authentication components, navigation prerequisite, steering controls, firmware gates and installation entry points are unchanged. MU1440 Skoda remains 790-only with the existing roller zoom support.

No music/navigation metadata, lane guidance, startup animation, NAVDB operations, device-bound encryption or 250 ms guard is added. The free open-source scope and separate optional paid-support information remain unchanged.

### Download and installation

Use the complete universal MQB toolbox or the complete M.I.B. archive matching your exact MU/cluster. Extract directly to the SD-card root. MQB still requires a full manual reboot after successful installation; M.I.B. retains its SWDL flow. Do not mix profiles. Verify downloads against `FULL_TOOLBOX_SHA256.txt`. Earlier releases remain available.

### Validation boundary

The optimization comes from earlier development tests. This packaging pass checks core hashes and the parameter-change scope, unchanged unrelated runtime files, firmware install/reinstall/factory-restore fixtures, full archive reads and checksums. Offline packaging validation does not claim a new live test on every MU, iPhone and map-app combination.
