#!/usr/bin/env python3
"""Apply the mathsls fork binary patches to a CarPlay Maps VC payload directory.

The payload directory is the one that contains common/ and profiles/, for example
MU1367/791_AID12.3/payload in this repository or
/Volumes/<SD>/Custom/carplay_maps_vc/payload on an MQB Toolbox SD card.

  common/carplay-cluster-supervisor   MOST bridge log path /ramdisk/... -> /dev/null
  profiles/<MU>/791/core.so           turn card on the cluster (see README.md)
  profiles/<MU>/791/altscreen-most-bridge
                                      written from common/altscreen-most-bridge with
                                      diasdio's AUSAFE queue/IDR fix; common/ stays stock
                                      because the 790 profiles share it

Every input is checked against known SHA-256 values before anything is written.
Unknown builds are refused and nothing is changed. dio_manager.wrapper.sh is a
text file and libcardshim.so / CLUSTER_ZOOM are shipped as files in this repository.
"""
import argparse
import hashlib
import pathlib
import sys

LOG_PATH = b"/ramdisk/altscreen-most-bridge.log\0"

SUPERVISOR = {
    "stock": "cdc2f9c64eae15ecfc57eafcd497a9a19582d04c7068b0a48e0b84c5d6546b24",
    "patched": "8e6ac3a85f04c3faca4850c9f09021bdd628fa1653d718f9c3db3be9d8057569",
    "edits": [(0x47C0, LOG_PATH.hex(), b"/dev/null".ljust(len(LOG_PATH), b"\0").hex())],
}

# By diasdio (omonob/MHI2-Carplay-Maps issue 1), offsets as published in
# ramasauskas/MHI2-Carplay-Maps apply.py.
BRIDGE = {
    "stock": "c846e505da704f101815a4658f1b815930d0b6f779d435349b864ef96056c8f3",
    "patched": "fed7dce45f923384fec47861c448b753913f0561ed7e2a279860a5d13f8a44f4",
    "edits": [
        (0x1B04, "0030a0e3", "1c6094e5"),  # flush the queue only when no access unit is part-written
        (0x1B08, "1c3084e5", "000056e3"),
        (0x1B0C, "0c2084e5", "0c208405"),
        (0x1B10, "143084e5", "14608405"),
        (0x4174, "07005383", "00005383"),  # request a fresh IDR under congestion
    ],
}

CORE_STOCK = "00b48a8fe876e6ff405124b87ed9eace1bcb079220ff7f02b866f42676477a4a"  # v1.0.0-v1.1.0
CORE_IDR5 = "f3a6d2575772307f8007c70057836df449af1509ba06438333baccb612d55826"   # v1.2.0-v1.2.2
CORE_PATCHED = {
    30: "7da43902b3a1662258533d815fbfb29c7635da36dca370b6bc480cd2bfe043df",
    40: "c9b7ae01d196d7c540feb05cc170085756ceca551a58a62be585561ebdfc7d12",
}
CORE_IDR_EDITS = [
    (0x4370, "1420a0e3", "0520a0e3"),  # forced keyframe every 20 -> 5 frames
    (0x452C, "1420a0e3", "0520a0e3"),  # same value, log line
]
CORE_DISPLAY_EDITS = [
    (0x2AD0, "a020a0e3", "f020a0e3"),  # physical width 160 -> 240 mm
    (0x2AF0, "6020a0e3", "9020a0e3"),  # physical height 96 -> 144 mm
    (0x2BA4, "a000a0e3", "f000a0e3"),  # safe area x 160 -> 240
    (0x2B94, "1410a0e3", "3c10a0e3"),  # safe area y 20 -> 60
    (0x2B98, "1e2ea0e3", "502fa0e3"),  # safe area width 480 -> 320
    (0x2B9C, "6e3fa0e3", "553fa0e3"),  # safe area height 440 -> 340
    (0x814B, "2f6d6170", "00000000"),  # maps:/car/instrumentcluster/map -> maps:/car/instrumentcluster
]
CORE_FPS_OFFSET = 0x2A70  # mov r2, #maxFPS
CORE_FPS_STOCK = "2820a0e3"


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def apply_edits(data, edits, name):
    for offset, old, new in edits:
        old, new = bytes.fromhex(old), bytes.fromhex(new)
        if data[offset:offset + len(old)] != old:
            sys.exit(f"unexpected bytes at {name}+{offset:#x}, nothing changed")
        data[offset:offset + len(new)] = new


def patch_simple(path, spec, target=None):
    data = bytearray(path.read_bytes())
    digest = sha256(data)
    if target is not None and target.is_file() and sha256(target.read_bytes()) == spec["patched"]:
        return None
    if target is None and digest == spec["patched"]:
        return None
    if digest != spec["stock"]:
        sys.exit(f"unknown build, nothing changed: {path} ({digest})")
    apply_edits(data, spec["edits"], path.name)
    if sha256(data) != spec["patched"]:
        sys.exit(f"result hash mismatch, nothing changed: {path}")
    return data


def patch_core(path, fps):
    data = bytearray(path.read_bytes())
    digest = sha256(data)
    if digest == CORE_PATCHED[fps]:
        return None
    if digest == CORE_STOCK:
        apply_edits(data, CORE_IDR_EDITS, path.name)
    elif digest != CORE_IDR5:
        sys.exit(f"unknown build, nothing changed: {path} ({digest})")
    apply_edits(data, CORE_DISPLAY_EDITS, path.name)
    apply_edits(data, [(CORE_FPS_OFFSET, CORE_FPS_STOCK, "%02x20a0e3" % fps)], path.name)
    if sha256(data) != CORE_PATCHED[fps]:
        sys.exit(f"result hash mismatch, nothing changed: {path}")
    return data


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("payload", type=pathlib.Path, help="payload directory containing common/ and profiles/")
    parser.add_argument("--fps", type=int, choices=sorted(CORE_PATCHED), default=40,
                        help="maxFPS declared for the 791 cluster stream (default 40)")
    args = parser.parse_args()

    common = args.payload / "common"
    supervisor = common / "carplay-cluster-supervisor"
    bridge = common / "altscreen-most-bridge"
    jobs =[(supervisor, supervisor, lambda p: patch_simple(p, SUPERVISOR))]
    for core in sorted(args.payload.glob("profiles/*/791/core.so")):
        target = core.parent / "altscreen-most-bridge"
        jobs.append((bridge, target, lambda p, t=target: patch_simple(p, BRIDGE, t)))
        jobs.append((core, core, lambda p: patch_core(p, args.fps)))

    pending = []
    for source, target, patch in jobs:
        if not source.is_file():
            sys.exit(f"missing: {source}")
        data = patch(source)
        if data is None:
            print(f"already patched: {target}")
        else:
            pending.append((target, data))

    for target, data in pending:
        target.write_bytes(data)
        print(f"patched: {target}")


if __name__ == "__main__":
    main()
