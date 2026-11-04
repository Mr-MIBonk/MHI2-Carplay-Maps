# CarPlay Maps VC V2.1 — experimental prerelease / 试验预发布版

Version: **2.1.0-rc.7**. **Experimental prerelease, not stable 2.1.**
作者已明确同意带已知风险发布，稳定版 2.0.1 不变。
The author approved experimental publication with the known risks below;
stable 2.0.1 remains unchanged.
真实手机／仪表的信息模块验收尚未完成，尤其是新增的斯柯达／西雅特适配。
离线检查不等于实车验证；稳定版 2.0.1 保持不变。

Physical phone/cluster information-module acceptance is **not complete**, notably
for the added Škoda/SEAT adaptations. Stable 2.0.1 remains unchanged.

## 已知风险 / Known risks — read before installing

**长时间 SEAT G11 虚拟机测试出现过多次 QNX 内核停止，另有一次媒体启动异常，
原因尚未查清。菜单功能测试通过不等于稳定性通过。** 后续历史菜单复现分支
也抓到了内核异常；未证明这些异常具有相同原因，不能认定只是虚拟机问题，
也不能认定已由当前菜单修正解决。本版未完成真实手机／仪表信息模块验收。
只建议具备完整系统备份、SSH／工具箱访问和恢复能力的测试者使用。
如安装报错或出现异常，停止测试并保留日志，不要反复重装。

**Long-running SEAT G11 emulator tests encountered multiple unresolved QNX kernel
stops and a separate media-startup failure. Successful menu tests are not a
stability pass.** Later historical-menu reproduction branches also encountered
kernel faults; neither a shared cause nor an emulator-only cause has been proven.
The current menu-layout fixes do not resolve these faults. Real phone/cluster
information-module acceptance is incomplete. Use only with a full system backup,
SSH/Toolbox recovery access and the ability to restore. Stop on an install/runtime
failure, preserve logs, and do not repeatedly reinstall.

## 社区特别致谢

