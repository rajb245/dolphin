#!/usr/bin/env python3
"""Helper to run golden DolphinTool CLI scenarios.

Each test extracts a small RVZ archive, invokes dolphin-tool, and compares the
command's stdout to a checked-in golden file. The RVZ archive defaults to the
repo owner's Balloon Pop sample but can be overridden with
DOLPHIN_TOOL_TEST_ARCHIVE.
"""

from __future__ import annotations

import argparse
import difflib
import shutil
import subprocess
import sys
import textwrap
import zipfile
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run a DolphinTool golden test")
    parser.add_argument("command", choices=["header", "verify", "convert-roundtrip"],
                        help="Scenario to execute")
    parser.add_argument("--dolphin-tool", required=True, dest="dolphin_tool",
                        help="Path to the dolphin-tool binary")
    parser.add_argument("--archive", required=True,
                        help="ZIP archive containing a single RVZ sample")
    parser.add_argument("--work-dir", required=True,
                        help="Directory used for extracted assets and outputs")
    parser.add_argument("--user-dir", required=True,
                        help="Per-test Dolphin user directory")
    parser.add_argument("--expected", required=True,
                        help="File containing the expected stdout for the scenario")
    return parser.parse_args()


def ensure_clean_dir(path: Path) -> None:
    if path.exists():
        shutil.rmtree(path)
    path.mkdir(parents=True)


def ensure_dir(path: Path) -> None:
    path.mkdir(parents=True, exist_ok=True)


def extract_rvz(archive: Path, dest_dir: Path) -> Path:
    ensure_clean_dir(dest_dir)
    with zipfile.ZipFile(archive) as zf:
        zf.extractall(dest_dir)
    rvz_files = list(dest_dir.glob("*.rvz"))
    if not rvz_files:
        raise RuntimeError(f"Archive {archive} did not contain an RVZ file")
    if len(rvz_files) > 1:
        raise RuntimeError(f"Archive {archive} contained multiple RVZ files")
    return rvz_files[0]


def run_tool(dolphin_tool: Path, *args: str) -> subprocess.CompletedProcess[str]:
    cmd = [str(dolphin_tool), *args]
    return subprocess.run(cmd, check=True, text=True, capture_output=True)


def compare_output(actual: str, expected_path: Path) -> None:
    expected = expected_path.read_text()
    if actual == expected:
        return

    diff = "\n".join(
        difflib.unified_diff(
            expected.splitlines(),
            actual.splitlines(),
            fromfile=str(expected_path),
            tofile="actual",
            lineterm="",
        )
    )
    message = textwrap.dedent(
        f"""
        Output did not match golden file {expected_path}:
        {diff}
        """
    ).strip()
    raise AssertionError(message)


def run_header(args: argparse.Namespace) -> None:
    work_dir = Path(args.work_dir) / "header"
    rvz_path = extract_rvz(Path(args.archive), work_dir)
    result = run_tool(Path(args.dolphin_tool), "header", "--input", str(rvz_path))
    compare_output(result.stdout, Path(args.expected))


def run_verify(args: argparse.Namespace, rvz_override: Path | None = None) -> None:
    rvz_path: Path
    if rvz_override is not None:
        rvz_path = rvz_override
    else:
        work_dir = Path(args.work_dir) / "verify"
        rvz_path = extract_rvz(Path(args.archive), work_dir)
    user_dir = Path(args.user_dir) / args.command
    ensure_clean_dir(user_dir)
    result = run_tool(
        Path(args.dolphin_tool),
        "verify",
        "--user",
        str(user_dir),
        "--input",
        str(rvz_path),
    )
    compare_output(result.stdout, Path(args.expected))


def run_convert_roundtrip(args: argparse.Namespace) -> None:
    command_name = "convert-roundtrip"
    work_root = Path(args.work_dir) / command_name
    extract_dir = work_root / "source"
    rvz_source = extract_rvz(Path(args.archive), extract_dir)

    user_dir = Path(args.user_dir) / command_name
    ensure_clean_dir(user_dir)

    iso_path = work_root / "roundtrip.iso"
    roundtrip_rvz = work_root / "roundtrip.rvz"

    try:
        run_tool(
            Path(args.dolphin_tool),
            "convert",
            "--user",
            str(user_dir),
            "--input",
            str(rvz_source),
            "--output",
            str(iso_path),
            "--format",
            "iso",
        )

        run_tool(
            Path(args.dolphin_tool),
            "convert",
            "--user",
            str(user_dir),
            "--input",
            str(iso_path),
            "--output",
            str(roundtrip_rvz),
            "--format",
            "rvz",
            "--block_size",
            "131072",
            "--compression",
            "zstd",
            "--compression_level",
            "19",
        )

        run_verify(args, roundtrip_rvz)
    finally:
        # Clean up large intermediate files to keep the workspace small.
        if iso_path.exists():
            iso_path.unlink()
        if roundtrip_rvz.exists():
            roundtrip_rvz.unlink()


def main() -> int:
    try:
        args = parse_args()
        ensure_dir(Path(args.work_dir))
        ensure_dir(Path(args.user_dir))

        if args.command == "header":
            run_header(args)
        elif args.command == "verify":
            run_verify(args)
        elif args.command == "convert-roundtrip":
            run_convert_roundtrip(args)
        else:
            raise ValueError(f"Unhandled command {args.command}")
    except subprocess.CalledProcessError as exc:
        sys.stderr.write(exc.stderr or "")
        return exc.returncode
    except Exception as exc:  # pylint: disable=broad-except
        sys.stderr.write(f"{exc}\n")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
