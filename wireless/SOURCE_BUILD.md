# Local source build notes

This tree contains owned integration sources, not complete source for every R26/OEM dependency. No QNX license, SDK, extracted stock HMI SDK JAR, private device identity, password, K1/K2, or pairing database is included.

## Native overlays

Use a properly provided ARM QNX EABI5 toolchain and its EGL/GLES libraries. This candidate was built offline with the existing locally reconstructed GNU 4.9.4 compiler, not by bypassing `qcc` licensing or sharing a QNX activation file. Native outputs were checked for 32-bit little-endian ARM EABI5 and `/usr/lib/ldqnx.so.2` for executables.

On Linux, with a new output directory:

```sh
QNX_CC=/path/to/arm-unknown-nto-qnx6.5.0eabi-gcc sh tools/build-native.sh /path/to/SD_ROOT /tmp/wmaps-native-build
python3 tools/verify-local.py /path/to/SD_ROOT
python3 tests/test_standalone.py /path/to/SD_ROOT /tmp/wmaps-fixture-results
```

The last test uses synthetic filesystems only, builds host versions of three helper programs, and installs fixture-local no-op `sync` commands. It never runs a QNX executable, touches a headunit, or flushes the host VM. Do not remove that fixture isolation. Tests requiring an actual loader/GPU/phone are a separate acceptance step, not implied by these commands.

Do not directly swap a rebuilt library into the supplied package: receiver wrappers pin hashes, and the SD manifest and SWDL checksums are coupled. Reassemble to a new candidate and run the transaction tests again. The public V2.1 core is shipped as frozen profile binaries and accompanied by its source; its original specialized core build configuration is distinct from the native overlay build above.

## Java menus

`src/menu-mu1102` contains the active MU1102 overlay sources; `src/vc-menu/src` contains common/native-model sources and the generic wired-only menu. They compile against exact OEM HMI APIs, with J9-compatible class version 46. They do not contain the stock OEM SDK classes. The five SDK hashes from the actual offline build are recorded under `evidence/java-sdk-profiles.json`. Never compile one firmware against a similar-looking SDK and assume ABI compatibility.

`src/wireless-boot/WirelessMapsBootstrap.java` is a connection-policy-only OSGi activator; it must not be replaced by the old information bootstrap. R26's CP/SI/Touch OEM patch JARs remain frozen dependencies. The sound hooks were removed from the native OEM view handler; startup-animation classes and information-menu fallback classes were removed from the built menus. The remaining OEM glue in the runtime JAR is an overlay dependency, not newly authored source.

## Removed features and preserved compatibility names

No V53 metadata consumer, song/cover normalizer, navigation-information publisher, welcome sound, or cluster startup-animation asset is installed. Reading a current navigation route solely to choose a video card/view area is not an information-to-cluster publisher.

Names such as `v75_*`, an SDK codec flag, or a receipt/temporary-file ABI can remain for compatibility with frozen R26 components. Product/maintainer declarations are omonob; blindly renaming functional flags or another author's actual notice is not a valid attribution cleanup.

`2026-08-29` is the requested owned-wireless-source date only. Build/test reports retain their actual dates. The retained R2 reports describe pre-publication checks. Publication checks are recorded separately; neither report is real-phone or real-vehicle acceptance.
