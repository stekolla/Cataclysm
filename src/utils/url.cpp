#include "url.h"

#include "debug.h"

#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <string>

#if defined(__EMSCRIPTEN__)
#    include <SDL3/SDL_error.h>
#    include <SDL3/SDL_misc.h>
#endif

auto open_url(const std::string& url) -> void {
#if defined(__EMSCRIPTEN__)
    if (!SDL_OpenURL(url.c_str())) { debugmsg("Failed to open URL: %s\n%s", url, SDL_GetError()); }
#else
    static const auto executable =
#    if defined(_WIN32)
        "start \"\"";
#    elif defined(__linux__) || defined(__FreeBSD__)
        "xdg-open";
#    elif defined(__APPLE__)
        "open";
#    elif defined(__ANDROID__)
        "am start -a android.intent.action.VIEW -d";
#    endif
    const auto command = std::string(executable) + " \"" + url + "\"";

    const auto exitcode = std::system(command.data());
    if (exitcode != 0) {
        debugmsg("Failed to open URL: %s\nAttemped command was: %s", url, command);
    }
#endif
}

auto encode_url(const std::string& text) -> std::string {
    auto escaped = std::ostringstream{};
    escaped.fill('0');
    escaped << std::hex;

    const auto accepted = std::string{"-_.~"};
    const auto is_accepted = [&accepted](const char c) {
        return std::isalnum(c) || accepted.find(c) != std::string::npos;
    };

    for (const auto& c : text) {
        if (is_accepted(c)) {
            escaped << c;
        } else {
            escaped << std::uppercase << '%' << std::setw(2)
                    << static_cast<int>(static_cast<unsigned char>(c)) << std::nouppercase;
        }
    }

    return escaped.str();
}
