<div align="center">

# CarPlay Maps VC for MHI2 (mathsls fork)

**Turn-by-turn card on the virtual cockpit, no more freezes after 20 minutes, MU1161 support.**<br>
**Cartão de manobra no painel digital, sem travamentos depois de 20 minutos, suporte à MU1161.**

[English](#english-fork) · [Português](#português-fork) · [Upstream README (omonob)](#upstream-readme-omonob)

</div>

This is a fork of [omonob/MHI2-Carplay-Maps](https://github.com/omonob/MHI2-Carplay-Maps) v1.2.2. All credit for the project goes to omonob; this fork only adds the changes below, under the same GPL-3.0 licence.

## English (fork)

### What changes

| Change | Result | Clusters |
| --- | --- | --- |
| CarPlay Maps logs no longer written to the RAM disk | The cockpit stream no longer freezes after about 20 minutes of driving | All |
| Cluster URL `maps:/car/instrumentcluster` instead of `…/map` | Waze, Google Maps and Apple Maps draw their turn card (arrow, distance, street) on the cockpit map | 791 |
| New safe area and physical size | Card centred at the top, below the clock bar; car marker centred near the bottom | 791 |
| `libcardshim` | Translucent "liquid glass" card style; the cluster is announced to iOS as an instrument cluster | 791 |
| diasdio's MOST bridge fix, keyframe every 5 frames, 40 fps | No lag or pixelation | 791 |
| MU1161 firmware (`lsd.jxe` 55,830,309 bytes) | Install on MU1161 units | 791 |

Why the freezes happened: the project writes three diagnostic logs to `/ramdisk`, a 20 MB RAM disk that also holds the 16 MB video ring. With a keyframe every 5 frames the video log grows about 4 KB/s and the MOST bridge log about 1 KB/s, so the RAM disk was 98 % full after about 20 minutes. Details and offsets: [`src/fork/README.md`](./src/fork/README.md).

### Compatibility

| Firmware | `lsd.jxe` size | 791 AID 12.3 | 790 AID 10.5 |
| --- | ---: | :---: | :---: |
| MU1102 | 55,915,411 bytes | ✅ all changes | ✅ log fix only |
| MU1161 | 55,830,309 bytes | ✅ all changes, tested in the car | ❌ |
| MU1367 | 55,840,541 bytes | ✅ all changes | ✅ log fix only |
| MU1440 Skoda | 55,840,933 bytes | ❌ | ✅ log fix only |

Tested by me only on MU1161/791: VW Passat B8 2018, Discover Pro, iPhone over USB, Waze and Google Maps. MU1102 and MU1367 use the same 791 files, but I have not tested them myself. Reports are welcome in the issues.

### Known limitations

- Waze does not show the ETA bar on the cluster; iOS hides it.
- The card size is fixed by iOS; a smaller safe area does not make it smaller.
- Waze locks the cluster map zoom (steering-wheel zoom works in Google Maps and Apple Maps).
- The maneuver is not shown in the cockpit's own VW graphics; the card is part of the CarPlay video.
- Declaring 60 fps did not help in my test (slight lag in turns). tomasz-twardzicki measured 25 to 27 frames per second delivered by the iPhone with 40 declared.

### Install

1. Download the full MQB Toolbox ZIP from this fork's [Releases](https://github.com/mathsls/MHI2-Carplay-Maps/releases) and extract it to the root of a FAT32 SD card.
2. Install or update the MQB Toolbox from this card through the unit's software update (hold MENU, then Update → SD card). MU1161 needs this step: the Toolbox scripts on the unit are what detect the firmware. On MU1102 or MU1367 with omonob's v1.2.x Toolbox already installed you can skip it.
3. In the Green Engineering Menu: **mqbcoding → Customization → CarPlay Maps VC → Install 791 AID 12.3 (AUTO MU)** (or 790).
4. Wait for `CPMAP_INSTALL_OK`, then do a full reboot (hold the power button until the VW logo).
5. Connect the iPhone, start a route and check the cockpit.

If you already run omonob's version, just install this one over it. **Restore factory files** in the same menu removes everything.

### Credits

- [omonob](https://github.com/omonob): CarPlay Maps VC, the whole project.
- diasdio: MOST bridge queue fix (omonob/MHI2-Carplay-Maps issue 1); offsets as published by [ramasauskas](https://github.com/ramasauskas/MHI2-Carplay-Maps).
- [tomasz-twardzicki](https://github.com/tomasz-twardzicki/MHI2-Carplay-Maps): found the same cluster URL independently for MU1427.
- [jilleb/mib2-toolbox](https://github.com/jilleb/mib2-toolbox): MQB Toolbox and the navigation-active patch.
- [luka-dev/qnx65-armv7-toolchain](https://github.com/luka-dev/qnx65-armv7-toolchain): linker used for `libcardshim`.

## Português (fork)

### O que muda

| Mudança | Resultado | Telas |
| --- | --- | --- |
| Logs do CarPlay Maps fora do RAM disk | O painel não trava mais depois de uns 20 minutos de uso | Todas |
| URL do painel `maps:/car/instrumentcluster` em vez de `…/map` | Waze, Google Maps e Apple Maps desenham o cartão de manobra (seta, distância, rua) no mapa do painel | 791 |
| Nova área útil e tamanho físico | Cartão centralizado no topo, abaixo do relógio; seta de posição centralizada embaixo | 791 |
| `libcardshim` | Cartão no estilo translúcido "liquid glass"; o painel é anunciado ao iOS como painel de instrumentos | 791 |
| Correção da ponte MOST do diasdio, keyframe a cada 5 quadros, 40 fps | Sem lag nem pixelização | 791 |
| Firmware MU1161 (`lsd.jxe` com 55.830.309 bytes) | Instalação em centrais MU1161 | 791 |

Por que travava: o projeto grava três logs de diagnóstico em `/ramdisk`, um disco em RAM de 20 MB que também guarda os 16 MB do buffer de vídeo. Com keyframe a cada 5 quadros, o log de vídeo cresce cerca de 4 KB/s e o da ponte MOST cerca de 1 KB/s, e o disco chegava a 98 % em uns 20 minutos. Detalhes e offsets: [`src/fork/README.md`](./src/fork/README.md).

### Compatibilidade

| Firmware | Tamanho do `lsd.jxe` | 791 AID 12.3 | 790 AID 10.5 |
| --- | ---: | :---: | :---: |
| MU1102 | 55.915.411 bytes | ✅ todas as mudanças | ✅ só a correção dos logs |
| MU1161 | 55.830.309 bytes | ✅ todas as mudanças, testado no carro | ❌ |
| MU1367 | 55.840.541 bytes | ✅ todas as mudanças | ✅ só a correção dos logs |
| MU1440 Skoda | 55.840.933 bytes | ❌ | ✅ só a correção dos logs |

Testei só na MU1161/791: VW Passat B8 2018, Discover Pro, iPhone no cabo, Waze e Google Maps. MU1102 e MU1367 usam os mesmos arquivos da 791, mas não testei pessoalmente. Relatos são bem-vindos nas issues.

### Limitações conhecidas

- O Waze não mostra a barra de horário de chegada no painel; o iOS esconde.
- O tamanho do cartão é definido pelo iOS; diminuir a área útil não diminui o cartão.
- O Waze trava o zoom do mapa do painel (o zoom pelo volante funciona no Google Maps e no Apple Maps).
- A manobra não aparece nos gráficos próprios do painel VW; o cartão faz parte do vídeo do CarPlay.
- Anunciar 60 fps não ajudou no meu teste (pequeno lag nas curvas). O tomasz-twardzicki mediu 25 a 27 quadros por segundo vindos do iPhone com 40 anunciados.

### Instalação

1. Baixe o ZIP completo da MQB Toolbox nas [Releases](https://github.com/mathsls/MHI2-Carplay-Maps/releases) deste fork e extraia na raiz de um cartão SD em FAT32.
2. Instale ou atualize a MQB Toolbox a partir desse cartão pela atualização de software da central (segure MENU, depois Atualizar → Cartão SD). Na MU1161 esse passo é obrigatório, porque são os scripts da toolbox na central que identificam o firmware. Na MU1102 ou MU1367 com a toolbox v1.2.x do omonob já instalada, dá para pular.
3. No Green Engineering Menu: **mqbcoding → Customization → CarPlay Maps VC → Install 791 AID 12.3 (AUTO MU)** (ou 790).
4. Espere aparecer `CPMAP_INSTALL_OK` e reinicie a central segurando o botão de ligar até aparecer o logo da VW.
5. Conecte o iPhone, inicie uma rota e confira o painel.

Se você já usa a versão do omonob, é só instalar esta por cima. **Restore factory files**, no mesmo menu, remove tudo.

### Créditos

- [omonob](https://github.com/omonob): CarPlay Maps VC, o projeto inteiro.
- diasdio: correção da fila da ponte MOST (issue 1 do omonob/MHI2-Carplay-Maps); offsets publicados por [ramasauskas](https://github.com/ramasauskas/MHI2-Carplay-Maps).
- [tomasz-twardzicki](https://github.com/tomasz-twardzicki/MHI2-Carplay-Maps): achou a mesma URL do painel de forma independente na MU1427.
- [jilleb/mib2-toolbox](https://github.com/jilleb/mib2-toolbox): MQB Toolbox e o patch de navegação ativa.
- [luka-dev/qnx65-armv7-toolchain](https://github.com/luka-dev/qnx65-armv7-toolchain): linker usado no `libcardshim`.

---

<div align="center">

## Upstream README (omonob)

The original README follows, including the author's notices; only its Chinese section was removed. Its download links point to omonob's releases.

# CarPlay Maps VC for MHI2

**Bring the factory CarPlay map video to Volkswagen and Skoda virtual cockpits.**

[![Release](https://img.shields.io/github/v/release/omonob/MHI2-Carplay-Maps?display_name=tag&style=flat-square)](https://github.com/omonob/MHI2-Carplay-Maps/releases/latest)
[![License](https://img.shields.io/badge/license-GPL--3.0-blue?style=flat-square)](LICENSE)
![Platform](https://img.shields.io/badge/platform-MHI2%20%2F%20QNX%206.5-2f6f9f?style=flat-square)
![Profiles](https://img.shields.io/badge/profiles-MU1102%20%7C%20MU1367%20%7C%20MU1440-green?style=flat-square)

[English](#english) · [Downloads](https://github.com/omonob/MHI2-Carplay-Maps/releases/latest)

**Author: [omonob (汽车大玩家)](https://github.com/omonob)**

</div>

---

## English

### v1.2.2 automatic navigation prerequisite repair

**Replace the complete old SD-card files with v1.2.2. Repeating an old installer does not apply this fix.**

- Removes the target-tool dependencies behind repeated false navigation-payload failures. A bundled read-only ZIP checker replaces `cksum`, `tr` and binary text searches.
- After exact firmware identification, continue with a complete existing patch, automatically complete a missing/partial prerequisite, or repair our damaged standalone navigation JAR. Other projects' JAR files are not rewritten.
- Check the installed patch before requiring an SD navigation JAR. Merge old classpath entries and support repeat installation; real missing/damaged files, unsupported firmware and write failures still stop explicitly.
- **MQB Toolbox interactive installation only.** M.I.B. automatic-flashing packages and their build entry points are withdrawn; do not use SWDL to install this project.
- The 791 40 FPS optimization, 790 parameters, Skoda roller and Volkswagen button controls are unchanged.

[Download the v1.2.2 full MQB toolbox and checksums](https://github.com/omonob/MHI2-Carplay-Maps/releases/tag/toolbox-plaintext-v1.2.2) · [Release notes](./releases/toolbox-plaintext-v1.2.2.md)

### v1.2.0 historical update

This release adds **791 40 FPS display optimization** while retaining the navigation-centered layout. Only MU1102/791, MU1367/791 and their cores in the universal MQB toolbox change. The 790 profiles, MU1440 Skoda roller control, navigation prerequisite and entry points remain unchanged.

40 fps is a negotiated ceiling; actual frame delivery and display consumption depend on the phone, content and cluster. The earlier 791 native core already advertised 40 fps; stale top-level documentation is corrected.

[Historical release notes](./releases/toolbox-plaintext-v1.2.0.md); use v1.2.2 above for installation.

### v1.1.0 historical update

Only the following two changes are included. Display parameters, compatibility, installation entry points and paid-support information remain unchanged.

1. **Automatic navigation prerequisite patch**: the installer checks the boot configuration and navigation patch classes. If missing, it installs the patch that prevents CarPlay navigation from reporting “activated on the mobile device” and closing the cluster map. An existing complete patch is retained, including a merged metadata JAR. Unpatched factory `lsd.sh` files are now supported. This enables map coexistence; it does not add navigation-text or music metadata mapping.
2. **MU1440 Skoda roller zoom**: roll up to zoom in and down to zoom out; pressing the roller is not mapped. Only the MU1440 control JAR changes; Volkswagen button behavior stays the same. The phone map app must support zoom commands: Amap was verified, while Baidu Maps does not support this operation.

[Historical release notes](./releases/toolbox-plaintext-v1.1.0.md); use v1.2.2 above for installation.

### Overview

CarPlay Maps VC is a free and open-source community project for Volkswagen and Skoda MHI2 systems. It sends the CarPlay map video to the factory virtual cockpit and adds steering-wheel map controls.

The public release uses the validated V70 display/control baseline, adds the 791 40 FPS display optimization, and provides **one complete MQB toolbox installation route**:

- **MQB Toolbox interactive package**: detects the MU firmware automatically; the user selects the cluster type.

### Included

- CarPlay map video on the factory virtual cockpit.
- Short steering-wheel press for map zoom in/out.
- Long steering-wheel press for map-view switching.
- Exact MU1102, MU1367 and MU1440 firmware gates.
- Factory-file rollback evidence captured during installation.
- Status, repeat-install and factory-restore paths.
- Plaintext runtime under <code>/mnt/app/carplaymaps</code>.

### Not included

- Music metadata or cover art.
- Navigation text, road names, maneuver or lane guidance.
- Startup animation.


### Display profiles

| Cluster | Fixed display behavior | Steering-wheel control |
| --- | --- | --- |
| 791/794 AID 12.3 | 40 fps declaration, navigation centered | Short press zoom; long press map switch |
| 790 AID 10.5 | Unchanged v1.1.0 profile, non-navigation centered, navigation right | Short press zoom; long press map switch |

### Compatibility

| Firmware | Exact lsd.jxe size | 791/794 AID 12.3 | 790 AID 10.5 |
| --- | ---: | :---: | :---: |
| MU1102 | 55,915,411 bytes | ✅ | ✅ |
| MU1367 | 55,840,541 bytes | ✅ | ✅ |
| MU1440 Skoda | 55,840,933 bytes | ❌ | ✅ |

Unknown file sizes and unsupported cluster combinations stop before factory files are changed.

### Downloads

Download complete, ready-to-extract toolbox distributions from [GitHub Releases](https://github.com/omonob/MHI2-Carplay-Maps/releases/latest). Extract each archive directly to the SD-card root.

For manual integration into an existing toolbox, use the standalone project files stored directly in this repository. They are separated by firmware and cluster and **do not include a complete toolbox**:

| Firmware / cluster | Standalone files |
| --- | --- |
| MU1102 / 791/794 AID 12.3 | [`MU1102/791_AID12.3`](./MU1102/791_AID12.3/) |
| MU1102 / 790 AID 10.5 | [`MU1102/790_AID10.5`](./MU1102/790_AID10.5/) |
| MU1367 / 791/794 AID 12.3 | [`MU1367/791_AID12.3`](./MU1367/791_AID12.3/) |
| MU1367 / 790 AID 10.5 | [`MU1367/790_AID10.5`](./MU1367/790_AID10.5/) |
| MU1440 Skoda / 790 AID 10.5 | [`MU1440_SKODA/790_AID10.5`](./MU1440_SKODA/790_AID10.5/) |

Each directory contains only the matching installer, shared runtime files, profile-specific `core.so`, status script, and factory-restore script. Most users should download the full toolbox archives from Releases.

- [Full MQB Toolbox V4.2A v1.2.2](https://github.com/omonob/MHI2-Carplay-Maps/releases/download/toolbox-plaintext-v1.2.2/VW_SKODA_ALL_CarPlay_Maps_VC_FULL_MQB_Toolbox_V4.2A_v1.2.2.zip)
- [SHA-256 manifest](https://github.com/omonob/MHI2-Carplay-Maps/releases/download/toolbox-plaintext-v1.2.2/FULL_TOOLBOX_SHA256.txt)

### Optional paid support

The open-source CarPlay Maps VC map-video and steering-control portion in this repository remains **free of charge**. The author does not sell the open-source package as a paid product, and downloading, using, or exercising the rights granted by GPLv3 does not require a purchase.

Users who want additional functions or wish to support continued development may choose a separate paid service: **¥399 RMB (approximately US$60)**. The USD figure is an estimate based on the [ECB reference rates for 30 September 2026](https://www.ecb.europa.eu/stats/policy_and_exchange_rates/euro_reference_exchange_rates/html/index.en.html); the actual converted amount may vary.

<p align="center">
  <a href="https://www.goofish.com/item?id=1076254792602"><img src="./assets/optional-paid-support-goofish.png" alt="CarPlay Maps VC optional paid support" width="620"></a>
</p>

[**View the optional paid service on Goofish**](https://www.goofish.com/item?id=1076254792602)

The paid option includes:

1. Exclusive model-specific startup logo customization for all Volkswagen and Skoda vehicle models.
2. CarPlay navigation information mapped to the instrument cluster.
3. CarPlay music information mapped to the instrument cluster.
4. Smoothness optimization for 791/794 clusters.
5. Navigation-centering optimization for the 790 cluster.
6. Ongoing research and possible future support for factory wireless CarPlay. This feature is still under evaluation, is not currently guaranteed to be available, and has no fixed delivery date.

> [!NOTE]
> This is a separate feature and technical-support service, not a license fee for the open-source software. Confirm the exact firmware, cluster and supported scope on the product page before purchasing.

### Installation using MQB Toolbox

1. Download the full MQB Toolbox archive.
2. Extract its contents directly to the SD-card root.
3. Insert the card and open MQB Toolbox.
4. Go to <code>Customization → CarPlay Maps VC</code>.
5. Select the 791 or 790 installer.
6. The toolbox detects the MU and stops on an incompatible selection.
7. After <code>CPMAP_INSTALL_OK</code>, exit the toolbox and perform one full unit reboot.
8. Connect the iPhone and verify map video and steering-wheel controls.

The MQB menu also includes status and factory-restore actions. A full reboot is required after restoration.

### Development stack

- QNX Neutrino 6.5 / ARMv7 runtime target.
- KornShell and POSIX shell for install, status, restore and automation.
- Java/JAR integration for steering-wheel map controls.
- MQB Toolbox GEM/ESD menu integration.
- PowerShell for complete-toolbox assembly, archives, SHA-256 manifests and QA.
- Git and GitHub Releases for versioning and distribution.

### Build and verification status

The v1.2.2 MQB archive is tested for all five firmware/cluster profiles: first/repeat installation, complete/merged/partial/missing/damaged patches, factory restore and wrong-profile rejection. Host fixtures model unavailable target tools and binary-search restrictions; they do not execute QNX on a head unit. Shell syntax, JAR integrity, rollback, native checker bounds, QNX ARM ABI, ASCII/LF requirements, full ZIP reads and SHA-256 are checked. Relative to v1.2.1, only the installer changes and a read-only checker is added; all existing video/runtime binaries remain byte-identical.

Each firmware/cluster combination still requires independent live-vehicle validation. Compressed 790 and 791 demonstration videos will be added in a later Release update.

### Open-source notice

- Maintainer and author: **omonob (QCDWJ)**.
- Original integration scripts, build logic and documentation in this repository are licensed under [GNU GPL v3](LICENSE).
- GPL-covered redistribution must preserve attribution and license terms and provide corresponding source as required by GPLv3.
- MQB Toolbox, M.I.B. 3.6.0, Volkswagen, Skoda, Apple CarPlay and other third-party components, names and trademarks remain the property of their respective owners.
- Bundling a complete toolbox is for a ready-to-use SD-card layout; upstream files and license notices must remain intact.
- GPLv3 does not relicense third-party firmware, toolbox or vendor components.
- No unit-specific passwords, private keys, K1/K2, <code>shadow</code>, NAVDB ciphertext or vehicle-bound data are published.

### Safety and disclaimer

Writing infotainment files carries a boot-failure risk. Confirm the exact firmware and cluster, maintain stable power and keep a tested recovery route. Do not install or debug while driving.

This independent community project is not affiliated with or endorsed by Volkswagen AG, Skoda Auto or Apple Inc. It is provided as-is, without warranty.

---

<div align="center">

Free and open-source CarPlay Maps VC project by **omonob (QCDWJ)**.

</div>
