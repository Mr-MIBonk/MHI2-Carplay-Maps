"""Native checker host tests; these are not QNX execution tests."""
from pathlib import Path
import random
import struct
import subprocess
import tempfile
import zipfile

from test_toolbox_install import REPO, host_checker, package

CLASSES = [
    "de/esolutions/fw/comm/dsi/carplay/impl/DSICarplayProxy.class",
    "de/esolutions/fw/dsi/androidauto2/DSIAndroidAuto2Dispatcher.class",
    "de/esolutions/fw/dsi/androidauto/DSIAndroidAutoDispatcher.class",
    "de/vw/mib/asl/internal/androidauto/target/NavigationListener.class",
    "de/vw/mib/asl/internal/androidauto/target/RequestHandler.class",
    "de/vw/mib/asl/internal/carplay/common/CarPlayModeHandling.class",
    "de/vw/mib/asl/internal/exboxm/api/impl/guidance/ExboxGuidanceManagerImpl.class",
]


def check(binary, *args, ok=True):
    result = subprocess.run([binary, *map(str, args)], capture_output=True)
    assert result.returncode in (0, 1, 2), result.stderr
    assert (result.returncode == 0) == ok, (args, result.returncode, result.stderr)
    assert b"runtime error" not in result.stderr


def main():
    with tempfile.TemporaryDirectory(prefix="cpmap-nav-unit-") as tmp:
        root = Path(tmp)
        sanitized = root / "checker-ubsan"
        subprocess.run(["/usr/bin/gcc", "-std=c99", "-O1", "-g", "-Wall", "-Wextra", "-Werror",
                        "-fsanitize=undefined,bounds", "-fno-sanitize-recover=all",
                        str(REPO / "src/toolbox/cpmap-nav-check.c"), "-o", str(sanitized)], check=True)
        nav = package("MU1102", "791") / "payload/common/CarPlayNavigationPatch.jar"
        data = nav.read_bytes()
        for binary in (host_checker(), str(sanitized)):
            check(binary, "--self-test")
            check(binary, "--payload", nav)
            check(binary, "--has-patch", nav)
            check(binary, "--same", nav, nav)
            check(binary, "--payload", root / "absent", ok=False)
            check(binary, "--same", nav, root / "absent", ok=False)
            bad = root / "bad.jar"
            for n in (0, 1, 21, 22, 30, 512, len(data) - 1):
                bad.write_bytes(data[:n])
                check(binary, "--payload", bad, ok=False)
                check(binary, "--has-patch", bad, ok=False)
            bad.write_bytes(b"\n".join(n.encode() for n in CLASSES))
            check(binary, "--has-patch", bad, ok=False)  # Names without a ZIP are not a patch.
            changed = bytearray(data)
            changed[256] ^= 1
            bad.write_bytes(changed)
            check(binary, "--payload", bad, ok=False)
            check(binary, "--same", nav, bad, ok=False)
            with zipfile.ZipFile(bad, "w") as archive:
                for name in CLASSES[:-1]:
                    archive.writestr(name, b"class-fixture")
            check(binary, "--has-patch", bad, ok=False)
            # Complete third-party layout, including a ZIP comment, is accepted.
            with zipfile.ZipFile(bad, "w", zipfile.ZIP_DEFLATED) as archive:
                for name in CLASSES:
                    archive.writestr(name, b"class-fixture")
                archive.comment = b"third-party patch"
            check(binary, "--has-patch", bad)
            check(binary, "--payload", bad, ok=False)
            with zipfile.ZipFile(bad, "a") as archive:
                archive.writestr(CLASSES[0], b"duplicate-class")
            check(binary, "--has-patch", bad, ok=False)
            # Mutate central-directory offsets, lengths, and local records.
            central = data.index(b"PK\x01\x02")
            end = data.rindex(b"PK\x05\x06")
            for offset, fmt, value in ((central + 42, "<I", 0xffffffff),
                                       (central + 28, "<H", 65535),
                                       (central + 20, "<I", 0xffffffff),
                                       (end + 16, "<I", 0xffffffff),
                                       (end + 12, "<I", 0xffffffff),
                                       (end + 10, "<H", 65535)):
                changed = bytearray(data)
                struct.pack_into(fmt, changed, offset, value)
                bad.write_bytes(changed)
                check(binary, "--has-patch", bad, ok=False)
            rng = random.Random(122)
            for _ in range(150):
                bad.write_bytes(bytes(rng.getrandbits(8) for _ in range(rng.randrange(22, 2048))))
                check(binary, "--has-patch", bad, ok=False)
        print("NAV_CHECKER_HOST_AND_UBSAN_OK")


if __name__ == "__main__":
    main()
