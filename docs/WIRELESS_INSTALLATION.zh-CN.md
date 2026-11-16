# Wireless CarPlay Maps V3.1 安装与目录说明 / Installation and layout

## 精确支持范围 / Exact support scope

| 固件 / Firmware | 仪表 / Cluster | 本包无线服务 / Wireless service |
|---|---|---|
| MU1102 | 790 / AID 10.5；791 / AID 12.3 | 提供 / Included |
| MU1367 | 790；791 | 新增试验适配；离线验证，待实机验收 / New experimental port; offline validation, hardware acceptance pending |
| MU1440 Škoda | 790 | 新增试验适配；离线验证，待实机验收 / New experimental port; offline validation, hardware acceptance pending |
| MU1433 Škoda G11 | 790 | 尚未移植，仅有线 / Wired only; wireless port pending |
| MU1447 SEAT G11 | 790 | 尚未移植，仅有线 / Wired only; wireless port pending |

安装器仍按固件原文件精确核对，不以型号名称相似放行。其他未列出的台湾、欧洲或 Audi MHI2／MHI2Q 平台仍在适配；不是所有同品牌车机都通用。

The installer still checks exact firmware inputs. Similar model names do not establish compatibility. Unlisted Taiwan, European and Audi MHI2/MHI2Q targets remain pending; this is not universal same-brand support.

## 该下载哪个文件 / Which download to use

| 下载文件 / File | 用途 / Purpose |
|---|---|
| `Wireless_CarPlay_Maps_Standalone_v3.1.zip` | 面向安装的完整 SD 目录。The complete SD folder for installation. |
| `SHA256SUMS.txt` | 校验下载文件是否完整。Download integrity checksums. |
| `INSTALLATION.zh-CN_EN.md` | 中英文安装、更新、恢复和目录说明。Bilingual installation, update, recovery and layout guide. |
| `WIRELESS_CONNECTION.zh-CN_EN.md` | 蓝牙配对、车机 Wi-Fi 连接与首次启动步骤。Bluetooth pairing, head-unit Wi-Fi and first-connection instructions. |
| `DEVELOPMENT_HISTORY.zh-CN.md` | 更完整的开发历程与实现说明。The extended Chinese development history. |
| `RELEASE_NOTES.md` | V3.1 发布详情及验收边界。V3.1 release details and acceptance boundaries. |

GitHub 自动显示的 `Source code (zip)`／`Source code (tar.gz)` 是对应标签的仓库快照，并非另行上传的源码包，也不是本项目整理的 SD 安装包。安装请只选择 **Standalone** 文件。

GitHub's automatic `Source code (zip)` and `Source code (tar.gz)` entries are snapshots of the tagged repository, not separately uploaded source packages or the prepared SD installer. For installation use only **Standalone**.

## 安装包目录结构 / Installer layout

```text
Wireless_CarPlay_Maps_Standalone_v3.1.zip
└─ SD_ROOT/
   ├─ metainfo2.txt                 软件更新入口 / SWDL entry metadata
   ├─ carplay/
   │  └─ payload/
   │     ├─ common/                 共用安装与地图程序 / Common installer and map runtime
   │     ├─ profiles/               精确固件与仪表载荷 / Exact firmware and cluster profiles
   │     └─ wireless/               无线接入及配套配置 / Wireless runtime and configuration
   ├─ install/
   │  ├─ esd/                      独立绿色菜单定义 / Dedicated green-menu definition
   │  ├─ scripts/                  注册、安装、更新与恢复 / Register, install, update and restore
   │  │  └─ final/                 软件更新收尾脚本 / SWDL final script
   │  └─ file-check                完整性检查辅助程序 / Integrity-check helper
   ├─ README.md                    项目与安装说明 / Project and installation notes
   ├─ CONTENTS.sha256              包内文件清单 / Per-file integrity manifest
   ├─ BUILD_INFO.json              当前构建信息 / Current build information
   ├─ DEPENDENCIES.json             依赖与来源记录 / Dependency provenance
   ├─ CREDITS.md                    致谢与归属 / Credits and provenance
   └─ ...                          其他随包说明和兼容标记 / Other notes and compatibility markers
```

