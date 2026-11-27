#!/usr/bin/env python3
"""Verify that Ctrl+C stops the runner and its container during builds and serving."""

import errno
import os
import pty
import select
import signal
import subprocess
import time
import uuid


def containers_for(marker: str) -> list[str]:
    result = subprocess.run(
        ["docker", "ps", "--no-trunc", "--format", "{{.ID}}\t{{.Command}}"],
        check=True,
        capture_output=True,
        text=True,
        timeout=30,
    )
    return [line.split("\t", 1)[0] for line in result.stdout.splitlines() if marker in line]


def check_interrupt(command: str, ready: str, wait_for_sleep: bool = False) -> None:
    marker = f"cata-signal-test-{uuid.uuid4().hex}"
    pid, fd = pty.fork()
    if pid == 0:
        os.execvp(
            "bash",
            ["bash", "build-scripts/run-docker.sh", "bash", "-c", command, "_", marker],
        )

    reaped = False
    try:
        output = b""
        deadline = time.monotonic() + 60
        while ready.encode() not in output and time.monotonic() < deadline:
            readable, _, _ = select.select([fd], [], [], 0.2)
            if readable:
                try:
                    chunk = os.read(fd, 65536)
                except OSError as error:
                    if error.errno == errno.EIO:
                        break
                    raise
                if not chunk:
                    break
                output += chunk
        if ready.encode() not in output:
            raise RuntimeError(f"runner did not become ready: {output.decode(errors='replace')}")
        containers = containers_for(marker)
        if len(containers) != 1:
            raise RuntimeError("expected exactly one test container")
        if wait_for_sleep:
            deadline = time.monotonic() + 30
            while time.monotonic() < deadline:
                result = subprocess.run(
                    ["docker", "exec", containers[0], "pidof", "sleep"],
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL,
                    timeout=30,
                    check=False,
                )
                if result.returncode == 0:
                    break
                time.sleep(0.1)
            else:
                raise RuntimeError("build subprocess did not start")

        os.write(fd, b"\x03")
        deadline = time.monotonic() + 30
        while time.monotonic() < deadline:
            readable, _, _ = select.select([fd], [], [], 0.2)
            if readable:
                try:
                    output += os.read(fd, 65536)
                except OSError as error:
                    if error.errno != errno.EIO:
                        raise
            finished, _ = os.waitpid(pid, os.WNOHANG)
            if finished:
                reaped = True
                break
        if not reaped:
            raise RuntimeError("runner did not exit after Ctrl+C")
        if containers_for(marker):
            raise RuntimeError("container remained running after Ctrl+C")
    finally:
        for container in containers_for(marker):
            subprocess.run(["docker", "stop", "-t", "2", container], check=True, timeout=30)
        if not reaped:
            try:
                os.kill(pid, signal.SIGKILL)
            except ProcessLookupError:
                pass
        os.close(fd)


if __name__ == "__main__":
    check_interrupt('echo "$1"; sleep 600; echo unexpected', "cata-signal-test-", True)
    check_interrupt('echo "$1"; exec python3 -u -m http.server 18923', "Serving HTTP")
    print("Ctrl+C stops build and server containers")
