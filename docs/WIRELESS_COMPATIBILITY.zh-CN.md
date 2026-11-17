# Wireless CarPlay Maps V3.2：固件与旧版兼容 / Compatibility

## 当前结论 / Current findings

本轮用户报告：MU1367 通过 SD 软件更新注册及功能安装。随后读取的管理入口和运行版本均为 V3.2。这个结果不等于 MU1367 所有仪表、所有功能和长期运行均完成验收，也不能替代 MU1102／MU1440 的本版实机复测。

The user reports successful MU1367 SD registration and feature installation. Read-only manager/runtime version checks identify V3.2. This does not certify every MU1367 cluster, function or long-duration case, nor replace a fresh V3.2 hardware retest on MU1102/MU1440.

| 精确固件 / Firmware | 组合 / Profile | V3.2 状态 / Status |
|---|---|---|
| MU1367 | 790 / AID 10.5 | 用户 SD 安装通过，离线验证通过 / User SD installation accepted; offline pass |
| MU1367 | 791 / AID 12.3 | 离线验证通过；此组合待实机验收 / Offline pass; hardware acceptance pending |
| MU1102 | 790、791 | 离线验证通过；本轮未实机复测 / Offline pass; no fresh hardware retest |
| MU1440 Škoda | 790 | 离线验证通过；本轮未实机复测 / Offline pass; no fresh hardware retest |
| MU1440 Škoda | 791 | 不支持 / Unsupported |

1367 与 1440 的连接管理程序保留为本机原程序；1102 只采用它自己已匹配的配置。检查固件识别、原生程序散列、仪表组合与配置生成，不能按相近版本号直接放行。未知修改必须保留并停止。

MU1367/MU1440 preserve the unit's own connection manager; MU1102 uses its separately matched profile. Firmware identity, native-program hashes, cluster and configuration generation are checked rather than allowing neighboring version numbers. Unknown changes remain stop-and-preserve cases.

## 安装修订覆盖 / Installation revision coverage

V3.2 将管理入口的 17 个关键组件作为完整清单核对，兼容旧入口发现并衔接新 SD；入口替换与功能提交分开记录。原厂更新收尾、完整节点名路径、开发者模式保存、系统可选工具缺失、实际小分区空间、中断后的可靠恢复、重复安装及首次基线保留分别检查。

V3.2 checks 17 critical manager components as a bundle. Legacy discovery/handoff, manager replacement, native finalization, qualified-node paths, saved green-menu access, optional-tool absence, realistic partition sizes, interruption recovery, repeated installation and first-baseline preservation are covered separately.

离线记录包含 419 项基本生命周期断言、49 个历史组合的 1127 项断言、29 个管理入口及旧恢复状态场景、36 个功能中断场景、16 个 RCC/MMX 收尾场景，以及三组小分区／缺少系统工具的检查。这些是合成文件系统或主机辅助程序测试，不是在三个真实车机上执行 ARM 业务运行的证明。

Offline records cover 419 basic assertions, 1127 assertions across 49 historical combinations, 29 manager/legacy-recovery cases, 36 feature-interruption cases, 16 RCC/MMX finalization cases and three constrained-partition/tool profiles. They model installation logic, not ARM media execution on three physical head units.

新版使用完整 V3.2 介质先注册或更新管理组件，再安装功能。已封存且可核验的中断记录可以先恢复；无可靠历史记录、损坏备份或外部改动不能自动猜测修复。首次基线不是必然原厂，不能丢弃旧恢复档或使用旧卸载器拆除新版。

Use complete V3.2 media to register/update the manager before features. Sealed, verifiable interrupted state can be recovered first. Unknown historical state, corruption or external changes are not guessed away. Preserve the first baseline and older recovery archives; do not run an older uninstaller against the new runtime.

## 当前已知显示问题 / Known display issue

VW 原厂返回图标可能不显示 `Volkswagen` 名称；用户要求留待 V3.3／V3.4，V3.2 不包含该项未验收的修改。首次无线连接个例、特定地图／卡片位置及其他车型的长期运行，仍按下面历史记录中的边界分别追踪，不能因安装成功就写成全部解决。

The VW return icon may lack its caption; this is explicitly deferred to V3.3/V3.4. Isolated first-connection behavior, specific map/card layouts and other targets' long-duration behavior remain separate acceptance items, not automatically solved by successful installation.

## 历史排查记录（截至 V3.1）/ Retained history through V3.1

以下保留此前的问题与验证方法；其中旧版的“本轮”“新增”“待复测”指当时记录。当前 V3.2 的状态以上方为准，不能把历史离线记录读成当前所有机型的实机结论。

The retained record below describes its original stage. Current V3.2 evidence is summarized above; historical offline claims are not current universal hardware acceptance.


