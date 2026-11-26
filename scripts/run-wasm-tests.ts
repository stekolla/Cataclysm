const testPage = Deno.args[0];
if (!testPage) throw new Error("Pass the generated cata_test-tiles.html path.");

const chrome = Deno.env.get("CHROME_BIN");
if (!chrome) throw new Error("Set CHROME_BIN to a Chromium or Chrome executable.");

const port = 8765;
const debugPort = 9222;
const pageName = testPage.split(/[\\/]/).pop();
const webDirectory = testPage.slice(0, testPage.lastIndexOf("/"));
const server = new Deno.Command("python3", {
  args: ["-m", "http.server", String(port), "--bind", "127.0.0.1", "--directory", webDirectory],
  stdout: "null",
  stderr: "null",
}).spawn();
const profile = await Deno.makeTempDir({ prefix: "cata-wasm-tests-" });
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
}).spawn();

try {
  const serverDeadline = Date.now() + 10_000;
  while (Date.now() < serverDeadline) {
    try {
      await fetch(`http://127.0.0.1:${port}/${pageName}`);
      break;
    } catch { /* Wait until the local HTTP server is listening. */ }
    await new Promise((resolve) => setTimeout(resolve, 100));
  }
  const deadline = Date.now() + 30_000;
  let target: { webSocketDebuggerUrl: string } | undefined;
  while (Date.now() < deadline) {
    try {
      const response = await fetch(`http://127.0.0.1:${debugPort}/json/list`);
      const pages = await response.json();
      target = pages.find((item: { type: string }) => item.type === "page");
      if (target) break;
    } catch { /* Chrome has not opened its debugging endpoint yet. */ }
    await new Promise((resolve) => setTimeout(resolve, 250));
  }
  if (!target) throw new Error("Chromium did not start its debugging endpoint.");

  const socket = new WebSocket(target.webSocketDebuggerUrl);
  await new Promise<void>((resolve, reject) => {
    socket.onopen = () => resolve();
    socket.onerror = () => reject(new Error("Could not connect to Chromium DevTools."));
  });
  let nextId = 0;
  const pending = new Map<number, (value: unknown) => void>();
  socket.onmessage = (event) => {
    const message = JSON.parse(event.data);
    if (message.id !== undefined) pending.get(message.id)?.(message);
  };
  const send = (method: string, params = {}) => new Promise<any>((resolve) => {
    const id = ++nextId;
    pending.set(id, resolve);
    socket.send(JSON.stringify({ id, method, params }));
  });

  await send("Page.enable");
  await send("Runtime.enable");
  await send("Page.navigate", { url: `http://127.0.0.1:${port}/${pageName}` });

  const testDeadline = Date.now() + 15 * 60_000;
  let outputLength = 0;
  while (Date.now() < testDeadline) {
    const result = await send("Runtime.evaluate", {
      expression: "({code: document.body.dataset.testExitCode, output: document.querySelector('#output')?.textContent || ''})",
      returnByValue: true,
    });
    const value = result.result?.result?.value;
    if (value?.output?.length > outputLength) {
      Deno.stdout.write(new TextEncoder().encode(value.output.slice(outputLength)));
      outputLength = value.output.length;
    }
    if (value?.code !== undefined) {
      socket.close();
      if (value.code !== "0") throw new Error(`Wasm tests failed with exit code ${value.code}.`);
      break;
    }
    await new Promise((resolve) => setTimeout(resolve, 1000));
  }
  if (Date.now() >= testDeadline) throw new Error("Wasm tests timed out after 15 minutes.");
} finally {
  browser.kill("SIGTERM");
  server.kill("SIGTERM");
  await Deno.remove(profile, { recursive: true });
}
