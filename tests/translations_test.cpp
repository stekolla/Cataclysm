#include "catch/catch.hpp"
#include "string_formatter.h"
#include "translations.h"

// Keep the test string out of source string extraction checks.
#ifndef TRANSLATE_MACRO
#    define TRANSLATE_MACRO(msg) _(msg)
#endif

// Keep the test string out of source string extraction checks.
#ifndef TRANSLATE_TRANSLATION
#    define TRANSLATE_TRANSLATION(msg) to_translation(msg).translated()
#endif

TEST_CASE("translations_sanity_test", "[translations][i18n]") {
    const std::string test_string = "__untranslated_test_string__";

    CHECK(TRANSLATE_MACRO(test_string) == test_string);
    CHECK(TRANSLATE_TRANSLATION(test_string) == test_string);
}

// English-only string wrappers must return the input unchanged.
TEST_CASE("translations_macro_string_stability", "[translations][i18n]") {
    std::vector<std::string> test_strings;

    SECTION("different input strings") {
        test_strings = {"__untranslated_string1__", "__untranslated_string2__"};
    }

    SECTION("ordinary English strings") { test_strings = {"thread", "yarn"}; }

    std::string test_arg;
    // note: this TRANSLATE_MACRO called in different iterations of the `for` loop will have
    //  the same static cache
    for (const std::string& test_string : test_strings) {
        test_arg.assign(test_string);
        // translation result should be assignable to the `const std::string &` local variable
        const std::string& res1 = TRANSLATE_MACRO(test_arg);
        CHECK(res1 == test_string);
    }
}

// The string wrapper must preserve the current input pointer when it returns that input.
TEST_CASE("translations_macro_char_address", "[translations][i18n]") {
    SECTION("literal input address is preserved") {
        const char* test_string = "__untranslated_string1__";
        CHECK(TRANSLATE_MACRO(test_string) == test_string);
    }

    SECTION("current argument address is returned") {
        // wrapped in lambda to ensure that different calls hit the same internal cache
        const auto translate = [](const char* msg) { return TRANSLATE_MACRO(msg); };

        // same content, different address (std::string is used to ensure that)
        const std::string test_string1 = "__untranslated_string1__";
        const std::string test_string2 = "__untranslated_string1__";
        // address should be different!
        REQUIRE(test_string1.c_str() != test_string2.c_str());
        // note: address comparison is intended
        CHECK(translate(test_string1.c_str()) == test_string1.c_str());
        CHECK(translate(test_string2.c_str()) == test_string2.c_str());
        CHECK(translate(test_string1.c_str()) == test_string1.c_str());
    }
}

TEST_CASE("translations_add_context", "[translations][i18n]") {
    SECTION("if context is absent, set new context as translation's context") {
        translation orig = translation::to_translation("msg");
        translation exp = translation::to_translation("ctxt", "msg");
        translation combined = orig;
        REQUIRE(combined == orig);
        REQUIRE(combined != exp);
        combined.add_context("ctxt");
        REQUIRE(combined != orig);
        REQUIRE(combined == exp);
    }
    SECTION("if context is present, append new context to existing context") {
        translation orig = translation::to_translation("yay", "msg");
        translation exp = translation::to_translation("yay|ctxt", "msg");
        translation combined = orig;
        REQUIRE(combined == orig);
        REQUIRE(combined != exp);
        combined.add_context("ctxt");
        REQUIRE(combined != orig);
        REQUIRE(combined == exp);
    }
}
