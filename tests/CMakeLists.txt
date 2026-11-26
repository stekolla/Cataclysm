file(GLOB CATACLYSM_BN_TEST_SOURCES ${CMAKE_SOURCE_DIR}/tests/*.cpp)

# Catch2 v2 uses __COUNTER__, which Clang 22 reports under -Wpedantic.
set(CATCH2_COMPILE_OPTIONS $<$<CXX_COMPILER_ID:Clang>:-Wno-c2y-extensions>)

if (CATA_TESTS_ONLY)
    if (EMSCRIPTEN OR NOT CURSES OR TILES OR SOUND OR CATA_SDL)
        message(FATAL_ERROR "Native tests require the Linux curses test-only configuration.")
    endif()

    add_executable(cata_test ${CATACLYSM_BN_TEST_SOURCES})
    target_link_libraries(cata_test PRIVATE cataclysm-bn-common)
    target_compile_options(cata_test PRIVATE ${CATCH2_COMPILE_OPTIONS})
    target_compile_definitions(cata_test PRIVATE CATCH_CONFIG_ENABLE_BENCHMARKING)
    add_test(
        NAME test
        COMMAND $<TARGET_FILE:cata_test> --rng-seed 1 "~[slow]~[wasm]~[.]"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
elseif (EMSCRIPTEN AND TILES AND NOT CURSES)
    add_executable(cata_test-tiles ${CATACLYSM_BN_TEST_SOURCES})
    target_link_libraries(cata_test-tiles PRIVATE cataclysm-bn-tiles-common)
    target_compile_options(cata_test-tiles PRIVATE ${CATCH2_COMPILE_OPTIONS})
    target_compile_definitions(cata_test-tiles PRIVATE CATCH_CONFIG_ENABLE_BENCHMARKING)
    set_property(TARGET cata_test-tiles PROPERTY CATA_WASM_TEST_RUNNER ON)
    cata_configure_web_target(cata_test-tiles)

    add_test(
        NAME test-tiles
        COMMAND ${DENO_BINARY} run -A
            ${CMAKE_SOURCE_DIR}/scripts/run-wasm-tests.ts
            $<TARGET_FILE:cata_test-tiles>
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
else()
    message(FATAL_ERROR "Tests are supported only in the Linux native test-only and Emscripten tiles configurations.")
endif()
