# MU1367 — AID 10.5 — V2.1 candidate

Instrument output: **1010×376**.

Includes CarPlay maps, screen mirroring, remembered native-menu settings,
navigation cards and **CarPlay information (experimental)**. The information
choices are All enabled, Navigation off, Music off and All off. The video and
information permissions are independent. Wireless CarPlay remains an
OFF-by-default test-state interface; no wireless service is bundled. Android
Auto/CarLife engines and startup animations are not included.

Navigation card: Center/Left/Right/Off. SEAT-specific safe areas are described in the adaptation notes; never substitute the Volkswagen crop assumptions for SEAT.

本版增加 CarPlay 信息（试验），包括全部开启、关闭导航、关闭音乐、全部关闭，
设置可记忆，与投屏开关独立。无线 CarPlay 仍未发布实际连接服务。
这是开发者集成文件，不是独立刷写实验包；普通用户使用完整传统 MQB Toolbox。
保存本机备份、断开手机并保持稳定供电；只有安装成功后才完整重启。

For developer integration only, not a standalone firmware update. On MMX use
`/bin/ksh ./run-install.sh` from this directory. After `CPMAP_INSTALL_OK`, perform
a complete reboot. Unknown firmware or third-party mm-ipod wrappers are rejected
before activation; use the original restore entry to undo this project.
Offline/API/emulator results are not universal phone/cluster acceptance.

[Full instructions](../../README.md) · [V2.1 candidate notes](../../releases/toolbox-plaintext-v2.1.0.md)
