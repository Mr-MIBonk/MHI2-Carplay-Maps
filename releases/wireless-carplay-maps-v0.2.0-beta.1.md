# Wireless CarPlay Maps v0.2.0-beta.1

Experimental prerelease / 试验预发布，不替代稳定版 2.0.1。

新增 MU1102 实际无线 CarPlay 与独立 SD／绿色菜单安装。中控主屏有线与无线均申请 **30 fps**；仪表保留既定的独立配置，不把申请数值当作实测输出。

## 下载与安装

- `Wireless_CarPlay_Maps_Standalone_v0.2.0-beta.1.zip`：独立安装包，把 SD_ROOT 内容放到卡根；从原车软件更新注册后，进入绿色菜单 Wireless CarPlay Maps 安装功能。
- 本次不另行提供实现源码包；安装请使用上述 Standalone 文件。No separate implementation-source package is distributed; use the Standalone file above for installation.
- `SHA256SUMS.txt`：下载后校验。安装、备份与恢复说明见 [README](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/wireless-carplay-maps-v0.2.0-beta.1/README.md)。

只保留 iPhone 17 Pro Max 手机配置。保留地图、镜像、导航卡片和方向盘地图缩放；不带独立音乐／封面／导航文字映射、仪表启动动画及迎宾／离车音效。无线设置只提供无线 CarPlay。

## 适配边界

MU1102（lsd.jxe 55915411）提供无线引擎，支持 790／791。MU1367、Škoda MU1440／MU1433、SEAT MU1447 仍仅有线，具体仪表与精确固件见 README。其他 MHI2／MHI2Q 平台在适配，不能根据相近固件号强装。

## 升级检查

R2 已通过 49 个历史版本／机型升级组合及 1,272 项离线断言。修复了旧 V2.1 媒体包装器依赖残留、旧信息类路径清理、旧 R1 配置迁移与备份校验。新装后用新入口恢复，不运行旧 SD 卸载脚本；未知或私人加密版本保留现场并停止。

## 开发过程

团队回顾的节点：8 月 11 日立项，17 日有进展，18 日早期跑通，25 日继续接手，29 日取得关键进展。CloudCode 参与早期辅助；“81 版”只记录另一条探索线的尝试，不是成功基线或 V8.1。

[详细开发历程](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/wireless-carplay-maps-v0.2.0-beta.1/docs/WIRELESS_DEVELOPMENT.zh-CN.md)说明蓝牙引导、WLAN 接收、原厂 HMI／输入接入、画面状态隔离、会话所有权和切换恢复；位置声明只解释实现方式，不公开具体坐标。源码声明日期不是这次发布或验收时间。

## 已知边界

新精简组合尚未完成所有真实手机／仪表／触屏验收。首次无线断连个例、790 卡片位置和实际镜像流畅度仍需实机确认；Home 后冻结、静态页面返回延迟和倒车影像上仪表仍暂缓。USB 仍插着时从主动断开的有线再选无线仍有限制。自定义网络策略共存需单独审查。

仅供具有完整备份及恢复能力的测试者。停稳、稳定供电、断开手机，完整校验成功后再手动重启。离线检查不是实车稳定性保证。

## 致谢与依赖

特别感谢 **[olli991](https://github.com/olli991)** 的大量开发思路、原车对照和测试反馈；感谢 **[octacon84](https://github.com/octacon84)** 的 SEAT/MU1447 素材与布局。维护者 omonob。OEM、SDK 与真实上游归属及许可保留，冻结二进制不冒称全新原创。

## English

Independent SD/GEM experimental release. Actual wireless service is limited to exact MU1102 targets; the other listed firmware profiles remain wired-only. Wired/wireless main-screen requests are 30 fps. Only the iPhone 17 Pro Max phone profile is supplied. See the README and development history for the architecture, upgrade tests, recovery procedure and known limits. Offline checks are not real-car acceptance. Existing stable and historical releases are unchanged.
