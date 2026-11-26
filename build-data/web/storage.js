const userDirectory = "/userdata";

function syncUserDirectory(populate) {
    return new Promise((resolve, reject) => {
        FS.syncfs(populate, (error) => error ? reject(error) : resolve());
    });
}

async function flushBrowserSaves() {
    try {
        await syncUserDirectory(false);
        return 1;
    } catch (error) {
        console.error("Could not save Cataclysm data to IndexedDB", error);
        return 0;
    }
}

Module["arguments"] ??= ["--userdir", userDirectory];
Module["preRun"] ??= [];
Module["preRun"].push(() => {
    const dependency = "browser-save-data";
    addRunDependency(dependency);

    async function restore() {
        if (!navigator.locks) {
            throw new Error("Open Cataclysm on localhost or HTTPS to use browser saves.");
        }

        // IDBFS reconciles the entire mount. Keep another tab from writing a stale copy.
        await navigator.locks.request("bright-nights-userdata", { ifAvailable: true }, async (lock) => {
            if (!lock) {
                throw new Error("Close the other Cataclysm tab and reload to use your browser saves.");
            }
            FS.mkdir(userDirectory);
            FS.mount(IDBFS, {}, userDirectory);
            await syncUserDirectory(true);
            removeRunDependency(dependency);
            // The browser releases this lock when the page closes or reloads.
            await new Promise(() => {});
        });
    }

    restore().catch((error) => abort(`Could not open browser saves. ${error.message}`));
});
