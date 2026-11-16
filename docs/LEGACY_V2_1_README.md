# CarPlay Maps VC for MHI2

Free open-source CarPlay maps and screen mirroring for Volkswagen, Škoda and SEAT virtual cockpits. Author: [omonob 汽车大玩家](https://github.com/omonob).

大众、斯柯达与西雅特 MHI2 的免费开源 CarPlay 地图和屏幕镜像项目。开源软件本身及 GPLv3 授予的权利不收费。

[Downloads 下载](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/releases/latest) · [2.0.1 release notes 中英文更新说明](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/toolbox-plaintext-v2.1.0-rc.7/releases/toolbox-plaintext-v2.0.1.md) · [GPLv3](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/toolbox-plaintext-v2.1.0-rc.7/LICENSE)

**V2.1 试验预发布版 / Experimental prerelease:**
[下载 2.1.0-rc.7 / Download](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/releases/tag/toolbox-plaintext-v2.1.0-rc.7) · [中英文说明 / Notes](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/toolbox-plaintext-v2.1.0-rc.7/releases/toolbox-plaintext-v2.1.0.md) · [离线检查与原厂界面截图 / Validation and native UI screenshots](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/toolbox-plaintext-v2.1.0-rc.7/docs/V2_1_VALIDATION.md)。新增西雅特专属卡片区域和 CarPlay 信息（试验）。**长时间虚拟机测试出现过多次 QNX 内核停止及一次媒体启动异常，原因未查清；真实手机／仪表验收未完成。仅供有备份与恢复能力的测试者，不是稳定版。** Long-running emulator tests encountered unresolved QNX kernel stops and a media-startup failure. Real phone/cluster acceptance is incomplete. This is for experienced testers with backups and recovery access, not a stability guarantee. Stable 2.0.1 remains unchanged.

[开发进度、图片与帧率／延迟数据](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/toolbox-plaintext-v2.1.0-rc.7/docs/development/2026-10-28/README.md) · [Development report, images and measurements](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/toolbox-plaintext-v2.1.0-rc.7/docs/development/2026-10-28/README_EN.md)

**社区特别致谢 / Special community thanks:**
特别感谢 [olli991](https://github.com/olli991) 提供大量开发思路、实际建议和西雅特测试反馈；
感谢 [octacon84](https://github.com/octacon84) 提交 MU1447 适配补丁及社区测试结果。
Special thanks to **olli991** for extensive development ideas, practical suggestions
and SEAT testing feedback, and to **octacon84** for the MU1447 adaptation and testing.
[完整致谢与贡献边界 / Full acknowledgements and provenance](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/toolbox-plaintext-v2.1.0-rc.7/docs/COMMUNITY_V2_1_CREDITS.md).

**早期 G11 测试版 / Earlier G11 prerelease:** [2.1.0-g11-beta.1](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/releases/tag/toolbox-plaintext-v2.1.0-g11-beta.1) · [说明 / Notes](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/toolbox-plaintext-v2.1.0-rc.7/releases/toolbox-plaintext-v2.1.0-g11-beta.1.md)。V2.1 rc.7 包含该版支持的 Škoda MU1433 和 SEAT MU1447，仅 AID 10.5；MU0033 不支持。稳定版 2.0.1 不变。Only the traditional full MQB package is published; the standalone installer remains a local experiment.

## 中文说明

### 2.1 虚拟座舱设置

App-Connect 设置里增加独立的“虚拟座舱设置”入口，保留原厂 MirrorLink 页面。菜单使用原厂选项条和连续滚动列表，包含 31 种语言，设置在重启后保留。

| 设置 | 功能 |
| --- | --- |
| 虚拟座舱地图 | CarPlay Maps 或原车地图 |
| 虚拟座舱镜像 | CarPlay 屏幕镜像开启／关闭，地图优先，冲突项置灰 |
| 无线 CarPlay | 测试状态接口，默认关闭；实际无线服务未发布 |
| 导航卡片 | AID 10.5：中间、靠左、靠右、关闭；AID 12.3：开启、关闭 |
| CarPlay 信息（试验） | 全部开启、关闭导航、关闭音乐、全部关闭；支持热切换和重启记忆 |

共用菜单保留 AA／CarLife 名称，但本包不包含对应引擎，相关选项不可选择。镜像缩放按键只调整画布，不改变仪表输出尺寸。V2.1 新增本地 V53 衍生的导航文字、音乐信息和封面实验适配，独立于地图／镜像开关；不包含启动动画。小屏 G11 的英文标签缩写为 `CP info (experimental)`，保留完整试验标记。适配细节与验收边界见 [信息模块说明](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/toolbox-plaintext-v2.1.0-rc.7/src/information/README.md)。

### 支持范围

| 固件 | lsd.jxe 大小 | AID 12.3 | AID 10.5 |
| --- | ---: | --- | --- |
| MU1102 | 55,915,411 bytes | 800×480 | 1010×376 |
| MU1367 | 55,840,541 bytes | 800×480 | 1010×376 |
| MU1440 Škoda | 55,840,933 bytes | 不支持 | 1010×376，保留滑轮方向 |
| MU1433 Škoda G11（测试版） | 52,751,492 bytes | 不支持 | 1010×376，原品牌 800×480 中控 UI |
| MU1447 SEAT G11（测试版） | 55,712,574 bytes | 不支持 | 1010×376，原品牌 800×480 中控 UI |

AID 10.5 保留大众／斯柯达既有左／右／中间卡片安全区；SEAT MU1447 在 V2.1 改用社区提供的独立安全区，不能套用大众的上下裁切与 x=42 纯地图修正，见 [SEAT 适配说明](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/toolbox-plaintext-v2.1.0-rc.7/docs/SEAT_V2_1_ADAPTATION.md)。Waze 卡片仍有已知布局问题。大众／斯柯达的 2.0.1 iPhone 17 Pro Max 纯地图居中声明保留，输出仍为 1010×376；参数不保证适合所有手机。

AID 12.3 使用实验安全区 `(220,65,360,348)`，底边保持 413；该最终参数尚待实车确认，不能当作已通过实车验证的黄金版。其原生核心在 2.0.1 中未变。完整包并未在所有车型上验证。

AID 12.3 保留 40 fps 协商声明与每 5 帧请求一次 IDR。AID 10.5 卡片基线声明上限为 60 fps；AID 10.5 CP 镜像模式将原厂 10 fps 编码请求改为 30 fps。协商／请求值不等于所有手机和应用的实测显示帧率。

### 安装

1. 保存本机系统备份，车辆停稳并持续供电，完全断开 CarPlay。
2. 从 Releases 下载完整 MQB Toolbox ZIP，解压到 SD 卡根目录，用随包 SHA-256 文件校验。
3. 进入 MQB Toolbox → Customization → CarPlay Maps VC，按真实仪表选择 AID 10.5 或 AID 12.3。
4. 安装器精确识别固件，已有有效导航共存补丁直接保留；缺少或本项目副本损坏则自动补齐。
5. 看到 `CPMAP_INSTALL_OK` 后，通过工具箱流程完整重启，再打开 App-Connect 设置 → 虚拟座舱设置。

只提供 MQB 完整工具箱，不提供 M.I.B. 自动刷写。安装失败不要连接手机测试。原厂恢复入口恢复保存的启动文件和 JAR，独立保存的用户设置不删除。已有加密版本请使用其专用迁移工具，公共安装器不会拆除它。

七种固件／仪表组合的集成文件位于 [MU1102](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/toolbox-plaintext-v2.1.0-rc.7/MU1102/)、[MU1367](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/toolbox-plaintext-v2.1.0-rc.7/MU1367/)、[MU1440 Škoda](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/toolbox-plaintext-v2.1.0-rc.7/MU1440_SKODA/)、[MU1433 Škoda G11](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/toolbox-plaintext-v2.1.0-rc.7/MU1433_SKODA/) 和 [MU1447 SEAT G11](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/toolbox-plaintext-v2.1.0-rc.7/MU1447_SEAT/)，适用于自行集成工具箱的开发者，不是独立刷写实验包。

### 联系和可选支持

需要未适配固件的免费版本，可联系作者并提供系统备份；备份可能包含身份和授权数据，请私下发送，不要上传到公开 Issue。

邮箱：[omonob@126.com](mailto:omonob@126.com) · Telegram：[omonob](https://t.me/omonob)。

额外启动 Logo、其他已验证适配与可选付费支持，和本仓库免费授权分开；V2.1 的信息实验模块不因此收费。[闲鱼付费选项](https://www.goofish.com/item?id=1076254792602)，原标价 ¥399 RMB；具体功能、固件和服务范围购买前联系确认。

## English

### Version 2.1

An independent Virtual Cockpit settings page is added under App-Connect settings without repurposing MirrorLink. It uses native selector rows and continuous scrolling. CRC-checked preferences survive reboots, with a previous valid copy available for recovery. Labels cover 31 locales; rendering uses the firmware's installed fonts.

The five selectors control VC Maps, VC Mirroring, CarPlay information (experimental), the unreleased Wireless CarPlay test interface, and Navigation Card. The information selector has All enabled, Navigation off, Music off and All off; its navigation/music policy is independent of video. On narrow G11 rows its localized title is shortened, e.g. `CP info (experimental)`, without dropping the experimental marker. Maps take priority over mirroring for the same source. Only the CarPlay engines are included; Android Auto and CarLife choices are disabled. The wireless switch is off by default and does not start a wireless service.

AID 10.5 has Center, Left, Right and Off card choices and a fixed 1010×376 output. AID 12.3 has On and Off and an 800×480 output. Canvas zoom changes content size, never the encoder dimensions. Steering zoom is routed to the canvas in CP mirroring and to the phone map otherwise.

MU1102 and MU1367 support both cluster profiles. MU1440 Škoda supports AID 10.5 only. Exact firmware sizes are listed above; unknown profiles are rejected before factory files are modified.

The **2.1.0-rc.7 experimental prerelease** includes exact Škoda MU1433 and SEAT MU1447 G11 firmware from the earlier G11 beta, AID 10.5 only. Their native menus use their own 800×480 head-unit geometry, while cluster video remains 1010×376. API/host checks cover both; the [validation report](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/toolbox-plaintext-v2.1.0-rc.7/docs/V2_1_VALIDATION.md) distinguishes native-menu checks from long-running emulator and real-phone/cluster acceptance. Unresolved emulator kernel stops and a media-startup failure are disclosed; this is not a stable build. MU0033 is excluded. Existing stable 2.0.1 downloads remain unchanged.

The existing Volkswagen/Škoda AID 10.5 Left, Right and Center card rectangles are retained; Waze still has a known card-layout issue. V2.1 uses a separate community-derived safe-area table for SEAT MU1447, without importing Volkswagen's x=42 pure-map correction; see the [SEAT adaptation notes](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/toolbox-plaintext-v2.1.0-rc.7/docs/SEAT_V2_1_ADAPTATION.md). The 2.0.1 iPhone 17 Pro Max pure-map view remains for Volkswagen/Škoda. Output stays 1010×376; these are not universal phone parameters.

The final AID 12.3 safe area `(220,65,360,348)` is an offline-tested experimental parameter, not a vehicle-validated golden release. Both AID 12.3 native cores are unchanged in 2.0.1. The complete package has not been verified on every vehicle.

AID 12.3 retains a 40 fps stream declaration and an IDR request every five frames. The AID 10.5 card baseline declares a 60 fps ceiling. Only in AID 10.5 CP-mirror mode, the original 10 fps encoder request becomes 30 fps. These are not universal measured-FPS guarantees.

### Install and restore

Back up the unit, maintain stable power with the vehicle stationary, and disconnect CarPlay completely. Extract the full MQB ZIP to the SD-card root. Open MQB Toolbox → Customization → CarPlay Maps VC and select the actual AID 10.5 or AID 12.3 instrument. The installer preserves an existing valid navigation coexistence patch and automatically installs or repairs this project's missing/incomplete copy.

After `CPMAP_INSTALL_OK`, perform a complete head-unit reboot through the Toolbox procedure. Open App-Connect settings → Virtual Cockpit settings. Do not connect the phone after a failed install. Restore factory files restores the saved startup and JAR files, retaining separate user preferences.

V2.1 adds an experimental V53-derived navigation/music/album-art adaptation with independent hot switches. No M.I.B. automatic flashing installer, password changes, device-bound ciphertext, paid watchdog or startup animation is included. Existing encrypted installations require their own migration tool. Information support is experimental, not a universal physical-vehicle compatibility guarantee.

For an unsupported firmware, contact the author about a free adaptation and share the system backup privately, not in a public Issue. Contact [omonob@126.com](mailto:omonob@126.com) or [Telegram](https://t.me/omonob). Optional paid extensions are separate from the public software license; confirm the supported firmware and feature scope before purchasing.

## Build and test

Source is under `src/card-core`, `src/vc-menu`, `src/vc-runtime`, `src/information`, `src/toolbox` and `src/steering-mu1440`. Full OEM firmware, private credentials, device keys and proprietary fonts are not redistributed. Supply your own exact original firmware JARs to compile the minimal UI hooks. V2.1 information builds, provenance and tests are documented in [src/information/README.md](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/toolbox-plaintext-v2.1.0-rc.7/src/information/README.md).

For V2.1, build the five exact menu targets and information targets, the G11 navigation adapter, G11 cores and native information helpers in fresh folders. See [the information build instructions](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/toolbox-plaintext-v2.1.0-rc.7/src/information/README.md). `Stage-V21Profiles.ps1` stages all seven combinations; `Build-V21MQBCandidate.py` creates only a **local validation candidate** from the exact published 2.0.1 full MQB archive. It never uploads or marks a build accepted. Run `test_v21_candidate_archive.py` against the candidate, and run the Linux installer regressions plus native firmware UI tests before publication. Physical phone/cluster acceptance remains separate.

The commands below document the earlier V2.0/2.0.1 build flow, not the complete V2.1 information build:

1. Run `tools/Build-V2Menu.ps1` with `-Mu1102Jar`, `-Mu1367Jar`, `-Mu1440Jar` and a fresh `-BuildRoot`. It compiles against each firmware API and runs host persistence, policy, locale and profile tests.
2. Run `tools/Build-V2Native.sh` inside a compatible locally built QNX 6.5 ARMv7 GNU toolchain container with this repository mounted at `/src`. It builds five native cores, two fixed-resolution renderers and the shared runtime, and runs the actual declaration/async-transport tests under sanitizers.
3. Run `tools/Stage-V2Profiles.ps1` with `-MenuBuild` and `-NativeBuild`, then `tools/Build-MQBRelease.ps1 -Version 2.0.1` with the unchanged published v2.0.0 (or v1.2.2) full MQB ZIP and a fresh `-BuildRoot`. For a limited AID 10.5 rebuild, `tools/Build-AID10CenterPatch.sh` builds the three cores and verifies that both AID 12.3 cores remain byte-identical.
4. On a Linux host run `python3 tests/test_v2_install.py` and `python3 tests/test_navigation_repair.py` for fixture-only install, repeat, repair, rollback and restore tests. These scripts do not connect to a vehicle.

[2.0.1 centering notes](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/toolbox-plaintext-v2.1.0-rc.7/releases/toolbox-plaintext-v2.0.1.md) and the [original 2.0 feature notes](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/toolbox-plaintext-v2.1.0-rc.7/releases/toolbox-plaintext-v2.0.0.md) give the validation boundaries. The project remains licensed under [GPLv3](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/toolbox-plaintext-v2.1.0-rc.7/LICENSE).