**复制的是 `SD_ROOT` 里面的全部内容，不是外层文件夹。** 卡根应直接能看到 `metainfo2.txt`、`carplay` 和 `install`。不要只复制几个脚本，也不要单独运行 `payload/common` 内的内部脚本。

**Copy all contents inside `SD_ROOT`, not the outer folder itself.** The card root must directly contain `metainfo2.txt`, `carplay` and `install`. Do not copy only selected scripts or invoke internal `payload/common` scripts individually.

```text
正确 / Correct SD root       错误 / Incorrect nesting
SD:/metainfo2.txt            SD:/SD_ROOT/metainfo2.txt
SD:/carplay/                 SD:/Wireless_CarPlay_Maps/SD_ROOT/...
SD:/install/
```

## 中文安装步骤

1. **确认支持范围。** 核对固件与仪表，准备完整系统备份和可用的恢复通路。不要跳过不支持提示。
2. **准备平台。** 车辆停稳并持续稳定供电，完全断开 USB 和无线 CarPlay。工程／绿色菜单应已可用，本包不替你修改工程模式开关。
3. **下载和校验。** 下载 V3.1 的 Standalone ZIP，与 `SHA256SUMS.txt` 对照；文件不完整时先重新下载，不尝试安装。
4. **准备 SD 卡。** 解压后把 `SD_ROOT` 内全部内容复制到卡根，保留目录层级及所有随包文件。按上面的结构检查，不要多套一层目录。
5. **注册独立入口。** 从原车软件更新入口选择对应 SD 介质，完成管理页注册。这一步不是直接刷整套固件，也不等于功能已经安装。
6. **进入绿色菜单安装。** 打开 **Wireless CarPlay Maps**。790 选择 AID 10.5 项，791 选择 AID 12.3 项；仍以精确固件门禁为准。安装器选择匹配载荷，备份并检查当前文件后提交。
7. **确认完成再重启。** 等待 `WMAPS_FEATURE_COMMIT_OK` 和全部校验成功，之后手动完整重启车机，再连接手机。失败时保留日志，先不要反复重装、删除备份或连接手机测试。
8. **分项验收。** 检查无线连接、正常 CarPlay 画面与声音、仪表地图、镜像往返、卡片、方向盘缩放以及原车雷达／倒车等功能；有异常时保持现场，先恢复或排查。

不同固件的原厂软件更新入口名称、SD 挂载方式和工程菜单位置可能不同，本说明不提供跨平台猜测的隐藏菜单按键组合。

## Installation steps in English

1. **Confirm support.** Check the exact firmware and cluster. Keep a full system backup and a working recovery path. Do not bypass an unsupported-target refusal.
2. **Prepare the unit.** Park safely, maintain reliable power and fully disconnect USB and wireless CarPlay. Engineering/green-menu access must already be available; this package does not enable engineering mode for you.
3. **Download and verify.** Use the V3.1 Standalone ZIP and check it against `SHA256SUMS.txt`. Redownload a damaged file instead of attempting installation.
4. **Prepare the SD card.** Extract the ZIP and copy every item inside `SD_ROOT` to the card root, preserving the directory layout. Do not add an extra outer folder.
5. **Register the independent entry.** Select the SD medium from the unit's native software-update interface and complete registration. This installs the manager, not a full firmware image or the entire feature runtime.
6. **Install from the green menu.** Open **Wireless CarPlay Maps**. Select the AID 10.5 entry for 790 or the AID 12.3 entry for 791, subject to the exact firmware check. The installer stages the matching files, checks current state and saves the baseline before commit.
7. **Reboot only after success.** Wait for `WMAPS_FEATURE_COMMIT_OK` and all checks to succeed, then fully reboot manually before connecting the phone. On failure preserve logs; do not repeatedly reinstall, delete backups or test with the phone connected.
8. **Validate separately.** Check wireless connection, normal main-screen video/audio, cluster maps, mirror transitions, cards, steering zoom and factory parking/reverse functions. Preserve a failing state for diagnosis or restore it first.

Native update labels, SD mounts and engineering-menu locations vary by firmware. This guide does not guess hidden-button sequences across platforms.

