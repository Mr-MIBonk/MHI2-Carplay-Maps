# Fork patches (mathsls)

Everything here is reproducible from the upstream omonob files. `patch_payload.py` checks the SHA-256 of every input and output, so it refuses unknown builds and never writes a partial result.

```text
python3 src/fork/patch_payload.py MU1367/791_AID12.3/payload [--fps 30|40]
python3 src/fork/patch_payload.py /Volumes/<SD>/Custom/carplay_maps_vc/payload
```

`common/` stays shared by every profile, as upstream. Files that only the 791 cluster uses live in `profiles/<MU>/791/` (`core.so`, `altscreen-most-bridge`, `libcardshim.so`, `CLUSTER_ZOOM`); the installer takes the bridge from the profile directory when it exists, otherwise from `common/`.

## 1. RAM disk logs (all clusters)

The CarPlay Maps VC processes write three diagnostic logs to `/ramdisk`, a 20 MB RAM disk that also holds the 16 MB video ring:

| Log | Written by | Path set in | Growth seen in the car |
| --- | --- | --- | --- |
| `carplaymaps-video.log` | `core.so` | `CARPLAY_ALTSCREEN_LOG` in `dio_manager.wrapper.sh` | ~4 KB/s with a keyframe every 5 frames |
| `carplay-maps-session.log` | `libcarplay-security-bridge.so` | `CARPLAY_SECURITY_BRIDGE_LOG` in `dio_manager.wrapper.sh` | small |
| `altscreen-most-bridge.log` | `altscreen-most-bridge` | string hard-coded in `carplay-cluster-supervisor` at `0x47c0` | ~1 KB/s |

With the v1.2.x keyframe interval the RAM disk reached 98 % in about 20 minutes of driving, after which the cluster stream froze. The bridge log is opened with `O_TRUNC` and without `O_APPEND`, so truncating it from outside does not free space.

The fix:

- `dio_manager.wrapper.sh`: both variables point to `/dev/null`.
- `carplay-cluster-supervisor`: the 35-byte string `/ramdisk/altscreen-most-bridge.log` at file offset `0x47c0` becomes `/dev/null` padded with NUL bytes. The path is only used in `open()` and in `copy_diagnostic_file()`, never in `unlink()`. SHA-256 `cdc2f9c6…` → `8e6ac3a8…`.
- `carplaymaps-launcher.log` is kept: a few lines per boot.

To get the logs back for debugging, restore the two variables in `/mnt/app/eso/bin/apps/dio_manager` (the installed copy of the wrapper) and the original supervisor.

## 2. MOST bridge (791)

diasdio's AUSAFE fix from omonob/MHI2-Carplay-Maps issue 1 (offsets as published in ramasauskas/MHI2-Carplay-Maps): the queue is flushed only when no access unit is part-written, and a fresh IDR is requested under congestion. SHA-256 `c846e505…` → `fed7dce4…`, written to `profiles/<MU>/791/altscreen-most-bridge`. Not tested on 790 clusters, so they keep the upstream bridge from `common/`.

## 3. Turn card on the cluster (791)

`core.so` (identical for MU1102, MU1161 and MU1367 on 791). Input: v1.0.0-v1.1.0 (`00b48a8f…`) or v1.2.0-v1.2.2 (`f3a6d257…`).

| Offset | Instruction | Upstream | Fork |
| --- | --- | --- | --- |
| `0x2a70` | maxFPS | 40 | 40 (`--fps 30` available) |
| `0x4370`, `0x452c` | forced keyframe interval | 20 (v1.2.x: 5) | 5 |
| `0x2ad0` | physical width (mm) | 160 | 240 |
| `0x2af0` | physical height (mm) | 96 | 144 |
| `0x2ba4` | safe area x | 160 | 240 |
| `0x2b94` | safe area y | 20 | 60 |
| `0x2b98` | safe area width | 480 | 320 |
| `0x2b9c` | safe area height | 440 | 340 |
| `0x8130` | cluster URL (`initialURL` and `showUI`) | `maps:/car/instrumentcluster/map` | `maps:/car/instrumentcluster` |

- With `…/map` iOS only draws the map (`mapOnly: 1`). The root URL makes it draw the turn card that Waze, Google Maps and Apple Maps render in the cluster video. tomasz-twardzicki found the same URL independently for MU1427.
- The card hangs from the top of the safe area; x 240 / width 320 centres it, y 60 keeps it below the clock bar and the car marker sits centred near the bottom.
- iOS chooses the card layout from the safe area and the physical size. This fork was tested only with 240×144 mm. tomasz-twardzicki got a card on MU1427 with the upstream 160×96 mm, so the larger size may not be required.
- iOS keeps the cluster at @2x (400×240 pt). Shrinking the safe area does not make the card smaller, and the ETA bar stays hidden for Waze.
- Values are ARM rotated immediates (`imm8 ror 2*rot`), e.g. 320 = `0x50` with rotation 15.

## 4. libcardshim (791)

`src/cardshim/cardshim.c`, shipped as `profiles/<MU>/791/libcardshim.so` and loaded by the wrapper in front of `core.so` when `/mnt/app/carplaymaps/libcardshim.so` exists. It intercepts `AirPlayCopyServerInfo` and adds `properties = ["Instrument Cluster"]` and `uid` to the cluster display (type 111). It also reads two optional files on every CarPlay connection:

- `CLUSTER_ZOOM`: written to `zoomFactor`. It does not change the scale; 1.5 gives the translucent "liquid glass" card, values below 1 the black card. The payload ships 1.5.
- `CLUSTER_URL`: overrides `initialURL` (for testing other URLs without rebuilding `core.so`).

The card itself comes from the cluster URL; the shim was added before the URL change and has not been tested on its own, so it may only be needed for the card style. Disable it with `touch /mnt/app/carplaymaps/NO_CARD_SHIM`. Log: `/dev/shmem/cardshim.log`. Build: `src/cardshim/build.sh`.
