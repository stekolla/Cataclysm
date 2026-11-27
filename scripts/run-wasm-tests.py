#!/usr/bin/env python3
"""Run the Wasm test page and check the real game in headless Chromium."""

import argparse
import json
import os
import subprocess
import tempfile
import time
from contextlib import closing
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from threading import Thread
from urllib.parse import quote
from urllib.request import urlopen

from websocket import create_connection


class QuietHandler(SimpleHTTPRequestHandler):
    def log_message(self, *args):
        pass


class DevTools:
    def __init__(self, socket):
        self.socket = socket
        self.message_id = 0

    def send(self, method, **params):
        self.message_id += 1
        self.socket.send(
            json.dumps({"id": self.message_id, "method": method, "params": params})
        )
        while True:
            message = json.loads(self.socket.recv())
            if message.get("id") == self.message_id:
                break
        if "error" in message:
            raise RuntimeError(message["error"]["message"])
        result = message["result"]
        if "exceptionDetails" in result:
            details = result["exceptionDetails"]
            raise RuntimeError(
                details.get("exception", {}).get("description", details["text"])
            )
        return result

    def evaluate(self, expression, *, await_promise=False):
        return self.send(
            "Runtime.evaluate",
            expression=expression,
            returnByValue=True,
            awaitPromise=await_promise,
        )["result"].get("value")

    def resize(self, width, height, scale):
        self.send(
            "Emulation.setDeviceMetricsOverride",
            width=width,
            height=height,
            deviceScaleFactor=scale,
            mobile=False,
        )


def connect_browser(browser, profile):
    active_port = profile / "DevToolsActivePort"
    deadline = time.monotonic() + 30
    while time.monotonic() < deadline:
        if browser.poll() is not None:
            raise RuntimeError(f"Chromium exited with code {browser.returncode}.")
        try:
            port = int(active_port.read_text(encoding="ascii").splitlines()[0])
            with urlopen(f"http://127.0.0.1:{port}/json/list", timeout=2) as response:
                pages = json.load(response)
            for page in pages:
                if page["type"] == "page":
                    return create_connection(
                        page["webSocketDebuggerUrl"], timeout=30, suppress_origin=True
                    )
        except (OSError, ValueError, IndexError):
            pass
        time.sleep(0.25)
    raise RuntimeError("Chromium did not start its debugging endpoint.")


def run_catch_tests(devtools, url):
    devtools.send("Page.navigate", url=url)
    deadline = time.monotonic() + 15 * 60
    output_length = 0
    while time.monotonic() < deadline:
        state = devtools.evaluate("""({
            code: document.body?.dataset.testExitCode,
            output: document.querySelector('#output')?.textContent || ''
        })""")
        output = state["output"]
        if len(output) > output_length:
            print(output[output_length:], end="", flush=True)
            output_length = len(output)
        if "code" in state:
            if state["code"] != "0":
                raise RuntimeError(f"Wasm tests failed with exit code {state['code']}.")
            return
        time.sleep(1)
    raise RuntimeError("Wasm tests timed out after 15 minutes.")


SIZE_STATE = """(() => {
            const canvas = document.querySelector("#canvas");
            if (typeof FS === "undefined" || !FS.analyzePath("/userdata/config/options.json").exists) {
              return {output: "waiting for game options"};
            }
            const options = JSON.parse(FS.readFile("/userdata/config/options.json", {encoding: "utf8"}));
            const option = name => options.find(entry => entry.name === name)?.value;
            const rect = canvas.getBoundingClientRect();
            return {output: JSON.stringify({
              width: rect.width, height: rect.height,
              pixelWidth: canvas.width, pixelHeight: canvas.height,
              terminalWidth: Number(option("TERMINAL_X")), terminalHeight: Number(option("TERMINAL_Y")),
              fontWidth: Number(option("FONT_WIDTH")), fontHeight: Number(option("FONT_HEIGHT")),
              tiles: option("TILES"), overmapTiles: option("OVERMAP_TILES"),
              bundledTiles: FS.readdir("/gfx").filter(name => name !== "." && name !== ".."),
              status: document.querySelector("#status").textContent
            })};
          })()"""

