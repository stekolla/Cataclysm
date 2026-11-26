#[=======================================================================[

windows-tiles-sounds-x64-msvc
-----------------------------

Pre-load script for Windows builds with Ninja Multi-Config and MSVC.

Used by CMakePresets.json -> "cacheVariables" -> "CMAKE_PROJECT_INCLUDE_BEFORE".

When CMake does not run under the VS environment (i.e. VSCMD_VER is not set),
it runs VsDevCmd.bat to bootstrap the VS toolchain environment, then writes
CMakeUserPresets.json with the baked environment variables so that subsequent
terminal builds (cmake --build --preset ...) also work outside of VS.

CMakeUserPresets.json is only written on the first configure run. If the file
already exists it is left untouched, preserving any user customizations.
See CMakeUserPresets.json.example for a reference and customization guide.

#]=======================================================================]

# Ref https://github.com/actions/virtual-environments/blob/win19/20220515.1/images/win/Windows2019-Readme.md#environment-variables
if (NOT $ENV{VCPKG_INSTALLATION_ROOT} STREQUAL "")
    set(ENV{VCPKG_ROOT} $ENV{VCPKG_INSTALLATION_ROOT})
endif()
# Ref https://vcpkg.io/en/docs/users/config-environment.html#vcpkg_root
if ("$ENV{VCPKG_ROOT}" STREQUAL "" AND WIN32)
    set(ENV{VCPKG_ROOT} $CACHE{VCPKG_ROOT})
endif()

include(${CMAKE_SOURCE_DIR}/build-scripts/VsDevCmd.cmake)

# Generate CMakeUserPresets.json from the template only on first configure.
# It is gitignored and provides the baked VS environment variables for
# terminal builds (cmake --build --preset ...) outside of the VS IDE.
# Subsequent configure runs leave the file alone so user customizations survive.
# @_MSVC_DEVENV@ may be empty (e.g. when running from VS IDE); that is fine.
set(CONFIGURE_PRESET "windows-tiles-sounds-x64-msvc")
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/CMakeUserPresets.json")
    configure_file(
        ${CMAKE_SOURCE_DIR}/build-scripts/CMakeUserPresets.json.in
        ${CMAKE_SOURCE_DIR}/CMakeUserPresets.json
        @ONLY
    )
    message(STATUS "Generated CMakeUserPresets.json for terminal builds.")
    message(STATUS "Customize it to add local build overrides (see CMakeUserPresets.json.example).")
endif()

# ccache integration for MSVC with Ninja
find_program(CCACHE_EXE ccache)
if(CCACHE_EXE)
    set(CMAKE_C_COMPILER_LAUNCHER   ccache)
    set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
endif()
