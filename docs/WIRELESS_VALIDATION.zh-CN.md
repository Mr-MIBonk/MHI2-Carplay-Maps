# Wireless CarPlay Maps V3.1 验证记录 / Validation

## QNX 功能脚本兼容修订 / QNX feature-script correction

MU1367 实机已确认管理入口注册成功及 `LOCAL_MANIFEST_PASS files=129`，但功能脚本第 21 行的 `hash(){...}` 在 QNX ksh 默认别名环境下解析失败。QNX 6.5 SDK 自带 PD KSH 重现了相同的行号和错误；此前 Linux sh 事务测试未覆盖这项解析行为。本次仅为该函数及其全部调用增加专用前缀，作者元信息原字节不动；全部其他安装脚本、原生程序和菜单载荷不变。隔离测试改用 SDK PD KSH 作为 `/bin/sh` 执行事务，仍不等于实机 QNX ARM 安装验收。

MU1367 hardware confirms manager registration and a valid manifest, but the line-21 `hash(){...}` definition fails under the default QNX ksh alias environment. The QNX 6.5 SDK's PD KSH reproduces the same line/error. Earlier Linux-sh transaction tests missed this parser behavior. Only the function and its calls gain a project prefix; author metadata and every other installer/native/menu payload remain unchanged. Isolated regressions now use SDK PD KSH as `/bin/sh`; this is still not QNX ARM hardware-installation acceptance.

历史状态构造与候选执行分开：旧 R1／R2／V3.0 包也含同名函数，直接用 SDK ksh 执行会在构造旧状态时失败。因此历史输入只沿用原来的 Linux sh 生成模拟旧状态，不把旧包因此认定为 QNX 可安装；当前候选的入口、功能安装、更新、恢复与卸载继续使用 SDK ksh。未跳过候选用例，也未修改历史归档文件。

Historical state construction is separate from candidate execution. Old R1/R2/V3.0 packages share the alias collision, so their original Linux-sh fixtures seed synthetic prior states; that is not QNX-installability evidence for those packages. Candidate/current-manager install, update, restore and uninstall still use SDK ksh. Candidate cases are not skipped and historical archives are not modified.

## 安装包装修订说明 / Installer-metadata correction

上一轮检查错误地把“与旧包一致”当作签名头有效的证明，也误把 `hashes.txt` 单文件 SHA-1 当成这套安装目录的校验。作者补充说明使用 7-Zip 对整个收尾目录计算 `SHA1 for data`，包含脚本和清单；本地独立复算已得到完全一致的 `f98e4c9162f089363b6a8d6c92cd40112ba902a1`。不能用之前的目录路径／组名推导覆盖作者提供的原文件。本次保留作者元信息的全部原始字节，并重新核对 CRLF、头部实际校验、目录计算、文件内容、完整清单及打包结果；我们的原车更新入口仍待实机复测。

The previous audit treated a prefix matching the earlier package as proof of validity and incorrectly used the SHA-1 of `hashes.txt` alone for this directory. The author clarified 7-Zip's directory `SHA1 for data`, including the script and hash list. Independent local calculation reproduces `f98e4c9162f089363b6a8d6c92cd40112ba902a1`. The author's metadata bytes, CRLF, descriptors and labels are now preserved; raw signed-header integrity, directory calculation, files, manifests and ZIP contents are checked separately. Our native-update hardware retest remains pending.

V3.1 为试验预发布。新增 MU1367/790、MU1367/791、Škoda MU1440/790 的无线接口适配；MU1440/791 不支持。MU1102 的现有组合继续保留，MU1433 与 MU1447 仍在仅有线范围内。

V3.1 is an experimental prerelease. Wireless ports are added for MU1367/790, MU1367/791 and Škoda MU1440/790. MU1440/791 is unsupported. Existing MU1102 combinations remain; MU1433 and MU1447 remain wired-only.

## 最终离线检查 / Final offline checks