SAVE_LEGACY_PROFILE = """(async () => {
        const path = "/userdata/config/options.json";
        const options = JSON.parse(FS.readFile(path, {encoding: "utf8"}));
        for (const option of options) {
          if (option.name === "TILES" || option.name === "OVERMAP_TILES") option.value = "retrodays";
        }
        FS.writeFile(path, JSON.stringify(options));
        FS.writeFile("/userdata/resize-test-preserved", "preserved");
        if (!await flushBrowserSaves()) throw new Error("Could not save legacy test profile");
        return {output: "saved"};
      })()"""


def check_game_size(devtools, width, height, scale):
    devtools.resize(width, height, scale)
    deadline = time.monotonic() + 180
    last_state = "no browser state"
    while time.monotonic() < deadline:
        last_state = devtools.evaluate(SIZE_STATE)["output"]
        if last_state.startswith("{"):
            state = json.loads(last_state)
            font_width = state["fontWidth"] or 0
            font_height = state["fontHeight"] or 0
            expected_width = max(width, font_width * 80)
            expected_height = max(height, font_height * 24)
            if (
                font_width > 0
                and font_height > 0
                and state["width"] == expected_width
                and state["height"] == expected_height
                and state["pixelWidth"] == expected_width * scale
                and state["pixelHeight"] == expected_height * scale
                and state["terminalWidth"] == expected_width // font_width
                and state["terminalHeight"] == expected_height // font_height
                and state["tiles"] == state["overmapTiles"] == "UNDEAD_PEOPLE_BASE"
                and state["bundledTiles"] == ["MSX++UnDeadPeopleEdition"]
            ):
                return
            if state["status"].startswith("Cataclysm stopped:"):
                raise RuntimeError(state["status"])
        time.sleep(0.5)
    raise RuntimeError(
        f"Game did not resize to {width}x{height} at {scale}x: {last_state}"
    )


def check_game(devtools, base_url):
    devtools.resize(1280, 720, 2)
    devtools.send("Page.navigate", url=f"{base_url}/cataclysm-bn-tiles.html")
    for width, height in [(1280, 720), (1024, 640), (500, 300)]:
        check_game_size(devtools, width, height, 2)

    devtools.evaluate(SAVE_LEGACY_PROFILE, await_promise=True)
    devtools.send("Page.reload")
    check_game_size(devtools, 500, 300, 2)
    preserved = devtools.evaluate(
        'FS.readFile("/userdata/resize-test-preserved", {encoding: "utf8"})'
    )
    if preserved != "preserved":
        raise RuntimeError("Reload did not preserve the browser profile")
    print("Browser game sizing, tileset, and saved-profile checks passed.")


def run(test_page, chrome):
    handler = partial(QuietHandler, directory=test_page.parent)
    with ThreadingHTTPServer(("127.0.0.1", 0), handler) as server:
        server_thread = Thread(target=server.serve_forever, daemon=True)
        server_thread.start()
        try:
            base_url = f"http://127.0.0.1:{server.server_port}"
            with tempfile.TemporaryDirectory(prefix="cata-wasm-tests-") as temp:
                profile = Path(temp)
                browser = subprocess.Popen(
                    [
                        chrome,
                        "--headless=new",
                        "--no-sandbox",
                        "--enable-unsafe-swiftshader",
                        "--remote-debugging-port=0",
                        f"--user-data-dir={profile}",
                        "about:blank",
                    ],
                    stdout=subprocess.DEVNULL,
                )
                try:
                    with closing(connect_browser(browser, profile)) as socket:
                        devtools = DevTools(socket)
                        devtools.send("Page.enable")
                        devtools.send("Runtime.enable")
                        run_catch_tests(devtools, f"{base_url}/{quote(test_page.name)}")
                        check_game(devtools, base_url)
                finally:
                    if browser.poll() is None:
                        browser.terminate()
                        try:
                            browser.wait(timeout=10)
                        except subprocess.TimeoutExpired:
                            browser.kill()
                            browser.wait()
        finally:
            server.shutdown()
            server_thread.join()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "test_page", type=Path, help="Generated cata_test-tiles.html file"
    )
    args = parser.parse_args()
    if not args.test_page.is_file():
        parser.error(f"Test page does not exist: {args.test_page}")
    chrome = os.environ.get("CHROME_BIN")
    if not chrome:
        parser.error("Set CHROME_BIN to a Chromium or Chrome executable.")
    run(args.test_page.resolve(), chrome)


if __name__ == "__main__":
    main()
