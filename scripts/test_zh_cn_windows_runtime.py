"""Exercise the deployed Windows binaries, without Qt or compiler paths.

The GUI version check only verifies startup dependencies. Actual read/search
coverage uses klogg_grep, which shares the application's log and search engines.
It does not replace visual testing of menus, dialogs, or language switching.
"""

from __future__ import annotations

import argparse
import codecs
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import time


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--package-dir", type=Path, required=True)
    parser.add_argument("--report", type=Path, required=True)
    args = parser.parse_args()
    package_dir = args.package_dir.resolve(strict=True)

    def record(message: str) -> None:
        print(message, flush=True)
        with args.report.open("a", encoding="utf-8") as report:
            report.write(message + "\n")

    if os.name != "nt":
        raise RuntimeError("This test is only for the Windows x64 artifact")

    with tempfile.TemporaryDirectory(prefix="klogg-zh-cn-test-") as temporary:
        work = Path(temporary)
        runtime = work / "runtime"
        shutil.copytree(package_dir, runtime)
        environment = os.environ.copy()
        system_root = Path(environment["SystemRoot"])
        environment["PATH"] = os.pathsep.join(
            map(str, (runtime, system_root / "System32", system_root))
        )
        for variable in ("QT_PLUGIN_PATH", "QT_QPA_PLATFORM_PLUGIN_PATH", "QML2_IMPORT_PATH"):
            environment.pop(variable, None)

        def run(executable: str, arguments: list[str], timeout: int) -> subprocess.CompletedProcess:
            result = subprocess.run(
                [str(runtime / executable), *arguments],
                cwd=work,
                env=environment,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                timeout=timeout,
                creationflags=subprocess.CREATE_NO_WINDOW,
            )
            if result.returncode:
                raise RuntimeError(
                    f"{executable} exited with {result.returncode}: "
                    f"{result.stderr.decode('utf-8', errors='replace')[-4000:]}"
                )
            return result

        for executable in ("klogg.exe", "klogg_portable.exe"):
            run(executable, ["-platform", "windows", "-v"], timeout=30)
            record(f"PASS: {executable} Windows platform / dependency startup smoke (version command)")

        def search(log: Path, pattern: str, expected: list[str], timeout: int = 60) -> None:
            started = time.monotonic()
            # The console tool normally logs to stdout. Its existing debug option
            # sets log level 0 here, leaving only fatal errors and matched lines.
            result = run(
                "klogg_grep.exe", ["--debug=-3", "--pattern", pattern, str(log)], timeout
            )
            actual = result.stdout.decode("utf-8-sig").splitlines()
            if actual != expected:
                raise AssertionError(
                    f"{log.name}: pattern {pattern!r}: expected {len(expected)} matching lines, "
                    f"got {len(actual)}; output begins {actual[:4]!r}"
                )
            record(
                f"PASS: {log.name}, {pattern!r}, {len(expected)} exact matching lines "
                f"({time.monotonic() - started:.2f}s)"
            )

        lines = [
            "KLOGG_CI INFO 1001 服务已启动，正在读取配置文件。",
            "KLOGG_CI ERROR 2001 中文错误 TraceId=abc123，网络连接失败。",
            "KLOGG_CI WARN 3001 请求处理时间较长，请检查服务器状态。",
            "KLOGG_CI ERROR 2002 Exception，文件保存失败，磁盘空间不足。",
        ]
        lines.extend(
            f"KLOGG_CI INFO {4000 + i} 应用程序运行正常，用户操作成功，数据库事务已经完成。"
            for i in range(100)
        )
        text = "\n".join(lines) + "\n"
        for name, encoding, bom in (
            ("utf8", "utf-8", b""),
            ("utf16le", "utf-16-le", codecs.BOM_UTF16_LE),
            ("utf16be", "utf-16-be", codecs.BOM_UTF16_BE),
            ("gb18030", "gb18030", b""),
        ):
            log = work / f"{name}.log"
            log.write_bytes(bom + text.encode(encoding))
            search(log, r"^KLOGG_CI ERROR \d{4} ", [lines[1], lines[3]])
            search(log, "中文错误", [lines[1]])
            search(log, "TraceId=abc123", [lines[1]])
            search(log, "__KLOGG_CI_NO_MATCH__", [])

        emoji_line = "KLOGG_CI INFO 中文与 emoji 混合日志 😀🚀"
        emoji_log = work / "utf8-emoji.log"
        emoji_log.write_text(emoji_line + "\n", encoding="utf-8")
        search(emoji_log, "emoji", [emoji_line])

        # More than 1 GiB, real bytes with bounded RAM use, and a rare match at the
        # very end: the reader/indexer must process the entire file to find it.
        large_log = work / "over-1GiB.log"
        filler = (b"KLOGG_CI INFO " + b"x" * (4096 - 15) + b"\r\n") * 1024
        last_line = "KLOGG_CI ERROR 9001 large_file_tail_marker"
        with large_log.open("wb") as log:
            for _ in range(256):
                log.write(filler)
            log.write((last_line + "\r\n").encode("ascii"))
        if large_log.stat().st_size <= 1024**3:
            raise AssertionError("Large-file fixture must exceed 1 GiB")
        search(large_log, r"^KLOGG_CI ERROR 9001 large_file_tail_marker$", [last_line], timeout=180)
        record("PASS: all runtime tests used a temporary package copy and isolated PATH")


if __name__ == "__main__":
    main()
