# CarPlay Maps VC v1.1.0

> Historical record: use [the v1.2.1 MQB fix](./toolbox-plaintext-v1.2.1.md). The old navigation check has a target-compatibility defect; M.I.B. automatic-flashing packages are withdrawn. The text below is historical only.

## English release notes

This release integrates the navigation prerequisite patch and adds MU1440 Skoda steering-wheel roller zoom. All other behavior remains as in v1.0.0. Downloads still contain a complete MQB Toolbox and five complete, profile-locked M.I.B. Toolbox distributions, ready to extract to the SD-card root.

### Automatic navigation patch

Without the prerequisite patch, starting CarPlay navigation may display “activated on the mobile device” and close the cluster map. The installer now checks Java boot loading and patch classes:

- Installs a separate `CarPlayNavigationPatch.jar` and its boot entry when missing.
- Preserves an existing complete patch, including JARs merged with metadata mapping.
- Supports factory `lsd.sh` without an existing `-Xbootclasspath/p:` patch entry.
- Avoids duplicate entries on reinstall; restoration removes only the patch added by this project and preserves pre-existing patches.
- Stops safely for incomplete third-party CarPlay class overrides instead of overwriting other functions.

This patch removes the external-navigation map shutdown restriction. It does not add navigation text, maneuver guidance, music metadata or cover art.

### MU1440 Skoda roller control

- One upward detent: zoom in.
- One downward detent: zoom out.
- Roller press: not mapped.
- MU1440 Skoda remains restricted to the 790 cluster profile. Other firmware control JARs are unchanged.

The phone map app must accept zoom commands. Amap was verified; Baidu Maps does not support this operation.

### Installation and verification

Entry points are unchanged. Perform a full reboot after a successful MQB installation; M.I.B. retains its normal SWDL reboot workflow. Check the firmware and cluster, verify downloads against `FULL_TOOLBOX_SHA256.txt`, and never mix M.I.B. profiles.

Offline tests cover all five profiles: first install, reinstall, factory restoration, existing/missing/damaged navigation patches and wrong-firmware stops. Roller event replay tests also passed. Each profile still needs independent live validation; offline results are not a claim that every combination has been tested on a vehicle for this release.

The 30 fps display baseline, existing 790 navigation position, video transport, toolbox entry points, free open-source scope and separate paid-support contents are unchanged.

## Third party component notice

The bundled navigation JAR is copied byte-for-byte from the navigation patch already distributed with the [MQB MIB2 Toolbox](https://github.com/jilleb/mib2-toolbox). It is not replaced by a music/navigation metadata component. Upstream toolbox files and license notices remain intact; the repository GPLv3 notice does not relicense third-party or vendor components.

MU1440 roller source, compilation stubs and host tests are included under `src/steering-mu1440`. Stubs are build-only and are not packed into the runtime JAR. Navigation detection and installation source is under `src/toolbox`, with offline tests under `tests`.
