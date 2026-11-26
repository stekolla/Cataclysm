#pragma once

#include <expected>
#include <string>

namespace browser_storage {

/// Flush a completed save to browser storage and show any error before returning.
#if defined(__EMSCRIPTEN__)
auto flush_with_feedback() -> std::expected<void, std::string>;
#else
inline auto flush_with_feedback() -> std::expected<void, std::string> { return {}; }
#endif

} // namespace browser_storage
