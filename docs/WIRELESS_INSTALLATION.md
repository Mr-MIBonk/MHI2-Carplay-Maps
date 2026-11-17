# Wireless CarPlay Maps V3.2：安装与更新 / Installation and update

## 安装包结构 / Package layout

```text
Wireless_CarPlay_Maps_Standalone_v3.2.zip
└─ SD_ROOT/
   ├─ metainfo2.txt
   ├─ install/
   │  ├─ MEDIA_ID                 兼容旧入口的介质识别 / Legacy media discovery
   │  ├─ PRODUCT_VERSION          产品版本 / Product version
   │  ├─ CONTENTS.sha256          整包文件清单 / Complete media manifest
   │  ├─ MANAGER_FILES.tsv        管理组件映射 / Manager component mapping
   │  ├─ MANAGER.sha256           管理组件校验 / Manager bundle manifest
   │  ├─ file-check               校验辅助程序 / Integrity helper
   │  ├─ esd/                     独立绿色菜单定义 / Dedicated green-menu definition
   │  └─ scripts/
   │     ├─ entry.sh              管理入口 / Manager
   │     ├─ manager-transaction.sh 整套入口更新 / Whole-manager update
   │     └─ final/                原车更新收尾 / Native-update finalization
   └─ carplay/payload/
      ├─ common/                  安装、恢复和共用载荷 / Installation, recovery, shared files
      ├─ profiles/                精确固件及仪表文件 / Firmware/cluster profiles
      └─ wireless/                无线服务、配置和兼容核对 / Wireless runtime and profiles
```

**SD 卡根目录只有一个文本文件 `metainfo2.txt`，以及两个必要目录 `install/` 和 `carplay/`。** 说明文档在发布页单独提供，不堆放到 SD 根目录。目录内部的文件不是多余文件，不能删除或只复制部分。

**The SD root contains one text file, `metainfo2.txt`, and two required directories, `install/` and `carplay/`.** Documentation is supplied separately. Files inside these directories are required; do not remove them or copy selected files only.

`MEDIA_ID` 保留旧版发现协议标记是为了让旧入口识别新卡，不代表产品仍是旧版；实际产品及管理组件版本独立核对。请勿手工改写包内版本或校验。

`MEDIA_ID` intentionally retains the legacy discovery protocol so an old entry can recognize new media. Product and manager versions are checked separately. Do not manually change version markers or checksums.

## 中文：完整 SD 安装流程

1. 核对精确固件与仪表。MU1102 支持 790／791，MU1367 支持 790／791，Škoda MU1440 仅支持 790。本轮用户实际通过的是 MU1367 的 SD 安装，不等于其他组合全部实机验收。保留完整系统备份和可用恢复通路。
2. 车辆停稳，保持可靠供电，完全断开手机 USB 与无线 CarPlay。不要在地图包或其他系统更新进行中叠加执行。
3. 下载 `Wireless_CarPlay_Maps_Standalone_v3.2.zip`，使用 `SHA256SUMS.txt` 核对下载文件。解压后把 **`SD_ROOT` 内全部内容**放到卡根，不要再包一层 `SD_ROOT`，也不要与旧包的本项目文件混用。
4. 将 SD 插入车机，从原车“软件更新”选择 SD，按系统流程完成 V3.2 管理入口注册及收尾。成功后项目一项应显示 `OK`。本步骤不刷整套固件，也不等于功能已经安装。
5. 入口成功注册后会开启并保存开发者／绿色菜单开关。进入独立 `wireless-carplay-maps` 页面：790 选择 **Install AID 10.5 - default iPhone 17 Pro Max**，791 选择 **Install AID 12.3 - VW only**；最终以精确固件检查结果为准，不能越过“不支持”提示。
6. 安装分为暂存、完整性检查、备份和提交。中间出现地图阶段完成、文件清单通过，不代表整套功能已完成。始终保留 SD 和稳定供电。
7. 最后应同时看到 `WMAPS_FEATURE_COMMIT_OK`、`WMAPS_STANDALONE_FEATURE_OK`。程序还会说明所有功能文件已校验提交，返回菜单后可以取卡。之后再按流程手动完整重启，连接手机验收。
8. 如出现任何 `FAIL`、恢复复核要求或更新收尾错误，保存完整截图和日志，先不要删除备份、锁目录、事务目录，也不要反复强刷或直接运行内部脚本。

系统软件更新的 `WirelessCarPlayMaps = OK` 只代表入口阶段成功；绿色菜单内的两条功能成功标志才代表功能文件已提交。它们都不能代替真实连接、地图、镜像、卡片、触控、声音与原车功能的逐项验证。

## English: complete SD workflow

