# VW MHI2 Wireless CarPlay Maps

**Wireless CarPlay Maps V3.2 — 公开测试版 / Public experimental release**

[下载 V3.2 安装包 / Download V3.2](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/releases/tag/v3.2) · [中英文安装说明 / Installation](docs/WIRELESS_INSTALLATION.md) · [开发历程 / Development history](docs/WIRELESS_DEVELOPMENT.zh-CN.md) · [升级兼容性 / Compatibility](docs/WIRELESS_COMPATIBILITY.zh-CN.md) · [连接方法 / Connection](docs/WIRELESS_CONNECTION.zh-CN_EN.md)

让支持的 MHI2 车机使用无线 CarPlay，把手机地图映射到仪表，让仪表也显示地图画面。保留地图、屏幕镜像、导航卡片和方向盘地图缩放，通过原车界面完成设置和切换。

Use wireless CarPlay on supported MHI2 head units and display the phone map on the instrument cluster. Map views, screen mirroring, navigation cards and supported steering-wheel zoom remain integrated with the native interface.

维护者 / Maintainer：[omonob](https://github.com/omonob)。本项目不隶属于 Volkswagen、Škoda、SEAT、Audi、Apple 或 QNX。

## V3.2 本次交付 / This release

本次发布的是用户刚刚通过 SD 卡测试安装的同一份安装包，不是重新修改后的另一个构建。用户反馈 MU1367 的 SD 软件更新与功能安装已正常完成；随后只读检查也读到了 V3.2 的管理入口及运行版本标记。安装成功与各项功能、不同仪表、长期运行的实机验收分开记录。

This is the exact installer tested by the user through the SD workflow, not a changed rebuild. The user reports successful native SD update and feature installation on MU1367. Subsequent read-only inspection found V3.2 manager and runtime markers. Installation acceptance is separate from full feature, cluster and long-duration acceptance.

- **整套入口更新**：管理页、校验工具、事务脚本和恢复助手作为一组检查，避免新 SD 配旧入口时缺少依赖。The complete manager is checked as one bundle, avoiding mixed old/new dependencies.
- **旧入口衔接**：保留介质发现兼容；旧入口遇到新卡时先升级完整管理组件，再继续匹配的安装请求。A legacy entry hands off to the complete new manager before continuing installation.
- **中断恢复**：先暂存并验证输入，再提交；保留持久恢复材料和首次基线。未知改动、损坏备份或不能确认的旧状态仍会停止，不强行覆盖。Staging, persistent recovery and first-baseline preservation remain fail-safe.
- **更新收尾与开发者模式**：修正原厂更新收尾、节点路径和工具兼容；注册成功后开启并保存开发者／绿色菜单开关。Native-update finalization and node/tool compatibility are corrected; successful registration enables and saves green-menu access.
- **按最终文件核对**：元信息换行、脚本和目录校验、包内清单与下载散列重新核对。Metadata, script/directory checksums, package manifests and download hashes are checked together.

不改密码、SSH、设备密钥、FEC、仪表编码、地图包或其他项目的文件。无线、地图、菜单及媒体业务载荷未在此次安装修订中另行改写。

No password, SSH, device key, FEC, cluster coding, map-package or unrelated-project changes. This installation revision does not introduce a different wireless/map/menu/media runtime payload.

## 固件与验收范围 / Firmware and acceptance scope

| 固件 / Firmware | 仪表 / Cluster | 本版记录 / V3.2 evidence |
|---|---|---|
| MU1102 | 790 / AID 10.5；791 / AID 12.3 | 离线回归通过，本轮未实机复测 / Offline regression passed; no fresh V3.2 hardware retest |
| MU1367 | 790 / AID 10.5 | 离线回归通过；用户报告 SD 安装成功 / Offline regression passed; user-reported SD installation success |
| MU1367 | 791 / AID 12.3 | 离线回归通过，仍待本组合实机验收 / Offline regression passed; this combination awaits hardware acceptance |
| MU1440 Škoda | 790 / AID 10.5 | 离线回归通过，本轮未实机复测 / Offline regression passed; no fresh V3.2 hardware retest |

MU1440＋791 不支持。MU1367、MU1440 保留各自原厂连接管理程序，不跨固件覆盖 MU1102 的程序。相近版本号或同品牌不代表可直接使用，安装器遇到未知原生文件会停止。其他先前仅有线的平台不因此变成已支持的无线平台。

MU1440/791 is unsupported. MU1367 and MU1440 preserve their own stock connection manager. Similar version numbers or the same vehicle brand do not establish compatibility. Other previously wired-only targets are not newly certified wireless ports.

## 安装和首次连接 / Installation and first connection

1. 下载发布页里的 `Wireless_CarPlay_Maps_Standalone_v3.2.zip`，按 `SHA256SUMS.txt` 校验。Download the Standalone ZIP and verify its checksum.
2. 将 `SD_ROOT` **里面**的 `metainfo2.txt`、`install/`、`carplay/` 放到 SD 卡根目录，不要多套一层文件夹。Copy the contents inside `SD_ROOT` directly to the SD root.
3. 车辆停稳、稳定供电、手机完全断开；先走原车软件更新注册 V3.2 管理入口，再进入绿色菜单安装匹配仪表的功能。With reliable power and CarPlay disconnected, register the manager through native SD software update, then install the matching feature profile in the green menu.
4. 等待 `WMAPS_FEATURE_COMMIT_OK` 与 `WMAPS_STANDALONE_FEATURE_OK`。返回菜单后再取卡，按流程完成重启后连接手机。Wait for both feature-success markers before removing media or restarting.

**首次无线连接必须先手动配对蓝牙，再让 iPhone 加入车机自身的 Wi-Fi 热点。两项连接完成后无线 CarPlay 才会启动。** 不是连接手机个人热点，也不是只打开无线开关。

**First pair Bluetooth manually, then join the head unit's own Wi-Fi hotspot from the iPhone. Both connections are required.** Do not substitute the phone's Personal Hotspot or just enable a switch.

## 已知问题 / Known issue

当前实测反馈：VW 返回原厂入口的图标可以显示，但下面的 `Volkswagen` 名称没有显示。此问题保留在 V3.2，不混入已测通过的安装包；计划留待 V3.3／V3.4 后续修订。不是本版已经完成修复的项目。

The VW return-to-native icon may appear without its `Volkswagen` caption. This remains a known V3.2 issue and is deferred to a later V3.3/V3.4 revision. No untested caption change is included in this installer.

## 开发过程 / Development background

2026-08-11 立项，08-17 有阶段性进展，08-18 早期链路跑通，08-25 在已有项目与交付材料上接手继续推进，08-29 取得新的关键进展。早期项目由 CloudCode 辅助开展；“81 版”只记录另一条探索线的累计尝试，不作为成功基线或本次版本号。早期节点依据团队回顾，不由后来的构建日期反推。

The project started on 11 August, progressed on 17 August, reached an early working path on 18 August, entered further structured development on 25 August, and reached another milestone on 29 August 2026. CloudCode assisted the early work. The separate “81 revisions” effort is exploration history, not a successful baseline or this release number.

后续研究依次处理蓝牙引导、热点实际信道、接收服务与网络规则、原厂界面和触控回传、地图与镜像的会话归属、返回地图的输入恢复、旧版包装器及类路径清理，以及 SD 注册、功能安装和中断回退。完整实现思路、遇到的问题、验证方法与仍未验收的边界见[详细开发历程](docs/WIRELESS_DEVELOPMENT.zh-CN.md)。

Further work addressed Bluetooth bootstrap, hotspot/channel state, receiver/network readiness, native UI/input integration, map/mirror session ownership, return-to-map recovery, legacy wrappers/classpaths and recoverable SD delivery. See the [development history](docs/WIRELESS_DEVELOPMENT.zh-CN.md) for the full engineering narrative and validation limits.

本精简版仅保留 iPhone 17 Pro Max 手机配置及各仪表必要硬件适配。不包含独立歌曲文字、封面、导航文字映射、仪表启动动画或迎宾／离车声音。正常 CarPlay 声音与媒体使用保留。

The lean package retains the iPhone 17 Pro Max profile and required per-cluster hardware adaptations. Separate song/artwork/navigation-text mapping, cluster startup animation and welcome/exit sounds are not included. Normal CarPlay audio/media remain available.

本次公开安装包、安装方法和开发说明，不单独上传实现源码包。安装所需脚本和运行文件仍在安装包内；请使用发布页的 Standalone ZIP，不把 GitHub 自动生成的标签快照当作 SD 安装包。[致谢与来源 / Credits](CREDITS.md)。

This publication provides the installer and documentation, without a separate implementation-source archive. Required installation scripts and runtime files remain in the installer. Use the Standalone ZIP, not GitHub's automatic tagged-repository snapshot.
