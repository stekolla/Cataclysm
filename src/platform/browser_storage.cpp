#include "browser_storage.h"

#if defined(__EMSCRIPTEN__)

#    include "output.h"
#    include "translations.h"

#    include <emscripten.h>

namespace {

EM_ASYNC_JS(int, sync_browser_saves, (), { return await flushBrowserSaves(); });

} // namespace

auto browser_storage::flush_with_feedback() -> std::expected<void, std::string> {
    if (sync_browser_saves()) { return {}; }

    const auto message = _(
        "Could not write to browser storage. Try saving again before reloading.");
    popup(message);
    return std::unexpected(message);
}

#endif
