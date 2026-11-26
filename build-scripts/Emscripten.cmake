# Install headers and archives in the SDK sysroot before CMake searches for them.
get_filename_component(EMSCRIPTEN_BIN_DIR "${CMAKE_C_COMPILER}" DIRECTORY)
execute_process(
    COMMAND "${EMSCRIPTEN_BIN_DIR}/embuilder" build zlib sqlite3 freetype-legacysjlj
    COMMAND_ERROR_IS_FATAL ANY
)

# FreeType's setjmp/longjmp must use the same exception mechanism as the game.
find_library(CATA_WASM_FREETYPE NAMES freetype-legacysjlj REQUIRED)
set(FREETYPE_LIBRARY_RELEASE "${CATA_WASM_FREETYPE}")
set(BUILD_SHARED_LIBS OFF)
add_compile_options(
    "$<$<COMPILE_LANGUAGE:CXX>:-fwasm-exceptions>"
    "$<$<COMPILE_LANGUAGE:C>:-sSUPPORT_LONGJMP=wasm>"
)

function(cata_configure_web_target target)
    set(web_dir "${CMAKE_BINARY_DIR}/web")
    get_target_property(is_test_runner ${target} CATA_WASM_TEST_RUNNER)
    if (is_test_runner)
        set(shell "${CMAKE_SOURCE_DIR}/build-data/web/test_shell.html")
    else()
        set(shell "${CMAKE_SOURCE_DIR}/build-data/web/shell.html")
    endif()
    set(storage "${CMAKE_SOURCE_DIR}/build-data/web/storage.js")
    set(data "${CMAKE_SOURCE_DIR}/data")
    set(tiles "${CMAKE_SOURCE_DIR}/gfx/RetroDaysTileset")
    file(GLOB_RECURSE assets CONFIGURE_DEPENDS "${data}/*" "${tiles}/*")
    set(test_exit_runtime)
    if (is_test_runner)
        set(test_exit_runtime -sEXIT_RUNTIME=1)
        set(test_data "${CMAKE_SOURCE_DIR}/tests/data/TEST_DATA")
        file(GLOB_RECURSE test_assets CONFIGURE_DEPENDS "${test_data}/*")
        set_property(TARGET ${target} APPEND PROPERTY LINK_DEPENDS ${test_assets})
        target_link_options(${target} PRIVATE "SHELL:--preload-file '${test_data}@/tests/data/TEST_DATA'")
        foreach(test_tileset ASCIITileset HitButton_iso)
            set(test_tiles "${CMAKE_SOURCE_DIR}/gfx/${test_tileset}")
            file(GLOB_RECURSE test_tile_assets CONFIGURE_DEPENDS "${test_tiles}/*")
            set_property(TARGET ${target} APPEND PROPERTY LINK_DEPENDS ${test_tile_assets})
            target_link_options(${target} PRIVATE "SHELL:--preload-file '${test_tiles}@/gfx/${test_tileset}'")
        endforeach()
    endif()

    set_target_properties(${target} PROPERTIES
        SUFFIX ".html"
        RUNTIME_OUTPUT_DIRECTORY "${web_dir}"
    )
    set_property(TARGET ${target} APPEND PROPERTY LINK_DEPENDS "${shell}" "${storage}" ${assets})
    target_link_options(${target} PRIVATE
        # Keep the compiler's Release optimization and skip whole-program optimization.
        -O1
        -fwasm-exceptions
        # SDL_Delay yields to the browser during the game's blocking input waits.
        -sJSPI
        -sSTACK_SIZE=2097152
        -sALLOW_MEMORY_GROWTH=1
        -sENVIRONMENT=web
        ${test_exit_runtime}
        -lidbfs.js
        "SHELL:--pre-js '${storage}'"
        "SHELL:--shell-file '${shell}'"
        "SHELL:--preload-file '${data}@/data'"
        "SHELL:--preload-file '${tiles}@/gfx/RetroDaysTileset'"
    )
endfunction()
