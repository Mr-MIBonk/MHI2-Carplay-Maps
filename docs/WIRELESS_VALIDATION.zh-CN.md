# Wireless CarPlay Maps V3.0 验证记录 / Validation

## 版本与文档 / Version and documentation

当前发行名称为 **V3.0**，标签为 `v3.0`。首页、下载文件名、安装器显示和包内当前版本一致。公开交付为安装包和说明，不附单独的实现源码包。历史兼容标记、备份格式、原厂／库版本及原始测试报告不冒改成新产品版本。

The current release is **V3.0**, tagged `v3.0`. Public names, archive filenames, installer display and current package version agree. The public delivery contains the installer and documentation without a separate implementation-source package. Historical compatibility markers, snapshot formats, vendor/library versions and original reports are not relabeled.

首页直接展开项目时间线、接入分工、开发问题及处理；安装与目录说明中英文均提供。软件运行所需配置保持原有已核对策略，本文档修订不改变原生接收、显示、地图位置或按键程序。

The README expands the timeline, architecture, problems and fixes. Installation and layout guidance are bilingual. This revision retains the established native receiver, display, geometry and control programs.

## 当前检查 / Current checks

- 当前七种机型配置：**355 项离线断言通过**，包含安装、管理入口更新、功能更新、恢复、卸载及故障处理。
- 历史升级检查：**994 项离线断言通过**，覆盖九个旧公开版本的四十九种组合，并额外检查前一个已发布无线包的七种配置升级到 V3.0、重复更新和恢复。
- 版本改动后的完整性清单及软件更新绑定重新计算、校验；最终 ZIP 逐文件回读并重新解压验证。
- 原生可执行文件和库与前一个已核对包逐项比对，未因版本命名或说明文字而替换。

Seven current profiles pass **355 offline assertions**, including installation, manager/feature updates, restore, uninstall and fault handling. The historical suite passes **994 assertions** across forty-nine combinations from nine earlier public releases, plus seven migrations from the published wireless predecessor to V3.0.

Manifests and SWDL bindings are regenerated and verified; final archives are read back and re-extracted. Native programs/libraries are compared with the previous checked package and are not replaced by this labeling/documentation revision.

## 边界 / Limits

这些是隔离主机文件系统里的安装与代码检查，没有连接车机、重启车机或把不存在的真实手机和仪表接口当作成功。新精简组合的完整实机验收、网络、触屏及长期运行仍以现场结果为准。

These are isolated host-filesystem installation/code checks. No unit was connected or rebooted, and missing real phone/cluster interfaces were not counted as successes. Full hardware, radio, touch and long-term acceptance remain separate.

原始 R2 和早期报告保留当时的构建时间、版本及结论；本次记录单独存放，避免把历史证据改写成 V3.0 已实车验证。

Original R2 and earlier reports keep their historical dates, versions and conclusions. Current V3.0 evidence is separate rather than rewriting those records as vehicle acceptance.

## 菜单配置识别 / Menu profile identification

MU1102 菜单改为读取独立 PROFILE，不再把产品版本号当仪表型号；实际交付的 Java 辅助类通过 92 项检查，覆盖当前、历史、缺失和冲突配置。本次只变更该 JAR 成员，原生接收与地图程序不变；以上完整安装回归已在更新后的包上重跑。

The MU1102 helper reads the dedicated PROFILE rather than parsing product versions as hardware identity. The shipped class passes 92 cases; only this JAR member changes. Full installation regression above is rerun with the updated package. Native receiver/map binaries are unchanged.
