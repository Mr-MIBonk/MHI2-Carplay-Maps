# Wireless CarPlay Maps V3.1 验证记录 / Validation

## 安装包装修订说明 / Installer-metadata correction

先前检查没有发现下载组名与组件 ID 不一致，以及目录声明缺少 `install` 前缀。这是原厂软件更新元信息关联检查的遗漏，不应以功能安装事务或 ZIP 校验通过代替原厂更新入口验收。本次修正两项声明，按最终文件重算校验，并独立检查目录关联、组名、源路径、脚本和组件校验、完整清单及归档内容；反向测试必须拒绝旧错误。无线、地图、菜单及安装／恢复程序不改动。软件更新界面的实机验收仍待复测。

Earlier checks missed a component/download-group mismatch and a missing `install` prefix in the final-script directory declaration. Passing feature transactions or ZIP checks did not validate native software-update metadata acceptance. This repack corrects both declarations, regenerates integrity metadata and independently checks directory/group relationships, source paths, script/component hashes, complete manifests and archive contents. Negative tests must reject the old errors. Wireless, map, menu and install/restore programs remain unchanged. Native software-update acceptance still requires a hardware retest.

V3.1 为试验预发布。新增 MU1367/790、MU1367/791、Škoda MU1440/790 的无线接口适配；MU1440/791 不支持。MU1102 的现有组合继续保留，MU1433 与 MU1447 仍在仅有线范围内。

V3.1 is an experimental prerelease. Wireless ports are added for MU1367/790, MU1367/791 and Škoda MU1440/790. MU1440/791 is unsupported. Existing MU1102 combinations remain; MU1433 and MU1447 remain wired-only.

## 最终离线检查 / Final offline checks

| 检查 / Check | 结果 / Result |
|---|---|
| 安装元信息、路径、完整清单与归档 / Installer metadata, paths, complete manifest and archive | 614 项检查通过；130 个文件、129 项清单；全部安装／运行载荷与修订前逐字节一致 / 614 checks; 130 files, 129 manifest entries; install/runtime payloads byte-identical |
| 错误注入拒绝 / Negative fault-injection tests | 13 种错误均被拒绝，包含旧组名、旧目录、错误校验、大小、源路径、目的路径、版本和签名前缀 / all 13 injected faults rejected, including both previously missed mismatches |
| 工具箱校验脚本交叉检查 / Upstream hash-generator cross-check | 在独立副本中重新生成后校验值等价，收尾脚本、目录清单及原签名前缀不变 / equivalent hashes after regeneration in an isolated copy; final script, hash list and signed preamble unchanged |
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