**重点感谢 [olli991](https://github.com/olli991)** 提供大量开发思路、实际建议，
以及西雅特原车导航、卡片位置、昼夜显示和重启后设置的详细测试反馈。
感谢 [octacon84](https://github.com/octacon84) 提供基于 MU1440 的 MU1447
适配包和社区测试结果。两位的贡献分别记录，详见 [完整致谢](../docs/COMMUNITY_V2_1_CREDITS.md)。

## 本版变化

- SEAT MU1447 使用独立卡片声明，不再套用大众的上下裁切假设。仍输出
  AID 10.5 的 1010×376；中控原厂 G11 UI 仍为 800×480。沿用社区包的滑轮控制。
- 新增 **CarPlay 信息（试验）**：全部开启、关闭导航、关闭音乐、全部关闭。
  与地图／镜像开关独立；设置持久保存，可热切换。基于本地 V53 分别适配五种固件。
- 导航清理、当前信息刷新与音乐监听注册通过原厂 ASL 队列执行，不从后台线程直接调用仪表接口。
  注册成功以原厂接口的实际结果为准，不把加入队列当作注册成功。
  封面关闭清理一次，重新开启可显示同一首歌的封面；保留重复封面去重和断开重放防护。
- 临时封面、缓存和有界日志使用专用内存目录；新增程序和 mm-ipod 原文件纳入
  安装、覆盖更新、失败回退和原厂恢复流程，不替换无关工具的未知包装器。
- 31 种菜单语言保留；G11 窄选项条使用缩写，试验标记不截断。字形仍由原固件字体提供。
- G11 菜单按原品牌原厂选择器重新核对：恢复列表内边距、视口宽度、行距和按钮偏移，
  修正右侧裁切；保留早期斯柯达 G11 的原厂负间距，避免误判导致选项无法展开。
  八项镜像使用连续滚动，不再套用大众大屏弹窗的纵坐标。
- 无线 CarPlay 仍然只是默认关闭的测试接口，**没有发布实际无线服务**。

支持 MU1102、MU1367 的 AID 12.3／AID 10.5；MU1440 Škoda、MU1433 Škoda G11、
MU1447 SEAT G11 仅 AID 10.5。精确门禁不变，MU0033 不支持。
不包含私有 V7.x 安装器、设备密钥或启动动画。独立安装实验包不公开，仍以完整
传统 MQB Toolbox 为发行方式。

## 安装与验收

离线检查覆盖七种配置的安装、更新、损坏补齐、失败回退和原厂恢复；最终完整包
通过九组 Linux 检查，包括从发行 ZIP 解出的脚本执行 147 项事务操作。
西雅特原厂 G11 虚拟机已验证四项信息选择可点击、保存，并在完整停止／启动后
保留设置；没有将虚拟机里不存在的手机和仪表接口计为成功。斯柯达候选盘与
无信息模块对照均在较长启动后进入 SmartLink，英文菜单完整点击验收另行记录。
西雅特候选与历史复现分支的长时会话出现过 QNX 内核停止，根因尚未隔离；不把该项列为
稳定性通过，也不宣称这只是虚拟机或只是新模块的问题。
详见 [检查结果与原厂 UI 图片](../docs/V2_1_VALIDATION.md)。

保存系统备份、断开 CarPlay，车辆停稳并稳定供电。解压完整 MQB 包到 SD 根目录，
从工具箱 CarPlay Maps VC 入口选择真实仪表。只有看到 `CPMAP_INSTALL_OK` 后
才完整重启并连接手机。失败不要反复重装、不要手工删除原厂备份。

预发布版仍需实际验收：导航／音乐四项切换、封面重开、手机反复断开重连、卡片
左／右／中／关闭及重启记忆、原厂恢复。社区之前反馈的 SEAT 昼夜外观和开机
卡片重新生效问题仍需真实会话复核，不能仅凭成功回调宣称已经修复。
虚拟机不模拟真实手机与 MOST 仪表，离线测试不等于实车验收。

## English

This is **2.1.0-rc.7, an experimental prerelease**, not stable 2.1. Physical
phone/cluster acceptance is incomplete. Seven-profile host install/update/repair/
rollback/restore checks pass. Nine final Linux suites include 147 transaction
operations using scripts extracted from the full release ZIP. Native SEAT G11
menu clicks, all four information choices and full emulator stop/start persistence
were checked; unavailable simulated DSIs are not counted as working cluster
output. Both the Škoda candidate and its no-information-JAR control eventually
reached SmartLink after slow startup; final English native-click coverage is
recorded separately. Long-running SEAT candidate and historical reproduction
sessions encountered QNX kernel stops whose causes are not isolated. These are
not stability passes and must not
be dismissed as either an emulator-only or new-module-only fault. See the
[validation report and screenshots](../docs/V2_1_VALIDATION.md).

**Special thanks to [olli991](https://github.com/olli991)** for extensive development
ideas, practical suggestions, and detailed factory-navigation, card, day/night
and restart feedback. Thanks separately to [octacon84](https://github.com/octacon84)
for the MU1447 adaptation package and community test results.

V2.1 adds a SEAT-specific card safe-area table and roller routing, without changing
the 1010×376 cluster output or original 800×480 G11 head-unit UI. It adds the
V53-derived **CarPlay information (experimental)** selector: All enabled,
Navigation off, Music off and All off. These remembered hot switches are separate
from map/mirror permissions. Five exact firmware API builds are supplied.

OEM information calls and music-listener registration use the ASL owning-thread
queue. A queued registration is not treated as successful; its actual result is
returned only to the matching target.
Cover clear/resume, unchanged-track re-emission, cover deduplication and disconnect
replay protection are retained. Bounded runtime files use a dedicated RAM directory.
The original mm-ipod is backed up exactly; install/upgrade/failure rollback/restore
include the new files. Unknown third-party wrappers are not overwritten.

All 31 menu locales remain. Compact G11 labels retain the experimental marker;
glyph availability still depends on the original firmware fonts. Wireless CarPlay
is an off-by-default saved test interface, **not a released wireless service**.
MU1102/MU1367 support both instrument sizes; MU1440/MU1433/MU1447 support AID 10.5
only. MU0033 remains excluded. Only the traditional full MQB Toolbox is intended
for publication; no private V7.x or standalone experiment is included.

G11 selector layout retains each brand's own native padding, viewport width,
row spacing and button offsets. It fixes the clipped right edge and prevents the
eight-choice scrolling popup from inheriting Volkswagen's large-screen y position.
Older Škoda G11 negative item spacing is retained rather than incorrectly rejected.
SEAT's four-item popup was compared directly with its original Fuel consumption
selector; screenshots and the exact validation boundary are linked above.

Back up the unit, disconnect CarPlay and keep stable power. Use the real cluster
profile and fully reboot only after `CPMAP_INSTALL_OK`. Do not test a failed install
with a connected phone. Physical phone/cluster acceptance, including day/night and
boot card reapplication, remains distinct from API, sanitizer and emulator tests.

Contact: [omonob@126.com](mailto:omonob@126.com) · [Telegram](https://t.me/omonob).