| 检查 / Check | 结果 / Result |
|---|---|
| 安装元信息、路径、完整清单与归档 / Installer metadata, paths, complete manifest and archive | 130 个文件、129 项清单逐项检查；仅功能事务脚本作函数重命名，其他载荷逐字节一致 / 130 files, 129 manifest entries; only the feature-transaction function name/calls change |
| 错误注入拒绝 / Negative fault-injection tests | 21 种错误拒绝测试，包含 QNX 别名冲突回归、LF／混合换行、错误签名头和错误目录算法 / 21 rejection cases including alias regression, line endings, signed headers and directory algorithm |
| QNX SDK ksh 语法检查 / QNX SDK ksh parser | 旧第 21 行错误可重现，候选脚本逐项解析；不是原车执行验收 / old line-21 error reproduced; candidate scripts parsed individually; not on-unit execution acceptance |
| 7-Zip 与独立实现交叉检查 / 7-Zip and independent calculation | 两文件合计 516 字节；目录 data 校验与作者一致；不是 data-and-names，也不是 hashes.txt 本身的值 / two files, 516 bytes; data hash matches the author, not data-and-names or hashes.txt alone |
| 新装、重复更新、管理入口更新、恢复、卸载与拒绝未知改动 / Install, updates, restore, uninstall and unknown-change refusals | 396 项通过 / assertions passed |
| 历史公开版升级 / Historical public-release upgrades | 1071 项通过；九个旧版本、四十九个版本／机型组合 / assertions passed; nine old releases and forty-nine combinations |
| 已发布无线前代与 V3.0 升级 / Published wireless predecessor and V3.0 migration | 各七种配置，含重复更新、恢复与卸载 / seven profiles each, including repeated updates, restore and uninstall |
| 三套原厂 HMI SDK / Three exact native HMI SDKs | 每套 296 项菜单契约与 92 项型号识别检查通过 / 296 menu-contract and 92 profile-identification checks per SDK |
| 蓝牙接入兼容 / Bluetooth integration compatibility | 三种原始程序、39 项入口签名核对；1367／1440 的 15 个关键函数窗口一致 / 39 entry-signature checks across three originals and fifteen matching critical function windows |
| 真实配置与启动文件 / Actual configuration and startup files | 36 项合并、重复合并及一致性检查通过 / 36 merge, repeated-merge and consistency checks |

1367、1440 的原厂连接管理程序与 1102 不同，本包保留各自原程序。关键原生程序按已知指纹核对，未知修改停止；没有通过改写固件名称来放宽匹配。MU1440 的生产配置检查使用保存的对应固件系统备份，MU1102、MU1367 使用原始固件映像中的配置。

MU1367 and MU1440 retain their own stock connection manager rather than receiving the MU1102 program. Known native-program fingerprints are checked and unknown modifications stop installation. MU1440 production-configuration checks use a retained exact-firmware system backup; MU1102 and MU1367 configuration comes from original firmware images.

安装器的配置合并辅助程序因原厂同名参数对象问题重新构建，ARM QNX ELF 格式已核对。已有无线接收、画面处理与地图原生程序未因此重新替换。最终安装与升级回归针对修订后的程序重新运行；安装包内容与受测载荷逐项比对，校验清单、软件更新绑定和 ZIP 回读另外验证。

The configuration-merging helper is rebuilt for the same-named native settings-object fix and checked as an ARM QNX ELF executable. Existing wireless receiver, video and map native programs are not replaced by that fix. Full install/upgrade regression is rerun against the revised files. Tested runtime files, manifests, SWDL bindings and ZIP readback are checked separately.

## 验收边界 / Acceptance boundary

没有连接、写入或重启车机；没有把主机模拟测试当作实际 QNX 加载或手机连接成功。新增机型的无线配对、音频、真实触控、仪表画面、重连、原车泊车功能与长期稳定性仍需实机分项验收。首次连接需要手动完成蓝牙配对和手机加入车机 Wi-Fi 热点。

No head unit is connected, written or rebooted. Host-fixture checks are not actual QNX loading or successful phone connection. New targets still require separate hardware checks for pairing, audio, touch, cluster display, reconnection, factory parking functions and long-term stability. First use requires manual Bluetooth pairing and joining the head-unit Wi-Fi hotspot from the phone.

公开交付只包含安装包、安装方法、开发过程和验证说明，不附实现源码包。原始交付、V3.0 及旧报告在本地保留，不改写为 V3.1 已完成实车验收。

The public delivery contains the installer, installation instructions, engineering history and validation notes without a separate implementation-source package. Original handoffs, V3.0 and earlier reports remain historical, not relabeled as V3.1 vehicle acceptance.