## 首次无线连接 / First wireless connection

安装成功并完整重启后，拔下手机 USB，打开车机的无线 CarPlay 与 WLAN 热点。在 iPhone“设置 → 蓝牙”里手动配对车机，再到“设置 → Wi-Fi”加入这台车机的热点，使用车机页面显示的网络名和密码。两项连接完成后再进入 CarPlay；没有自动进入时，在 App-Connect 选择当前 iPhone。

After successful installation and a full restart, unplug USB and enable Wireless CarPlay and the head-unit WLAN hotspot. Manually pair the head unit in iPhone **Settings → Bluetooth**, then join its hotspot in **Settings → Wi-Fi**, using the network name and password displayed by the head unit. Enter CarPlay after both connections are ready; select the current iPhone in App-Connect if it does not open automatically.

这里连接的是**车机自身热点**，不是手机个人热点或外部上网 Wi-Fi。后续再分项检查中控画面、声音、仪表地图和方向盘缩放。完整步骤与连接排查见[中英文连接说明](docs/WIRELESS_CONNECTION.zh-CN_EN.md)。

Join **the head unit's own hotspot**, not the phone's Personal Hotspot or an external internet Wi-Fi network. Check main-screen video/audio, cluster maps and steering zoom separately. See the [bilingual connection guide](docs/WIRELESS_CONNECTION.zh-CN_EN.md) for the complete steps and troubleshooting.

## 更新与恢复 / Update and recovery

| 绿色菜单项 / Menu item | 含义 / Meaning |
|---|---|
| `Update this management entry only` | 从新的完整介质更新管理入口，保留首次备份。Update the manager from new complete media while retaining the first baseline. |
| `Update installed Wireless CarPlay Maps` | 按已保存的精确配置更新功能。Update the feature using the saved exact profile. |
| `Show status` | 查看识别结果与已安装配置。Inspect detected firmware and installed profile. |
| `Restore saved first baseline - keep this entry` | 恢复首次保存的状态，保留管理入口。Restore the saved first baseline and keep the manager. |
| `Restore baseline and remove this entry` | 先恢复，再移除本项目入口。Restore first, then remove this project's entry. |

升级请使用 **新的完整 V3.1 介质先更新管理入口，再更新功能**。首次保存的基线不一定是原厂：如果安装前已有旧公开版本，恢复会回到那个版本。不要再运行旧 SD 卸载脚本来拆除新运行目录。

Use **new complete V3.1 media to update the manager first, then update the feature**. The first baseline is not necessarily factory state: if an older public version was already installed, restore returns to it. Do not run an old SD uninstaller against the new runtime.

未知包装器、私人加密版本、损坏备份或安装后被其他项目修改的目标会停止处理，不强行覆盖；安装工具或空间不足也会明确停止。旧名称的个别脚本只是升级兼容壳，不代表新增手机配置，不能脱离管理入口单独运行。

Unknown wrappers, private encrypted versions, damaged backups or targets modified by another project stop the operation. Missing required tools or insufficient space also stop it. A retained legacy script name is only an updater compatibility shim, not another phone profile or a standalone install command.

## 公开文件结构 / Published files

```text
GitHub repository
├─ README.md                       当前 V3.1 项目首页 / Current V3.1 introduction
├─ standalone/SD_ROOT/             独立安装目录 / Independent installer tree
├─ docs/                           开发、安装与兼容说明 / Development, installation and compatibility
├─ releases/                       当前与历史发布说明 / Current and historical release notes
└─ CREDITS.md                       致谢与来源 / Credits and provenance
```

开发历程说明蓝牙引导、网络会话、原厂界面与仪表地图如何衔接，以及遇到的问题和处理方法；不附实现源码或源码构建工具。安装包内的 `DEPENDENCIES.json` 和 `CREDITS.md` 保留组件来源、真实上游和原有权利说明。

The development history explains how Bluetooth bootstrap, network sessions, the native interface and cluster maps fit together, including problems and solutions. It does not attach implementation sources or source-build tools. The installer's `DEPENDENCIES.json` and `CREDITS.md` retain component provenance, upstream attribution and applicable rights.
