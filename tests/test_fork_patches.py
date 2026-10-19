"""Offline checks that every fork payload is reproducible from upstream v1.2.2. No device connection."""
import hashlib
import importlib.util
from pathlib import Path
import subprocess

REPO = Path(__file__).resolve().parents[1]
UPSTREAM = "02d3f35872c44c7b3cbff7fcd76b19e13222ebc9"  # omonob v1.2.2
spec = importlib.util.spec_from_file_location("fork_patch", REPO / "src/fork/patch_payload.py")
patch = importlib.util.module_from_spec(spec)
spec.loader.exec_module(patch)

PACKAGES = ["MU1102/790_AID10.5", "MU1102/791_AID12.3", "MU1161/791_AID12.3", "MU1367/790_AID10.5",
            "MU1367/791_AID12.3", "MU1440_SKODA/790_AID10.5"]
SHIM_SHA256 = "1cb962c5"


def upstream(name):
    return subprocess.run(["git", "show", f"{UPSTREAM}:{name}"], cwd=REPO,
                          check=True, capture_output=True).stdout


def sha(data):
    return hashlib.sha256(data).hexdigest()


def main():
    wrapper = (REPO / "MU1367/791_AID12.3/payload/common/dio_manager.wrapper.sh").read_text()
    assert wrapper.count("=/dev/null") == 2 and "/ramdisk/carplaymaps-video.log" not in wrapper
    assert "/ramdisk/carplay-maps-session.log" not in wrapper and "libcardshim.so" in wrapper
    installer = (REPO / "src/toolbox/install-carplay-maps.sh").read_bytes()
    supervisor = bytearray(upstream("MU1367/791_AID12.3/payload/common/carplay-cluster-supervisor"))
    patch.apply_edits(supervisor, patch.SUPERVISOR["edits"], "supervisor")
    bridge = bytearray(upstream("MU1367/791_AID12.3/payload/common/altscreen-most-bridge"))
    patch.apply_edits(bridge, patch.BRIDGE["edits"], "bridge")
    assert sha(supervisor) == patch.SUPERVISOR["patched"] and sha(bridge) == patch.BRIDGE["patched"]

    common = None
    for pkg in PACKAGES:
        payload = REPO / pkg / "payload"
        files = {p.relative_to(payload).as_posix(): p.read_bytes()
                 for p in payload.rglob("*") if p.is_file()}
        # Upstream ships the MU1440 roller-control JAR in the standalone common/.
        mine = {k: v for k, v in files.items() if k.startswith("common/")
                and not (pkg.startswith("MU1440") and k == "common/CarPlayClusterControls.jar")}
        common = common or mine
        assert mine.keys() <= common.keys() and all(common[k] == v for k, v in mine.items()), \
            f"{pkg}: common/ differs between packages"
        assert files["common/dio_manager.wrapper.sh"].decode() == wrapper
        assert files["common/install-carplay-maps.sh"] == installer
        assert (REPO / pkg / "install.sh").read_bytes() == installer
        assert files["common/carplay-cluster-supervisor"] == supervisor
        base = pkg.replace("MU1161", "MU1367")
        for rel in ("common/altscreen-most-bridge", "common/CarPlayClusterControls.jar",
                    "common/CarPlayNavigationPatch.jar", "common/libcarplay-security-bridge.so"):
            assert files[rel] == upstream(f"{base}/payload/{rel}"), f"{pkg}/{rel}"

        profiles = sorted(k for k in files if k.startswith("profiles/"))
        mu = profiles[0].split("/")[1]
        cluster = pkg.split("/")[1][:3]
        core = files[f"profiles/{mu}/{cluster}/core.so"]
        if cluster == "791":
            assert sha(core) == patch.CORE_PATCHED[40]
            assert files[f"profiles/{mu}/791/altscreen-most-bridge"] == bridge
            assert sha(files[f"profiles/{mu}/791/libcardshim.so"]).startswith(SHIM_SHA256)
            assert files[f"profiles/{mu}/791/CLUSTER_ZOOM"] == b"1.5\n"
            assert len(profiles) == 4, profiles
        else:
            assert core == upstream(f"{base}/payload/profiles/{mu}/790/core.so")
            assert len(profiles) == 1, profiles
        print("PASS fork payload", pkg)
    print("FORK_PAYLOADS_REPRODUCIBLE_OK")


if __name__ == "__main__":
    main()
