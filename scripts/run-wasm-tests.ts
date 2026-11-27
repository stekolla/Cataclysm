#!/usr/bin/env -S deno run --allow-read --allow-write --allow-run --allow-env --allow-net
/**
 * @module
 * Run the browser test page and check the game in headless Chromium.
 */
async function removeProfile(profile: string): Promise<void> {
  for (let attempt = 0;; ++attempt) {
    try {
      await Deno.remove(profile, { recursive: true })
      return
    } catch (error) {
      if (
        !(error instanceof Error && "code" in error && error.code === "ENOTEMPTY") || attempt >= 19
      ) throw error
      await new Promise((resolve) => setTimeout(resolve, 100))
    }
  }
}

if (import.meta.main) {
  const testPage = Deno.args[0]
  if (!testPage) throw new Error("Pass the generated cata_test-tiles.html path.")

  const chrome = Deno.env.get("CHROME_BIN")
  if (!chrome) throw new Error("Set CHROME_BIN to a Chromium or Chrome executable.")

  const port = 8765
  const debugPort = 9222
  const pageName = testPage.split(/[\\/]/).pop()
  const webDirectory = testPage.slice(0, testPage.lastIndexOf("/"))
  const server = new Deno.Command("python3", {
    args: ["-m", "http.server", String(port), "--bind", "127.0.0.1", "--directory", webDirectory],
    stdout: "null",
    stderr: "null",
  }).spawn()
  const profile = await Deno.makeTempDir({ prefix: "cata-wasm-tests-" })
  const browser = new Deno.Command(chrome, {
    args: [
      "--headless=new",
      "--no-sandbox",
      "--enable-unsafe-swiftshader",
      "--remote-allow-origins=*",
      `--remote-debugging-port=${debugPort}`,
      `--user-data-dir=${profile}`,
      "about:blank",
    ],
    stdout: "null",
    stderr: "null",
  }).spawn()

  try {
    const serverDeadline = Date.now() + 10_000
    while (Date.now() < serverDeadline) {
      try {
        await fetch(`http://127.0.0.1:${port}/${pageName}`)
        break
      } catch { /* Wait until the local HTTP server is listening. */ }
      await new Promise((resolve) => setTimeout(resolve, 100))
    }
    const deadline = Date.now() + 30_000
    let target: { webSocketDebuggerUrl: string } | undefined
    while (Date.now() < deadline) {
      try {
        const response = await fetch(`http://127.0.0.1:${debugPort}/json/list`)
        const pages = await response.json()
        target = pages.find((item: { type: string }) => item.type === "page")
        if (target) break
      } catch { /* Chrome has not opened its debugging endpoint yet. */ }
      await new Promise((resolve) => setTimeout(resolve, 250))
    }
    if (!target) throw new Error("Chromium did not start its debugging endpoint.")

    const socket = new WebSocket(target.webSocketDebuggerUrl)
    await new Promise<void>((resolve, reject) => {
      socket.onopen = () => resolve()
      socket.onerror = () => reject(new Error("Could not connect to Chromium DevTools."))
    })
    let nextId = 0
    type Response = {
      id?: number
      error?: { message: string }
      result?: {
        exceptionDetails?: { text: string }
        result?: { value?: { code?: string; output: string } }
      }
    }
    const pending = new Map<number, (value: Response) => void>()
    socket.onmessage = (event) => {
      const message = JSON.parse(event.data)
      if (message.id !== undefined) pending.get(message.id)?.(message)
    }
    const send = (method: string, params = {}) =>
      new Promise<Response>((resolve, reject) => {
        const id = ++nextId
        pending.set(id, (response) => {
          pending.delete(id)
          if (response.error) reject(new Error(response.error.message))
          else if (response.result?.exceptionDetails) {
            reject(new Error(response.result.exceptionDetails.text))
          } else resolve(response)
        })
        socket.send(JSON.stringify({ id, method, params }))
      })

    await send("Page.enable")
    await send("Runtime.enable")
    await send("Page.navigate", { url: `http://127.0.0.1:${port}/${pageName}` })

    const testDeadline = Date.now() + 15 * 60_000
    let outputLength = 0
    while (Date.now() < testDeadline) {
      const result = await send("Runtime.evaluate", {
        expression:
          "({code: document.body.dataset.testExitCode, output: document.querySelector('#output')?.textContent || ''})",
        returnByValue: true,
      })
      const value = result.result?.result?.value
      if (value && value.output.length > outputLength) {
        Deno.stdout.write(new TextEncoder().encode(value.output.slice(outputLength)))
        outputLength = value.output.length
      }
      if (value?.code !== undefined) {
        if (value.code !== "0") throw new Error(`Wasm tests failed with exit code ${value.code}.`)
        break
      }
      await new Promise((resolve) => setTimeout(resolve, 1000))
    }
    if (Date.now() >= testDeadline) throw new Error("Wasm tests timed out after 15 minutes.")

    /// Check the real game after startup and each browser resize: the backing canvas and the
    /// game terminal must both follow the viewport, including high DPI and minimum-size windows.
    const checkGameSize = async (width: number, height: number, scale: number) => {
      await send("Emulation.setDeviceMetricsOverride", {
        width,
        height,
        deviceScaleFactor: scale,
        mobile: false,
      })
      const deadline = Date.now() + 180_000
      let lastState = ""
      while (Date.now() < deadline) {
        const result = await send("Runtime.evaluate", {
          expression: `(() => {
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
          })()`,
          returnByValue: true,
        })
        lastState = result.result?.result?.value?.output ?? "no browser state"
        if (lastState.startsWith("{")) {
          const state = JSON.parse(lastState)
          const expectedWidth = Math.max(width, state.fontWidth * 80)
          const expectedHeight = Math.max(height, state.fontHeight * 24)
          if (
            state.width === expectedWidth && state.height === expectedHeight &&
            state.pixelWidth === expectedWidth * scale &&
            state.pixelHeight === expectedHeight * scale &&
            state.terminalWidth === Math.floor(expectedWidth / state.fontWidth) &&
            state.terminalHeight === Math.floor(expectedHeight / state.fontHeight) &&
            state.tiles === "UNDEAD_PEOPLE_BASE" && state.overmapTiles === "UNDEAD_PEOPLE_BASE" &&
            state.bundledTiles.length === 1 && state.bundledTiles[0] === "MSX++UnDeadPeopleEdition"
          ) return
          if (state.status.startsWith("Cataclysm stopped:")) throw new Error(state.status)
        }
        await new Promise((resolve) => setTimeout(resolve, 500))
      }
      throw new Error(`Game did not resize to ${width}x${height} at ${scale}x: ${lastState}`)
    }

    await send("Emulation.setDeviceMetricsOverride", {
      width: 1280,
      height: 720,
      deviceScaleFactor: 2,
      mobile: false,
    })
    await send("Page.navigate", { url: `http://127.0.0.1:${port}/cataclysm-bn-tiles.html` })
    await checkGameSize(1280, 720, 2)
    await checkGameSize(1024, 640, 2)
    await checkGameSize(500, 300, 2)

    /// A legacy browser profile must retain unrelated files and replace unavailable RetroDays
    /// selections with the only bundled tileset after a reload.
    await send("Runtime.evaluate", {
      expression: `(async () => {
        const path = "/userdata/config/options.json";
        const options = JSON.parse(FS.readFile(path, {encoding: "utf8"}));
        for (const option of options) {
          if (option.name === "TILES" || option.name === "OVERMAP_TILES") option.value = "retrodays";
        }
        FS.writeFile(path, JSON.stringify(options));
        FS.writeFile("/userdata/resize-test-preserved", "preserved");
        if (!await flushBrowserSaves()) throw new Error("Could not save legacy test profile");
        return {output: "saved"};
      })()`,
      awaitPromise: true,
      returnByValue: true,
    })
    await send("Page.reload")
    await checkGameSize(500, 300, 2)
    const preserved = await send("Runtime.evaluate", {
      expression: `({output: FS.readFile("/userdata/resize-test-preserved", {encoding: "utf8"})})`,
      returnByValue: true,
    })
    if (preserved.result?.result?.value?.output !== "preserved") {
      throw new Error("Reload did not preserve the browser profile")
    }
    console.log("Browser game sizing, tileset, and saved-profile checks passed.")
    socket.close()
  } finally {
    browser.kill("SIGTERM")
    server.kill("SIGTERM")
    await Promise.all([browser.status, server.status])
    await removeProfile(profile)
  }
}