1. Confirm the exact firmware and cluster. MU1102 supports 790/791; MU1367 supports 790/791; Škoda MU1440 supports 790 only. This round's user-accepted SD installation is MU1367, not universal hardware acceptance. Retain a complete system backup and recovery path.
2. Park safely, maintain reliable power and fully disconnect USB and wireless CarPlay. Do not overlap this installation with a map-package or another system update.
3. Download the V3.2 Standalone ZIP and verify `SHA256SUMS.txt`. Copy **all contents inside `SD_ROOT`** directly to the card root. Do not add an outer folder or mix files from older project packages.
4. Insert SD and select it in the native software-update interface. Complete V3.2 manager registration and finalization according to the system prompts. The project entry should report `OK`. This is neither a complete firmware flash nor full feature installation.
5. Successful registration enables and saves engineering/green-menu access. Open the dedicated `wireless-carplay-maps` page. Select **Install AID 10.5 - default iPhone 17 Pro Max** for 790 or **Install AID 12.3 - VW only** for 791, always subject to the exact-firmware refusal checks.
6. Installation stages and verifies inputs, saves the baseline and commits the feature. A successful file manifest or map-stage message alone is not final success. Keep SD inserted and maintain power.
7. Wait for both `WMAPS_FEATURE_COMMIT_OK` and `WMAPS_STANDALONE_FEATURE_OK`. The console also confirms verified committed files and that SD may be removed after returning to the menu. Then restart fully as required before connecting a phone.
8. On any `FAIL`, recovery-review request or finalization error, preserve complete screenshots and logs. Do not delete recovery material, lock/journal directories, repeatedly force updates or invoke internal scripts directly.

The native-update `OK` confirms manager registration only. The green-menu success markers confirm feature-file commit. Neither replaces separate hardware checks of connection, maps, mirroring, cards, controls, audio or factory functions.

## 首次无线连接 / First wireless connection

重启后拔下手机 USB，启用本项目的无线 CarPlay 及车机 WLAN 热点。先在 iPhone“设置 → 蓝牙”手动配对车机，再到“设置 → Wi-Fi”加入**车机自身热点**，使用本机页面显示的网络名和密码。两项连接都完成后才启动无线 CarPlay；未自动进入时，在 App-Connect 选择当前手机。

After restart, unplug USB and enable Wireless CarPlay and the head-unit WLAN hotspot. First pair the unit manually in iPhone **Settings → Bluetooth**. Then join **the head unit's own hotspot** in **Settings → Wi-Fi**, using its displayed network name/password. Both connections are required; select the current iPhone in App-Connect if necessary.

不是手机个人热点，不是其他上网 Wi-Fi。不要复制别台车的热点、配对数据、地址或密钥。详见仓库 `docs/WIRELESS_CONNECTION.zh-CN_EN.md`。

Do not use the phone's Personal Hotspot or substitute an external internet Wi-Fi. Never copy another unit's credentials, pairing data, addresses or keys. See the repository's bilingual wireless-connection guide.

## 旧版更新与恢复 / Upgrade and recovery

使用新的完整 V3.2 SD 走软件更新注册，再在绿色菜单更新或安装功能。已经存在本项目入口时，也有 **Update this management entry only** 用于只更新管理组件；它不代替完整 SD 流程的验收。旧入口遇到新卡的安装请求会先衔接完整管理组件，不能仅靠手动补一个丢失的助手文件。

Use complete V3.2 media for native registration, then update/install the feature in GEM. **Update this management entry only** is available for an existing entry but is not a substitute for testing the complete native SD workflow. Legacy installation handoff upgrades the whole manager, not a single missing helper.

保留首次基线；首次基线可能是安装前的旧公开版本，并不一定是原厂。用本版 **Restore saved first baseline - keep this entry** 恢复并保留入口，或用 **Restore baseline and remove this entry** 先恢复再移除入口。不要让旧 SD 的卸载脚本拆除新运行目录。

The first baseline may be an older public installation, not necessarily factory state. Use this manager's restore options to restore it with or without retaining the entry. Do not use an old SD uninstaller against the new runtime.

具备有效记录的中断安装会先核对恢复材料及当前目标，再进行恢复。损坏、未知改动或缺乏可靠旧记录的状态仍会停止并要求复核，不会静默删除备份或宣称成功。

Verifiable interrupted transactions are reviewed against their saved state before recovery. Corrupt, externally modified or historically unsealed unknown states remain stop-and-review cases; backups are not silently discarded.

## 已知问题 / Known issue

VW 返回原厂图标下方的 `Volkswagen` 文字可能为空；该显示问题保留在 V3.2，留待 V3.3／V3.4 后续处理。本次发布不修改已经通过用户 SD 安装测试的包。

The VW return icon may lack its `Volkswagen` caption. This remains in V3.2 and is deferred to a later V3.3/V3.4 revision. The user-tested installer is not changed for this publication.

只发布安装包和说明，不附单独的实现源码包。请安装 Standalone ZIP；GitHub 自动提供的标签快照不是 SD 安装包。

Only the installer and documentation are attached, with no separate implementation-source archive. Use the Standalone ZIP rather than GitHub's automatic repository snapshots.
