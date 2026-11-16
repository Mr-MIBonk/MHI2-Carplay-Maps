# VW MHI2 Wireless CarPlay Maps

**Wireless CarPlay Maps V3.1 — 试验预发布 / Experimental prerelease**

**V3.1 安装包修订：**修正软件更新入口的下载组名与收尾目录声明，重新计算最终文件校验。请重新下载发行页的 Standalone 安装包及校验清单，完整替换 SD 卡上的本项目文件，不与旧包混用。无线、地图和菜单程序未改动；原厂软件更新界面仍需实机复测。

**V3.1 installer repack:** corrected the download-group identifier and final-script directory declaration, then regenerated integrity metadata from the final files. Download the Standalone installer and checksum list again; replace this project's complete SD contents rather than mixing packages. Wireless, map and menu programs are unchanged. Native software-update acceptance still requires a hardware retest.

让支持的 MHI2 车机连接无线 CarPlay，并把手机地图映射到仪表，让仪表也能显示地图画面。项目同时提供屏幕镜像、导航卡片和方向盘地图缩放，使用原车界面完成设置和切换。

Connect wireless CarPlay on supported MHI2 head units and display the phone's map on the instrument cluster. The project also provides screen mirroring, navigation cards and steering-wheel map zoom through the original head-unit interface.