状态：试验预发布的离线兼容性记录。此次验证未连接或操作车机。离线验证范围与实车验证范围分别记录，本文不承诺“没有任何未知问题”。

## 排查边界

检查对象包括：可取得的历史公开安装包、历史 Git 标签、此前交付的本地 R1、此次独立 SD/GEM 安装器，以及保留的无线启动脚本和接口适配层。

## V3.1 新增固件与旧版覆盖

MU1367/790、MU1367/791 和 Škoda MU1440/790 新增无线试验适配；仍待对应实机验收。MU1440/791 不支持，MU1433 与 MU1447 仍沿用仅有线范围。

从旧的仅有线配置更新时，先使用新介质更新管理入口，再更新功能。新入口会将对应的已保存配置迁移为无线配置，不改仪表编码，也不替换首次备份。V3.0 与更早已知公开版都保留在升级识别范围中。

1367、1440 的原厂连接管理程序与 1102 不相同，本包保留它们各自的原程序。安装器在写入前核对已知蓝牙及连接管理程序，遇到未知版本、其他项目修改或不匹配配置时停止，不直接跨固件复制程序。

首次无线使用需要手动完成蓝牙配对和手机连接车机 Wi-Fi 热点；详见[中英文连接方法](WIRELESS_CONNECTION.zh-CN_EN.md)。

历史首版采用从项目发布页取得的文件，并与相应 Git 标签记录中 **MQB 安装包那一项**的 SHA-256 对照；其他版本保留实际归档散列或标签提交。没有只根据文件夹名称判断版本。

