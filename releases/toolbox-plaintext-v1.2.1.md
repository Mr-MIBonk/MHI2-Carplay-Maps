# CarPlay Maps VC v1.2.1 MQB installer fix

Author: [omonob（汽车大玩家）](https://github.com/omonob)

## English

This release fixes a valid navigation prerequisite JAR being rejected as `NAVIGATION_PAYLOAD_INVALID`. **The complete MQB Toolbox is the only supported distribution; M.I.B. automatic-flashing packages are withdrawn.**

The old installer searched binary JAR data directly with a text utility. The fixed installer checks the bundled patch's POSIX checksum and size, then converts printable archive strings into text before class inspection. It does not depend on GNU binary-grep behavior or require a separate patch download.

- Missing prerequisite: install the included patch and one startup reference automatically.
- Existing complete patch, including a merged metadata JAR: preserve the file and avoid duplicate startup entries.
- Damaged standalone navigation JAR owned by this project: repair during reinstall.
- Corrupt bundled SD-card patch: stop before factory-file changes and ask for a fresh extraction.
- Unknown incomplete CarPlay overrides from other projects: stop for review rather than overwrite unrelated mapping code.

Download only `VW_SKODA_ALL_CarPlay_Maps_VC_FULL_MQB_Toolbox_V4.2A.zip`, verify `FULL_TOOLBOX_SHA256.txt`, and replace the old SD files with the archive contents at the card root. Select 791 or 790 under `Customization → CarPlay Maps VC`. The compatibility matrix remains MU1102 and MU1367 with 791/790, plus MU1440 Skoda with 790 only. After `CPMAP_INSTALL_OK`, perform a full manual reboot and test CarPlay. The installer never reboots automatically.

All video/runtime binaries, the 791 40 FPS optimization, 790 display parameters and steering controls remain byte-identical to v1.2.0. Only one installer changes inside the ZIP. No music/navigation metadata mapping, startup animation, device-bound encryption or watchdog is added. Attribution, licensing, disclaimers and separate optional paid-support terms remain unchanged.

Host-side tests cover all five firmware/cluster combinations, first/repeat install, patch preservation/installation/repair, factory restore, corrupted payloads, activation rollback and the final archive. A text-only grep shim reproduces the old archive's failure; it models target utility behavior and is not a substitute for live testing of every unit. Historical tags and notes are retained for reference, not installation.
