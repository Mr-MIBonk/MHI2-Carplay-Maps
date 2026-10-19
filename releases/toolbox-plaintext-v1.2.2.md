# CarPlay Maps VC v1.2.2 - navigation prerequisite installer repair

Author: [omonob（汽车大玩家）](https://github.com/omonob)

## English

Fixes repeated false navigation-prerequisite failures. Replace the complete old SD-card contents; do not keep running an older installer.

- After exact supported-firmware/profile identification, handle the navigation prerequisite automatically and continue installation without another confirmation.
- Keep a complete active patch. Do not demand an unused SD navigation JAR before checking the installed patch.
- Automatically supply the separate bundled patch if missing, partially installed, or referenced but absent. Other projects' JAR files are not rewritten; when completion is necessary, our seven navigation-coexistence classes load first.
- Repair a damaged project-owned navigation JAR, preserve repeat-install idempotency and factory restoration.
- No dependency on target `cksum`, `tr`, `strings`, `unzip`, or binary-grep behavior. A small read-only ARM helper validates ZIP directory records, required classes and bundled-payload integrity directly.
- Unsupported firmware, genuinely missing/damaged required files, write failures and unsupported startup layouts still stop with an explicit error. Failures are not hidden or reported as success.

Only the complete **MQB Toolbox** archive is provided: `VW_SKODA_ALL_CarPlay_Maps_VC_FULL_MQB_Toolbox_V4.2A_v1.2.2.zip`. Verify `FULL_TOOLBOX_SHA256.txt`, extract to the SD root, and select 791 or 790 under `Customization → CarPlay Maps VC`. Perform a full manual reboot after `CPMAP_INSTALL_OK`; no automatic reboot is performed.

Compatibility remains MU1102/791, MU1102/790, MU1367/791, MU1367/790 and MU1440 Skoda/790. The 791 40 FPS optimization, 790 parameters, steering controls and all existing runtime binaries remain unchanged. No metadata mapping, startup animation, device-bound data or watchdog is added. Attribution, licensing, disclaimers and separate paid-support terms remain intact.

Host regression tests cover missing target utilities, absent/complete/merged/partial/damaged patches, installation/reinstallation/restoration, activation rollback and all five profiles extracted from the final archive. The checker is host-tested with undefined-behavior checks and cross-built for the QNX 6.5 ARM ABI. This is not a claim of fresh live-vehicle execution.