参考发布页：[首版](https://github.com/omonob/VW-MHI2-Carplay-Maps/releases/tag/toolbox-plaintext-v1.0.0)、[2.0.1](https://github.com/omonob/VW-MHI2-Carplay-Maps/releases/tag/toolbox-plaintext-v2.0.1)、[2.1 rc.7](https://github.com/omonob/VW-MHI2-Carplay-Maps/releases/tag/toolbox-plaintext-v2.1.0-rc.7)。最终覆盖组合及测试计数以随包机器报告为准。

## 已发现并修订的升级问题

| 项目 | 旧行为/风险 | 当前处理 |
|---|---|---|
| V2.1 媒体进程包装器残留 | 精简版替换项目目录后，旧 `mm-ipod` 包装器仍指向已被删除的程序，可能使有线 CarPlay 无法启动。 | 识别精确的已知公开包装器；验证保存的原生程序，再原子恢复。缺失或未知包装器不猜测覆盖。 |
| 旧信息 JAR 与类路径 | 只删运行目录，旧 Java 信息覆盖类仍可能被启动脚本加载。 | 只对已知版本、匹配固件且散列一致的本项目信息 JAR做退出处理，同时去掉其启动引用；其他未知文件保留并拒绝继续。 |
| 旧 1.x 版本标记 | 早期公开包内部使用的标记不同，原 R1 入口无法识别。 | 按历史资料建立精确版本表，不以宽泛的“看起来像公开版”前缀放行私人或自定义版本。 |
| 旧 R1 更新器固定文件列表 | 直接删掉旧脚本名，会导致它在更新管理入口时失败。 | 保留一个旧文件名兼容转发壳，不保留旧手机声明、接收器或菜单选项。 |
| 旧 R1 手机选择 | 保存过旧配置的 R1 不能简单变成“不认识的配置”。 | 更新时明确迁移到当前唯一的 iPhone 17 Pro Max 配置；不同仪表型号仍保留必要的独立适配。 |
| 旧独立管理入口 | 旧入口的卸载脚本可能拆掉新运行目录，却不知道如何恢复无线配置。 | 仅在旧独立入口的 OWNER、脚本和 ESD 均匹配时，替换为迁移提示；恢复首次基线时可恢复原入口。不删除完整 MQB 工具箱。 |
| R1 首次备份覆盖范围不足 | 新增迁移目标不在旧备份表中，直接追加可能破坏回退。 | 使用独立候选备份表补齐范围，验证后再提交；更新失败时恢复原表和原运行目录。 |
| 已删除目标的后续变化 | 若只记录“存在的文件”，别人后来重建一个已删文件，恢复时可能被静默覆盖。 | 最终回执也记录应当不存在的目标；出现新文件时停止恢复/更新，保留现场。 |
| 备份完整性 | 仅检查零散目标，不能证明整个保存的运行目录仍完整。 | 新快照校验文件内容、运行目录清单和索引；额外文件、符号链接、损坏清单或缺失校验信息均有检查。旧 R1 无历史校验的备份不能被追认成“当时已校验”。 |
| 原生载荷复制 | 文件存在不等于复制内容正确。 | 对暂存的原生运行文件和保存的原程序逐项比对；错误原生备份拒绝接入。 |
| 中断与遗留目录 | 中断可能错误返回成功，或清理时误删前次留下的恢复材料。 | 信号走非成功退出及回退；只清理本次创建的暂存目录，未知恢复阶段要求人工检查。 |
| 其他项目改动 | 旧安装完成后，启动脚本可能已被别的项目修改。 | 对有本项目回执的版本先比较当前目标；发现变化先停止，不擅自覆盖。 |
| 固件命令差异 | 主机离线环境有的文件工具，精简 MMX 镜像未必提供。 | 事务先检查必需命令，缺少时明确停止，不进入文件写入。本包不把主机工具或不匹配的工具箱程序复制到车机补缺。 |

### R1 已发生不完整升级时的边界

如果 R1 曾覆盖了 V2.1，导致外部包装器还在、当前目录中的原程序已经没有，新版本会检查 R1 保存的首次快照。只有快照中的原程序存在、结构有效且没有相互冲突的备份时，才准备恢复。

这不是从其他车提取一个程序来顶替，也不会改密码、SSH、K1/K2、FEC 或仪表编码。没有可靠备份时，应停止并保留现场。

### 使用旧包恢复不等于正确回退

新版本安装后，不应再用旧 SD 卡上的 CarPlay 项目卸载脚本做恢复。它不认识新版本的全部目标。应使用新入口的“恢复首次保存基线”，或者经过另外核对的完整备份恢复方案。

覆盖安装完成后必须完整重启，再连接手机验收。磁盘文件完成迁移，不代表旧 HMI/信息进程已从内存卸载；本包不会为了热测而按名称粗暴停止共享系统服务。

## 本轮仍不能离线宣布解决的事项

| 项目 | 已看见的代码/现象 | 当前结论与下一步 |
|---|---|---|
| 自定义防火墙共存 | 保留的无线启动/规则脚本会选择并重载原厂规则文件；有些路径还有针对既有台架网络的固定基底加载。 | 不能宣称任意自定义 PF、VPN 或其他网络项目均无损兼容。涉及此类平台应先审查运行时规则，不能把本包直接当成通用网络补丁。 |
| 首次无线连接后自动断连再恢复 | 既有个例反馈；启动代码还涉及热点稳定、规则重载及重复启动观察器。 | 不能根据一份静态代码或删除 Wi-Fi 列表就确定唯一根因。需要同一次冷启动的接口、规则、接收进程及会话边沿日志。未写成已修复。 |
| 观察器重复调用 | 启动观察器与接收程序的幂等检查顺序存在需要进一步核对的路径，共用临时文件也值得关注。 | 属于代码层风险，不等同于已证明它导致某台车断连。避免把未经实车回归的启动策略改动混进本批兼容修订。 |
| Home 后后台视频暂停 | 保留最后一帧与持续渲染不是一回事。 | 先前明确暂缓；仍不宣称已解决。 |
| 静态画面返回中控延迟亮屏 | 动态页面与静态页面的恢复表现不同。 | 仍需独立处理，不与基本无线连接成功混为一谈。 |
| 仪表卡片位置及实际流畅度 | 具体仪表、场景与手机组合需要实际显示验证。 | 本轮只收敛手机配置与验证安装路径，不把它写成全部位置和性能问题均已实车解决。 |
| 其他 MHI2/MHI2Q 平台 | 接收端接口、原厂 HMI API 与显示边界不同。 | 无线引擎仍按已明确支持的平台限制；不能因为同品牌或相近固件号就放行。 |
| 源码与二进制覆盖范围 | 原交付包含大量源码和记录，但部分冻结接收器/SDK/OEM 依赖仍不能完整对应重建。 | 如实保留依赖清单和真实归属。公开前继续核对分发范围，不把所有依赖改称新原创代码。 |

## 离线验证方法

历史测试不是只改一个 VERSION 文本。测试使用实际取得的旧安装脚本与对应载荷，在合成的原厂文件系统中先完成旧安装，再运行新版安装、重复更新、恢复与卸载。

首版要求预先存在导航共存补丁及类路径，夹具会明确准备这个历史前置条件；不会把“首版不能从纯净环境直接运行”误判成新版升级失败。

测试还覆盖：已知包装器缺少原程序、未知信息 JAR、机型不匹配、其他项目改动、恢复材料损坏、旧管理入口迁移、回执要求缺失文件却被重建，以及扩展首次备份提交失败的回退。

夹具只在自己的临时目录里运行，使用主机版本的只读/校验辅助程序，替换了夹具范围内的 `sync`，并拦截不应出现的系统命令。这些离线测试没有连接车机、启动真实 QNX 媒体程序、改变宿主网络或重启共享虚拟机。

离线通过所证明的是安装文件、识别、迁移和回退逻辑，不是新精简组合的手机音视频、触屏、GPU、真实显示效果或长期运行已经验收。
