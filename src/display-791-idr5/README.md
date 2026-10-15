# 791 40 fps and IDR5 parameter update

This update preserves the existing public 791 native core and changes only the periodic IDR request interval. It does not import an encrypted/private installer, another profile, a diagnostic queue experiment, metadata mapping or startup animation.

## Exact baseline

- Public commit: `222e76a47a13308042e097bb127c60a5effb6848`.
- Input files: `MU1102/791_AID12.3/payload/profiles/MU1102/791/core.so` and the equivalent MU1367/791 core. The two files are identical.
- Input size: 75,099 bytes.
- Input SHA-256: `00b48a8fe876e6ff405124b87ed9eace1bcb079220ff7f02b866f42676477a4a`.
- The existing native display declaration already uses **800x480 and maxFPS=40**. Earlier top-level documentation saying 30 fps was stale.

## Minimal reproducible transformation

`patch_core.py` checks the entire input SHA-256 and ELF header before writing anything. Two fixed-width ARM instructions in `passive_listener_thread` change from `mov r2, #20` to `mov r2, #5`:

| File offset | Purpose |
| --- | --- |
| `0x4370` | Divisor supplied to the 64-bit remainder check for periodic requests |
| `0x452c` | Matching interval value passed to the periodic-request log |

Only the immediate byte at each offset changes. File size, layout, symbols, dependencies, geometry, map controls, video decryption and shared-ring behavior remain unchanged. The `maxFPS=40` instruction at `0x2a70` is explicitly checked and not modified. This is a parameter transformation of an already-published executable, not a new compilation; embedded build metadata remains that of the baseline. SHA-256 identifies the new result.

The equivalent source-level behavior is:

```c
/* Inside the existing valid-frame/session path. Do not alter the H.264 frame. */
if (nal_count > 0 && listener->session && (frames % 5ULL) == 0) {
    send_alt_command(listener->session, "forceKeyFrame", NULL);
}
```

`forceKeyFrame` asks the iPhone to produce a genuine alternate-display keyframe. It does not relabel a predicted frame. The response is asynchronous; startup/recovery requests may also occur. IDR5 means a five-received-frame **request interval**, not guaranteed five-frame IDR output spacing. `maxFPS=40` is a declaration, not proof of a measured 40 displayed frames per second.

## Reproduce and verify

Obtain the exact baseline core from the commit above, then run:

```text
python src/display-791-idr5/patch_core.py baseline-core.so core-idr5.so
python tests/test_791_idr5.py
```

No QNX compiler, licence change, device access or installation is performed. Unknown binaries, 790 cores, damaged inputs and an already-updated core are rejected as transformation inputs. Tests additionally compare all unchanged profile files against the previous public commit.

The 40 fps/IDR5 setting was previously exercised in the author's test series. Packaging checks for this release are offline; they do not claim a new live test on every MU/phone combination. 790 and MU1440 Skoda remain unchanged.

中文：791 保持已有的 800×480、40 fps 声明和导航居中，只把周期请求的除数与对应日志参数从 20 改为 5；不导入私有加密版或低延迟探针。脚本先核对整个旧文件哈希，再做两处常量变更，其他字节不变。IDR5 是请求节奏，不是保证手机实际每五帧输出一个 IDR。
