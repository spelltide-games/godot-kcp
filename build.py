#!/usr/bin/env python3
import argparse
import glob
import os
import shutil
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent
BIN_DIR = ROOT / "bin"
DIST_BIN = ROOT / "dist" / "addons" / "godot-kcp" / "bin"
GDEXT_FILE = ROOT / "godot-kcp.gdextension"


def run(cmd, env=None):
    print(f"+ {' '.join(cmd)}")
    subprocess.run(cmd, check=True, env=env)


def copy_if_exists(src: Path, dst_dir: Path):
    if not src.exists():
        return False
    dst_dir.mkdir(parents=True, exist_ok=True)
    if src.is_dir():
        target = dst_dir / src.name
        if target.exists():
            shutil.rmtree(target)
        shutil.copytree(src, target)
    else:
        shutil.copy2(src, dst_dir / src.name)
    return True


def collect_artifacts(platform: str):
    patterns = {
        "linux": ["*.so"],
        "windows": ["*.dll"],
        "macos": ["*.framework", "*.dylib"],
        "android": ["*.so"],
        "ios": ["*.a"],
        "web": ["*.wasm"],
    }.get(platform, ["*"])

    found = False
    for pat in patterns:
        for path in BIN_DIR.glob(pat):
            if path.is_dir():
                # framework directory
                copy_if_exists(path, DIST_BIN)
                found = True
            else:
                shutil.copy2(path, DIST_BIN / path.name)
                found = True
    if not found:
        print(f"Warning: no artifacts matched for platform={platform}", file=sys.stderr)


def main():
    parser = argparse.ArgumentParser(description="Build godot-kcp for a platform and collect artifacts.")
    parser.add_argument("--platform", required=True)
    parser.add_argument("--target", default="template_release")
    parser.add_argument("--arch", default=None)
    parser.add_argument("--emsdk", default=None, help="EMSDK path for web")
    parser.add_argument("--extra", nargs="*", default=[], help="Extra key=value pairs for scons")
    args = parser.parse_args()

    env = os.environ.copy()
    if args.emsdk:
        env["EMSDK"] = args.emsdk

    scons_cmd = ["scons", f"platform={args.platform}", f"target={args.target}"]
    if args.arch:
        scons_cmd.append(f"arch={args.arch}")
    scons_cmd.extend(args.extra)

    run(scons_cmd, env=env)

    # Prepare dist addon layout
    DIST_BIN.mkdir(parents=True, exist_ok=True)
    if GDEXT_FILE.exists():
        gdext_dest = DIST_BIN.parent / GDEXT_FILE.name
        if not gdext_dest.exists():
            shutil.copy2(GDEXT_FILE, gdext_dest)

    collect_artifacts(args.platform)


if __name__ == "__main__":
    main()