维护者 / Maintainer：[omonob](https://github.com/omonob)。本项目不隶属于 Volkswagen、Škoda、SEAT、Audi、Apple 或 QNX。

[下载 V3.1 / Download](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/releases/tag/v3.1) · [详细开发历程 / Development history](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/main/docs/WIRELESS_DEVELOPMENT.zh-CN.md) · [中英文安装说明 / Installation guide](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/main/docs/WIRELESS_INSTALLATION.md) · [升级兼容 / Upgrade compatibility](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/main/docs/WIRELESS_COMPATIBILITY.zh-CN.md) · [致谢 / Credits](CREDITS.md)

**首次无线连接：先手动配对蓝牙，再让 iPhone 加入车机的 Wi-Fi 热点，完成两项连接后无线 CarPlay 才会启动。** 仅开启无线开关并不够。[中英文连接步骤 / Connection instructions](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/main/docs/WIRELESS_CONNECTION.zh-CN_EN.md)。

**First wireless connection: manually pair Bluetooth, then join the head unit's Wi-Fi hotspot from the iPhone. Both are needed before wireless CarPlay starts.** Enabling the wireless switch alone is not enough.

本次公开内容为安装包、安装方法、开发历程与实现思路，不提供单独的实现源码包。安装包保留安装、更新和恢复所必需的脚本及运行文件。

This publication provides the installer, installation instructions, development history and engineering explanations. It does not provide a separate implementation-source package. Scripts and runtime files required for installation, updates and recovery remain included.

## 本版能做什么 / What V3.1 provides

- **无线连接 / Wireless connection**：保留 MU1102 的无线服务，并为精确匹配的 MU1367、Škoda MU1440 增加试验适配，后两者仍待实机验收。Retain the MU1102 wireless service and add experimental ports for exact MU1367 and Škoda MU1440 targets; the new targets still require hardware acceptance.
- **仪表地图 / Cluster maps**：手机地图映射到仪表，仪表显示地图画面；中控仍保留正常 CarPlay 使用方式。Display the phone map on the cluster while retaining normal CarPlay use on the main screen.
- **镜像与卡片 / Mirroring and cards**：通过虚拟座舱设置选择地图、镜像和导航卡片，并处理模式间的衔接。Use Virtual Cockpit settings to select map, mirror and navigation-card views.
- **方向盘控制 / Steering-wheel control**：接入本项目的地图放大、缩小操作。Forward the supported map zoom commands.
- **独立安装 / Independent installation**：从 SD 卡注册独立绿色菜单入口，再安装功能、查看状态、更新或恢复。不要求完整 MQB Toolbox。Register a dedicated green-menu entry from SD, then install, inspect, update or restore without the full MQB Toolbox.

本版只保留 **iPhone 17 Pro Max** 手机配置；790 与 791 仍保留各自必要的硬件适配。地图卡片属于地图显示场景，不等于另外安装歌曲文字、封面或导航文字映射。独立信息映射、仪表启动画面及迎宾／离车声音不在本精简版范围内。正常 CarPlay 声音和媒体使用方式仍保留。

Only the **iPhone 17 Pro Max** phone profile is offered, with separate hardware adaptations for 790 and 791. Navigation cards are part of the map display, not a separate song, artwork or navigation-text publisher. Separate information mapping, cluster startup presentations and welcome/exit sounds are outside this lean package. Normal CarPlay audio and media use remain available.

## 本次新增的固件适配 / New firmware ports

V3.1 为 MU1367 的 790／791 仪表组合，以及 Škoda MU1440 的 790 组合加入无线安装与菜单接入。这是按精确固件核对的试验适配，不是按相近版本号统一放行；MU1440 的 791 组合不在本包范围内。

适配时分别对照原厂 HMI 类、蓝牙接口布局、连接管理程序、启动脚本和生产配置。1367、1440 保留各自的原厂连接管理程序，不用 1102 的程序覆盖。安装前还核对关键原生程序，遇到未知修改停止并保留现场。升级原有仅有线的对应配置时，由新管理入口迁移到匹配的无线配置，首次备份继续保留。

原厂配置中的“蓝牙进程入口”和同名“蓝牙参数设置”需要区分。V3.1 只修改唯一、已知的进程入口，保留参数设置；真实固件配置合并和重复执行另行检查，不只依赖简化夹具。

V3.1 adds experimental wireless installation and menu integration for MU1367 with 790/791 and Škoda MU1440 with 790. MU1440/791 is not included. Exact firmware checks remain; similar version numbers are not sufficient.

The port compares native HMI classes, Bluetooth interface layouts, connection-management programs, startup scripts and production configuration. MU1367 and MU1440 retain their own stock connection manager rather than receiving MU1102's program. Unknown native modifications stop installation. The new manager upgrades matching wired-only predecessor profiles while retaining the first baseline. Configuration merging distinguishes the executable Bluetooth launcher from its same-named settings object, leaving the settings intact.

**新增机型的离线检查不等于实车验收。** 请按中英文安装与连接说明分项测试；无线首次使用需要手动配对蓝牙和连接车机 Wi-Fi。

**Offline checks of the new ports are not vehicle acceptance.** Follow the bilingual installation and connection checks. First use requires manual Bluetooth pairing and connection to the head-unit Wi-Fi hotspot.

## 开发时间线 / Development timeline

项目不是从添加一个“无线”按钮开始，也不是在某次连接成功后就结束。早期先解决连接与显示，之后继续处理原厂界面、触控、仪表接入、有线／无线切换以及交付恢复。

The work went beyond adding a wireless button. Early development established connection and display; later work integrated native controls, the cluster, wired/wireless transitions, installation and recovery.

| 时间 / Date | 进展 / Milestone |
|---|---|
| 2026-08-11 | 立项，开始探索在 MHI2 上接入无线 CarPlay。Project started; initial feasibility work. |
| 2026-08-17 | 早期研究取得阶段性进展。Early development made progress. |
| 2026-08-18 | 早期链路跑通，继续处理完整使用流程。The initial path worked; end-to-end integration continued. |
| 2026-08-25 | 在已有项目和交付材料上接手推进，开始系统定位连接、显示和原厂交互问题。Further research and structured diagnosis began from the existing project and handoff. |
| 2026-08-29 | 取得新的关键进展，后续研究继续围绕可重复运行、正确切换和可恢复交付展开。A further milestone led into continued work on repeatable operation, transitions and recovery. |

以上早期节点依据团队回顾整理，不是用后来的构建日期反推历史。早期项目由 CloudCode 辅助开展，期间也有过“81 次修改”这样的其他探索线；它只说明尝试过程，不代表已经成功的基线，也不是本次正式版本号。本次发行统一为 **V3.1**，技术结论以最终保留的实现和对应测试结果为准。

These early milestones come from the team's account, not reconstructed commit dates. CloudCode assisted the early project. A separate line of work went through “81 revisions”; that describes exploration, not a successful baseline or the release number. This release is **V3.1**. Source-header dates, build dates and hardware acceptance dates are different records.

## 我们怎样实现无线接入 / How wireless integration works

有线 CarPlay 已经有 USB 设备识别、会话建立、音视频和退出入口。无线接入需要补上发现、引导和网络会话，同时继续与原厂 CarPlay 界面、显示和输入系统协同。不能只把设备名称改成“无线”，也不能用一个“已连接”标志代替整个链路。

Wired CarPlay already has USB discovery and a native session path. Wireless integration adds discovery, bootstrap and a network session while keeping the original CarPlay display and input systems coordinated. Seeing a device card is not the same as accepting a session, and receiving audio does not prove that video is ready.

```text
iPhone
  ├─ 蓝牙引导 / Bluetooth bootstrap
  │    └─ 原厂蓝牙进程与限定接入层 / Native Bluetooth process and scoped integration
  └─ WLAN 会话 / WLAN session
       └─ 无线接收端 / Wireless receiver
            ├─ 中控 CarPlay 画面和声音 / Main-screen CarPlay video and audio
            ├─ 原厂触控与控制回传 / Native touch and control return path
            └─ 仪表地图、镜像和卡片 / Cluster map, mirror and card views
```

### 蓝牙负责引导，网络负责会话 / Bootstrap and network session

接入层包装原厂蓝牙进程，而不是替换整套蓝牙系统。它使用本机当前热点信息引导手机，不把其他车的热点密码或配对数据库带进安装包。只属于蓝牙进程的加载环境不能扩散到接收器或视频进程，否则同一个库可能在错误的程序里运行。

The integration wraps the stock Bluetooth process instead of replacing the whole Bluetooth system. It uses the current unit's hotspot information rather than shipping another unit's credentials or pairing database. Process-specific loading settings must not leak into unrelated child processes.

热点已打开，也不一定意味着接口、实际信道、网络规则和接收服务都已就绪。因此开发中分别检查网络条件、监听状态和真正的会话建立。原厂网络管理还可能在状态变化时重新加载规则；这也是“看起来一直在连接”不能只按蓝牙故障处理的原因。

An enabled hotspot does not mean the interface, current channel, network policy and receiver are all ready. Network readiness, listening state and session establishment are checked separately. Native network management can reload rules, so a connection spinner is not automatically a Bluetooth failure.

### 中控与仪表分工，控制回到正确会话 / Display paths and session ownership

接收程序收到媒体后，还需要视频程序解码并提交到原厂显示路径；原厂界面需要进入正确状态，触控也要回到当前手机会话。仪表地图、镜像和卡片是继续接入的衍生通路，不能因为中控可用，就默认仪表各项都已接通。

Received media still needs decoding, submission to the native display path and the correct HMI state. Touch must return to the current phone session. Cluster maps, mirroring and cards are separate integration points; a working main screen alone does not verify them.

地图位置也不是随意移动一个窗口。编码画面、物理显示描述、可见区域与安全区域要相互匹配，地图和卡片切换不能沿用过期状态。不同仪表、不同品牌的显示边界需要分开核对，不能把一台仪表的结果直接套到另一台上。

Map placement requires consistent encoded, physical, visible and safe-area descriptions. View changes must not reuse stale state. Different cluster and brand boundaries require separate checks rather than copying another target's layout assumptions.

## 开发中遇到的问题与处理 / Problems encountered and their resolution

下面记录的是这条开发线的重要问题及处理思路。已验证的场景与仍待实机复核的边界分别说明，不把历史单项成功扩大成全部机型已经验收。

The following are key problems encountered during development. Verified scenarios and remaining hardware boundaries are kept distinct; a successful individual test is not universal acceptance.

### 连接转圈，但主屏没有画面 / Connection spinner without main-screen video

曾出现会话已经开始，但主屏等待画面而无法完成正常进入的情况。定位时不是先延长超时，而是对照主屏与仪表的视频归属，发现共享分类状态可能被另一条显示通路污染，导致媒体送错位置。

处理重点是把这类状态限制在正确的线程和会话范围内，同时对接口形态与接收程序做精确校验。普通调用继续走原有实现，不伪造会话，也不凭空生成画面。

A session could start while the main screen still received no usable video. Diagnosis compared the main and cluster paths instead of merely extending a timeout. Shared classification state could be polluted by another path. The correction scoped state to the appropriate thread/session and retained exact receiver-interface checks.

### 地图偏移或比例不正确 / Map offset or distortion

导航开启前后，画面曾出现位置跳动和比例异常。我们将物理显示描述、可见区域与地图入口声明逐项对齐，而不是简单裁掉画面掩盖问题。已经验证的组合保留其结果；个别仪表的卡片位置仍作为独立问题继续核对。

Map position could change when navigation started, with visible distortion. Physical, visible-area and map-entry descriptions were aligned rather than hiding the mismatch through cropping. Verified combinations were preserved; remaining card-placement cases stay separate.

### 自定义菜单能打开，却点不了 / Menus opened but selections did not work

页面出现不代表触摸链路已经正确。下拉层、选择列表和页面控制器如果没有按原厂生命周期接入，可能出现遮罩残留、列表吞输入或返回后状态不同步。

这部分回到原生列表、选择器和页面控制器处理：进入时正确绑定，选择后更新状态，退出时释放本页监听和临时层；菜单响应、页面过渡和触摸选择分开验证。

A visible page did not guarantee working input. Popup layers, list controls and page controllers needed the native lifecycle. The correction used native selectors, explicit binding/state updates and cleanup on exit, with separate checks for entry, transitions and selection.

### 产品版本与仪表型号不能混用 / Release names are not hardware profiles

发布核对还发现，旧菜单曾把产品的 `VERSION` 文件当成仪表型号读取。独立包使用产品版本标记后，菜单可能把它判为未知配置。现在改为优先读取专用的 `PROFILE`，与已确认的仪表配置核对；只对真正包含旧仪表标识的历史值保留兼容。

此修订通过了九十二项分类、缺失、冲突和历史兼容检查。它不靠改写产品版本来伪装仪表，也不根据手机画布猜测型号；菜单实际触屏及整机运行仍属于后续硬件验收。

Publication checks found that an older menu read the product `VERSION` file as if it identified the cluster. It now prioritizes the dedicated `PROFILE` and checks it against the confirmed cluster, with fallback only for genuine historical hardware identifiers. Ninety-two classification, missing-input, conflict and legacy cases pass. Real touch and unit acceptance remain separate.

### 看见手机卡片，不等于已经接受连接 / Device presence was mistaken for connection

设备被发现、用户点击连接、会话被接受和手动断开是不同事件。早期状态混淆会让仪表过早接管，或在用户主动断开后又被后台逻辑拉回。

处理时明确当前选择的设备和当前已接受会话，再决定是否允许仪表接入。连接提示的处理也绑定这次选择事件，而不是只按固定文字拦截，以免切换分支后旧提示再次出现。

Discovery, user selection, session acceptance and manual disconnect are separate events. Instrument ownership is granted to the accepted session, not merely a discovered device. Connection-prompt handling follows the selected device/event rather than matching a fixed label.

### 无线有画面，仪表镜像却没有启动 / Wireless video existed but cluster mirroring did not start

排查中确认无线视频已经在产生，旧启动条件却仍要求有线投屏进程存在。无线模式切换时，这个条件就会把正常的镜像启动挡住。

修订后依据有效无线会话判断启动衔接，不伪造有线进程，也不改变地图位置和已经通过的信息通路。镜像显示、方向盘画布操作和切回地图分项验证。

Wireless video was present, but the old startup condition still required the wired projection process. The scheduler was updated to recognize the valid wireless session without fabricating a wired process or changing the established map geometry. Mirror display, controls and return-to-map were tested separately.

### 镜像切回地图后，窗口恢复但地图未显示 / Map did not return after leaving mirroring

模式文字切回去，不代表消费者已拿到能够继续解码的地图流。修订把恢复动作绑定到有效模式边沿，向正确会话请求重新建立可用的地图输入，不依赖用户碰一下手机才恢复显示，也不反复重启整个接收器。

Changing a menu value did not guarantee that the map consumer had usable input again. Recovery was tied to the valid mode transition and the correct session, rather than waiting for user interaction or repeatedly restarting the receiver.

### 有线与无线切换后，共享输入状态丢失 / Shared input state broke across transport changes

有线与无线可以复用上层功能，但两边创建和释放输入的时机不同。此前出现过进程仍在、输入却被另一条路径替换的情况，因此需要处理文件与会话的所有权、输入重新打开及订阅恢复，不能只看进程是否活着。

这些切换经验用于完善接入和恢复。V3.1 仍按前述精简范围提供功能，不把更早完整研究线中的所有独立信息模块都装入本包。

Wired and wireless paths can share higher-level functions, but their inputs have different lifetimes. A live process could still be attached to a replaced input. Ownership, reopening and subscription recovery were handled explicitly. V3.1 retains the lean scope above, not every information module explored in earlier work.

### 新版本可以安装，不代表旧版可以安全覆盖 / Clean installation was not enough

兼容性检查发现，旧信息模块的媒体包装器可能仍留在系统外部，但它引用的原程序已经随着项目目录替换而消失。只删除旧目录，会留下不可用的调用链。

升级改为先识别已知包装器、核对保存的原生程序，再恢复它；同时退出已知旧信息 JAR 和启动引用。缺少可靠备份、遇到未知包装器或其他项目改动时停止，不从另一台车拿程序硬顶。

首次基线表还增加了完整性校验、缺失文件状态和失败回退。旧版本留下的备份不能被追认成“当时已校验”，旧卸载脚本也不能拿来拆新版本。

Historical wrappers could survive outside the replaced project directory while losing their saved executable dependency. Upgrade handling now recognizes known wrappers, verifies and restores the saved native executable, and retires known information JARs/references. Unknown modifications or unreliable backups stop the operation. First-baseline integrity, missing-file receipts and rollback are checked explicitly.

## 采用的验证方式 / Validation approach

我们保留已经通过的功能基线，每次只改一个主要问题，先保存故障现场，再做隔离测试。诊断修订若引入黑屏或声音异常，会先回退并确认恢复，而不是继续叠加补丁。

历史升级测试不是只改一个 VERSION 文本：先用实际旧安装器生成旧状态，再运行新版安装、重复更新、恢复和卸载。前期已建立覆盖九个历史公开版本、四十九个版本／机型组合的离线检查；V3.1 继续检查当前安装流程和已发布前代的迁移。

文件清单、压缩包回读、恢复快照、目标命令可用性和其他项目改动都有对应检查。离线测试只证明它实际覆盖的安装和代码路径，真实手机、仪表、触屏、网络与长期运行仍需要对应实机结果。

Known-good baselines are preserved and major changes are tested separately. Fault evidence is saved before editing; a diagnostic regression is rolled back before further changes. Historical tests run actual old installers, then upgrade, repeat, restore and uninstall. The established matrix covers nine public historical releases and forty-nine release/profile combinations. V3.1 also checks current installation and migration from its published predecessor.

Manifests, archive readback, recovery snapshots, required target commands and unrelated-file changes are checked. Offline tests do not establish real-phone, cluster, touch, radio or long-term vehicle acceptance. [验证记录 / Validation records](docs/WIRELESS_VALIDATION.zh-CN.md).

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

这里连接的是**车机自身热点**，不是手机个人热点或外部上网 Wi-Fi。后续再分项检查中控画面、声音、仪表地图和方向盘缩放。完整步骤与连接排查见[中英文连接说明](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/main/docs/WIRELESS_CONNECTION.zh-CN_EN.md)。

Join **the head unit's own hotspot**, not the phone's Personal Hotspot or an external internet Wi-Fi network. Check main-screen video/audio, cluster maps and steering zoom separately. See the [bilingual connection guide](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/blob/main/docs/WIRELESS_CONNECTION.zh-CN_EN.md) for the complete steps and troubleshooting.

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

## 当前边界与后续工作 / Known limits and next work

V3.1 按试验预发布提供。仍需复核个别首次无线断连、790 卡片位置、实际使用流畅度、自定义网络策略共存及新精简组合的完整实机使用。Home 后后台画面暂停、静态页面返回延迟和倒车影像到仪表仍暂缓；USB 未拔除时，从主动断开的有线再次选择无线仍存在限制。

V3.1 remains experimental. Individual first-connection drops, 790 card placement, real-use smoothness, custom network-policy coexistence and complete hardware acceptance of the lean combination still need verification. Background display pause after Home, delayed return from static pages and reverse-camera-to-cluster work remain deferred. Re-selecting wireless after manually disconnecting wired CarPlay while USB remains attached still has a limitation.

后续继续扩大精确平台覆盖，并按“保存现场、隔离改动、分批验证、保留回退”的方式推进。历史标签与原始交付保留，不能为了新版本命名把历史记录改写成 V3.1。

Further work will expand exact-platform coverage while preserving fault evidence, isolated changes and rollback. Historical tags and original handoffs remain historical; they are not relabeled as V3.1.

## 致谢 / Credits

特别感谢 **[olli991](https://github.com/olli991)** 提供大量开发思路、原车行为对照、测试反馈及独立绿色菜单方向；感谢 **[octacon84](https://github.com/octacon84)** 提供 SEAT/MU1447 素材、布局工作与社区测试结果。

Special thanks to **olli991** for extensive development ideas, factory-behavior comparisons, testing feedback and the independent green-menu direction. Thanks to **octacon84** for SEAT/MU1447 materials, layout work and community testing.

omonob 负责本阶段继续研究、集成、修订与交付维护。真实上游、原厂及 SDK 归属保留。[完整致谢与来源 / Full credits and provenance](CREDITS.md)。[历史发布 / Historical releases](https://github.com/omonob/VW-MHI2-Wireless-Carplay-Maps/releases) 保留各自版本和安装方法，不与本版独立安装方式混用。
