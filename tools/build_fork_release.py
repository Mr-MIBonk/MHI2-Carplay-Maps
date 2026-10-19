#!/usr/bin/env python3
"""Build the fork's full MQB Toolbox ZIP from omonob's v1.2.2 release archive.

  python3 tools/build_fork_release.py VW_SKODA_ALL_CarPlay_Maps_VC_FULL_MQB_Toolbox_V4.2A_v1.2.2.zip

The upstream archive is checked by SHA-256. Its Custom/carplay_maps_vc/payload is
replaced by the merge of this repository's standalone payloads (common/ is identical
in all of them), MU1161 is added to the two Toolbox scripts that pick the profile,
and the result goes to release-assets/ with a SHA-256 manifest.
"""
import hashlib
import pathlib
import sys
import zipfile

REPO = pathlib.Path(__file__).resolve().parents[1]
UPSTREAM_SHA256 = "06955ade05e461b26ec2fdb78631378b690fefb64b1ffa672b802caf55061ab5"
NAME = "VW_SKODA_ALL_CarPlay_Maps_VC_FULL_MQB_Toolbox_V4.2A_v1.2.2-mathsls.1.zip"
PAYLOAD = "Custom/carplay_maps_vc/payload/"
PACKAGES = ["MU1102/790_AID10.5", "MU1102/791_AID12.3", "MU1161/791_AID12.3",
            "MU1367/790_AID10.5", "MU1367/791_AID12.3", "MU1440_SKODA/790_AID10.5"]
FIXED_TIME = (2026, 10, 19, 0, 0, 0)

TEXT_EDITS = {
    "Toolbox/scripts/cpmap_common.sh": [
        ('        55915411) CPMAP_MU="MU1102" ;;\n',
         '        55915411) CPMAP_MU="MU1102" ;;\n        55830309) CPMAP_MU="MU1161" ;;\n'),
    ],
    "Toolbox/scripts/cpmap_install_791.sh": [
        ("    MU1102|MU1367) ;;\n", "    MU1102|MU1161|MU1367) ;;\n"),
    ],
    "Toolbox/GEM/carplay-maps-vc.esd": [
        ('    label   "Author: omonob (QCDWJ)"\n    poll    0\n',
         '    label   "Author: omonob (QCDWJ)"\n    poll    0\n\n'
         'keyValue\n    value   String sys 0x00000000 0\n'
         '    label   "Fork by mathsls: RAM log fix, turn card, MU1161"\n    poll    0\n'),
    ],
    "CARPLAY_MAPS_VC_README.md": [
        ("- `791 AID 12.3`: MU1102 and MU1367 only.\n",
         "- `791 AID 12.3`: MU1102, MU1161 and MU1367.\n"),
        ("Free open-source project by **omonob (QCDWJ)**.",
         "Free open-source project by **omonob (QCDWJ)**.\n\n"
         "Fork by mathsls (https://github.com/mathsls/MHI2-Carplay-Maps): CarPlay Maps logs moved off the "
         "RAM disk (fixes the freeze after about 20 minutes), turn-by-turn card on the 791 cluster, "
         "MU1161 support."),
    ],
}


def fork_payload():
    files = {}
    for pkg in PACKAGES:
        root = REPO / pkg / "payload"
        for path in sorted(p for p in root.rglob("*") if p.is_file()):
            rel = path.relative_to(root).as_posix()
            if pkg.startswith("MU1440") and rel == "common/CarPlayClusterControls.jar":
                rel = "profiles/MU1440/790/CarPlayClusterControls.jar"
            data = path.read_bytes()
            if rel in files and files[rel] != data:
                sys.exit(f"payload conflict between packages: {rel}")
            files[rel] = data
    return files


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    source = pathlib.Path(sys.argv[1])
    if hashlib.sha256(source.read_bytes()).hexdigest() != UPSTREAM_SHA256:
        sys.exit(f"not the omonob v1.2.2 archive: {source}")

    payload = fork_payload()
    out_dir = REPO / "release-assets"
    out_dir.mkdir(exist_ok=True)
    target = out_dir / NAME
    pending = dict(TEXT_EDITS)
    with zipfile.ZipFile(source) as old, zipfile.ZipFile(target, "w", zipfile.ZIP_DEFLATED) as new:
        upstream_payload = {i.filename[len(PAYLOAD):] for i in old.infolist()
                            if i.filename.startswith(PAYLOAD) and not i.is_dir()}
        missing = upstream_payload - payload.keys()
        if missing:
            sys.exit(f"fork payload lacks upstream files: {sorted(missing)}")
        for info in old.infolist():
            if info.filename.startswith(PAYLOAD) or info.is_dir():
                continue
            data = old.read(info)
            for before, after in pending.pop(info.filename, []):
                text = data.decode("utf-8")
                if text.count(before) != 1:
                    sys.exit(f"unexpected content in {info.filename}")
                data = text.replace(before, after).encode("utf-8")
            entry = zipfile.ZipInfo(info.filename, FIXED_TIME)
            entry.external_attr = info.external_attr
            new.writestr(entry, data, zipfile.ZIP_DEFLATED)
        for rel in sorted(payload):
            entry = zipfile.ZipInfo(PAYLOAD + rel, FIXED_TIME)
            entry.external_attr = 0o100755 << 16
            new.writestr(entry, payload[rel], zipfile.ZIP_DEFLATED)
    if pending:
        sys.exit(f"files to edit not found in the archive: {sorted(pending)}")

    digest = hashlib.sha256(target.read_bytes()).hexdigest()
    (out_dir / "FULL_TOOLBOX_SHA256.txt").write_text(f"{digest}  {NAME}\n")
    print(digest, target)


if __name__ == "__main__":
    main()
