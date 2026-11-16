# VW MHI2 Wireless CarPlay Maps

**Wireless CarPlay Maps v0.2.0-beta.1 — experimental prerelease / 试验预发布。**

Wireless CarPlay, phone-map video and screen mirroring for supported VW MHI2 platforms. Maintainer: [omonob](https://github.com/omonob).

大众 MHI2 无线 CarPlay、手机地图视频到仪表及屏幕镜像。**中控主屏申请 30 fps**，有线与无线采用同一主屏策略。仪表保留各自既定地图／镜像策略，申请值不等于实测消费帧率。

[Download independent SD package / 独立安装包](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/releases/tag/wireless-carplay-maps-v0.2.0-beta.1) · [开发历程](docs/WIRELESS_DEVELOPMENT.zh-CN.md) · [安装与兼容性](docs/WIRELESS_INSTALLATION.zh-CN.md) · [旧版升级检查](docs/WIRELESS_COMPATIBILITY.zh-CN.md) · [源码](wireless/) · [Credits](CREDITS.md)

## 本版范围

- MU1102 新增实际无线 CarPlay 接收服务；手机连接设置只提供本项目的“无线 CarPlay”开关。
- 保留 CarPlay 地图、镜像、导航卡片和方向盘地图缩放。卡片是视频场景，与独立文字信息映射不同。
- 只提供 iPhone 17 Pro Max 手机配置；790 和 791 保留必要的独立硬件适配。
- 精简版不包含歌曲文字、封面及导航文字映射模块，不包含仪表启动动画或迎宾／离车声音。
- 独立 SD 软件更新注册、独立绿色菜单管理，不再要求完整 MQB Toolbox。原车其他手机协议不被擅自删除。
- 不改密码、SSH、设备密钥、FEC、仪表编码或其他项目的文件。

## 精确支持范围

| 固件 | lsd.jxe 字节数 | 仪表 | 无线服务 |
|---|---:|---|---|
| MU1102 | 55915411 | 790／791 | 本版提供 |
| MU1367 | 55840541 | 790／791 | 尚未移植，本版仅有线 |
| MU1440 Škoda | 55840933 | 790 | 尚未移植，本版仅有线 |
| MU1433 Škoda G11 | 52751492 | 790 | 尚未移植，本版仅有线 |
| MU1447 SEAT G11 | 55712574 | 790 | 尚未移植，本版仅有线 |

其他台湾、欧洲及 Audi MHI2／MHI2Q 精确平台仍在适配，未列出即不支持，不以相近固件号推定兼容。新精简组合尚未完成全部真实手机／仪表／触屏验收；无线主屏与衍生功能的既有测试经验不等于所有组合均已验证。

## 独立安装

1. 完整备份原系统，准备恢复通路。车辆停稳、稳定供电，完全断开 USB 与无线 CarPlay。
2. 从本页试验预发布下载 `Wireless_CarPlay_Maps_Standalone_v0.2.0-beta.1.zip`，按 `SHA256SUMS.txt` 校验。
3. 把 ZIP 内 **SD_ROOT 的内容**复制到 SD 卡根目录，让 `metainfo2.txt` 位于卡根。
4. 从原车软件更新入口选择 SD，注册独立绿色菜单页。此步只安装管理入口，不刷整套固件。
5. 打开绿色菜单 **Wireless CarPlay Maps**，按真实仪表选择安装项；等待 `WMAPS_FEATURE_COMMIT_OK` 和全部校验通过。
6. 手动完整重启后再连接手机。失败时保留日志，不反复运行旧安装包。

首次安装前保存的基线用于恢复；若原本已有公开旧版，恢复的是该旧版，不冒称恢复原厂。新装后请使用新管理入口恢复，不使用旧 SD 卸载脚本。未知包装器、自定义启动文件、私人加密版本或缺少安装工具时会停止，不强制覆盖。

## 离线验证与已知问题

发布依据 R2 的 **49 个历史版本／机型升级组合、1,272 项离线断言**，覆盖 9 个历史公开版本、7 个当前机型配置以及安装、重复更新、失败回退、首次基线恢复和卸载。发布前另行复核主屏申请和最终归档完整性，记录见 [检查报告](wireless/evidence/)。

已修正旧 V2.1 升级时的媒体包装器残留、旧信息 JAR／类路径退出、旧 R1 手机配置迁移和备份完整性检查。原 R26、R1 交付及旧发布标签不改写。

仍需实机验证：个例首次无线断连、790 卡片位置、无线镜像实际流畅度、自定义网络策略共存。Home 后后台镜像暂停、静态页面返回中控延迟恢复及倒车影像到仪表仍暂缓；USB 仍插着时主动断开有线后再选无线仍有限制。仅供有完整备份与恢复能力的测试者使用。

## 开发历程

根据团队回顾：8 月 11 日立项、17 日取得进展、18 日早期跑通、25 日接手继续研究、29 日取得关键进展。早期由 CloudCode 辅助；“81 版”仅指另一路尚未成功的累计尝试，不是正式版本号或成功黄金基线。

[详细开发说明](docs/WIRELESS_DEVELOPMENT.zh-CN.md)解释蓝牙引导、WLAN 接收、原厂 HMI 接入、主屏与仪表状态隔离、触控、会话所有权及切换恢复；介绍显示能力和位置声明的实现方法，不公布具体位置参数。源码页眉日期不是发行或验收日期。

## Source and provenance

Owned integration sources are in `wireless/`; the corresponding independent installer is in `standalone/SD_ROOT/`. The source archive is **not a complete C-source rebuild of the frozen wireless receiver or OEM/SDK dependencies**. See `DEPENDENCIES.json`, `SOURCE_BUILD.md` and component notices. The project's GPL-3.0 terms do not relabel OEM/SDK code or grant rights the project does not own.

特别感谢 **[olli991](https://github.com/olli991)** 提供大量开发思路、原车行为对照及测试反馈；感谢 **[octacon84](https://github.com/octacon84)** 提供 SEAT/MU1447 素材与布局工作。[完整致谢](CREDITS.md)。

## Previous releases

Stable [2.0.1](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/releases/tag/toolbox-plaintext-v2.0.1) and the earlier [2.1 rc.7](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/releases/tag/toolbox-plaintext-v2.1.0-rc.7) remain available. Their full-toolbox installation instructions apply to those releases, **not this independent wireless package**. [Historical introduction](docs/LEGACY_V2_1_README.md).

## English summary

This experimental release adds an actual wireless CarPlay service on exact MU1102 targets, plus independent SD/GEM installation. Wired and wireless main-screen requests use 30 fps. Instrument-map/mirror policies remain profile-specific. Only the iPhone 17 Pro Max phone profile is offered. Other firmware rows are wired-only; this is not universal MHI2/MHI2Q support.

Song/cover/navigation-text mapping, instrument startup animations and welcome/exit sounds are outside this lean package. Back up the unit, disconnect the phone, install the independent manager, select the real cluster, and fully reboot only after all checks succeed. Use the new manager to restore the saved first baseline. Do not use an old uninstaller on the new runtime.

Offline synthetic upgrade and rollback tests do not prove phone/GPU/touchscreen acceptance or long-term real-car stability. Known issues and frozen dependency boundaries are listed above. Old stable releases remain unchanged.
