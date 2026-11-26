file(GLOB SOL_SOURCES config.hpp forward.hpp sol.hpp)

add_library(libsol INTERFACE ${SOL_SOURCES})
# HACK: "/.." here keeps the generated path layout compatible with existing includes.
#       Get rid of it once we get rid of Make, and move sol sources into subdirectory.
if(MSVC)
    target_include_directories(libsol SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
else()
    target_compile_options(libsol INTERFACE -idirafter ${CMAKE_CURRENT_SOURCE_DIR}/..)
endif()
target_link_libraries(libsol INTERFACE liblua)
