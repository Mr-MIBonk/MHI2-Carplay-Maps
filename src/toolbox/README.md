# Navigation prerequisite integration

`install-carplay-maps.sh` is the canonical installer copied to the five standalone profiles and the single complete MQB Toolbox archive. M.I.B. automatic-flashing distributions and their build entry points are withdrawn. `restore-carplay-maps.sh` restores the separate navigation JAR and is used by the standalone packages and MQB restore entry.

The navigation prerequisite is the unchanged upstream MQB Toolbox `Modifications/Lsd/NavActiveIgnore.jar`, distributed here as `CarPlayNavigationPatch.jar`:

- Upstream: https://github.com/jilleb/mib2-toolbox/blob/master/Modifications/Lsd/NavActiveIgnore.jar
- Size: 27,179 bytes.
- SHA-256: `a4803f7fd008152aca19d4e00b8b45789ac18dccf4cf405d129ce02297982ffe`.
- Nine classes, upstream classfile major 46 (Java 1.2); no recompilation or bytecode conversion.

v1.2.2 uses the bundled `cpmap-nav-check` C program, not target `cksum`, `tr`, `strings`, `unzip` or binary `grep`. `--payload` pins the unchanged upstream JAR to length **27179**, standard CRC32 **b6c892dc**, valid central/local ZIP directory records and the seven required class entries. The whole-release SHA-256 protects distribution integrity; CRC32 here detects accidental SD/copy corruption, not adversarial substitution. `--has-patch` inspects actual ZIP records of an existing active JAR; class names appearing in an arbitrary text file are not enough. Existing third-party class bytecode is not decompressed or semantically audited. `--same` compares staged/installed bytes directly.

A filename alone is not sufficient. This is a prerequisite check, not screen-message recognition or a claim that the running HMI has been inspected. Complete active patches are kept, even if the unused SD navigation JAR is absent/damaged. If missing or incomplete, prepend the standalone bundled patch before third-party classpaths without rewriting their archives. The seven navigation coexistence classes then take precedence; unrelated classes remain available. A damaged project-owned navigation JAR is repaired. An actual missing/damaged required SD patch still stops before activation.

The installer merges standard existing `-Xbootclasspath/p:` assignments, removes duplicate owned references, or creates one before `$J9` on stock scripts. Commented references do not trigger duplicate failures. Unknown nonstandard startup layouts are not guessed. Original startup state and navigation-JAR presence are retained for factory restoration. Failed activation uses an immediate pre-update snapshot so an already-installed version is not replaced by an older factory baseline during rollback.

Only MU1440 selects the optional profile-specific roller control JAR. All other profiles retain their previous control JAR. Firmware gates, native libraries, display parameters and MQB installation entry points are unchanged.

Build the small libc-only checker with `tools/Build-NavChecker.ps1`, using the approved open GCC `qnx65-armv7:4.9` development image. It does not invoke original SDK/qcc/licensing tools. The binary targets ARMv7 EABI5, `/usr/lib/ldqnx.so.2`, and only `libc.so.3`. The script propagates the helper and canonical installer to all five standalone profiles. From RCC, the installer executes the checker on MMX using `on -f mmx`; a host-only override requires an explicit fixture root. The helper never writes device files, reads identities, or contacts the network.

`tests/test_qnx_navigation.py` reproduces the old published archive's failure with unavailable target tools/text-only grep and tests the final ZIP. `tests/test_nav_checker.py` checks malformed archives and undefined behavior. `tests/test_navigation_repair.py` tests optional SD patches, partial overrides, duplicate/commented classpath lines and MMX dispatch. These are host regressions, not live QNX execution. `tools/Build-MQBRelease.ps1` builds only the MQB hotfix from the exact v1.2.1 archive; `tests/test_mqb_release.py` permits only one changed installer and one new checker, preserving every existing runtime payload.

导航补丁来自工具箱原有组件，按原字节分发，不加入信息映射功能。检测依据是启动加载行和 JAR 内补丁类，不是识别仪表提示截图。已有完整补丁保持原样；补装使用单独文件，并在原厂恢复时撤销。MU1440 以外的方向盘控制 JAR 与原版保持一致。
