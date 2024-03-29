#
# Notes to myself:
#
#   - the exact build commands are logged with -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
#
#   - Apple Clang seems to be a stripped down version of LLVM-Clang,
#     some things will not work (e.g. -fuse-ld=lld):
#     https://stackoverflow.com/questions/60944732/using-fuse-ld-lld-with-clang-on-macos
#
cmake_minimum_required(VERSION 3.14)
project(AGE)

# options
OPTION(USE_CLANG_TIDY "Run clang-tidy") # disabled by default
OPTION(COMPILE_LOGGER "Compile AGE with logging enabled") # disabled by default
OPTION(FORCE_FIFO_RENDERER "Compile AGE with forced fifo rendering enabled") # disabled by default

# set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Avoid warning about missing DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24,
# see also: https://cmake.org/cmake/help/latest/policy/CMP0135.html
if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.24.0)
    cmake_policy(SET CMP0135 NEW)
endif ()

# log compiler details
if (CMAKE_CXX_COMPILER_LOADED)
    message(STATUS "Compiler path:           ${CMAKE_CXX_COMPILER}")
    message(STATUS "Compiler ID:             ${CMAKE_CXX_COMPILER_ID}")
    message(STATUS "Compiler version:        ${CMAKE_CXX_COMPILER_VERSION}")
    message(STATUS "Compiler is part of GCC: ${CMAKE_COMPILER_IS_GNUCXX}")
endif ()

# Enable link time optimizations for all targets (only for Release builds).
# Note that this will not work for e.g. -DCMAKE_BUILD_TYPE=ReLeAsE
# https://gitlab.kitware.com/cmake/cmake/-/issues/22591
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif ()



###############################################################################
# options

# run clang-tidy?
if (USE_CLANG_TIDY)
    find_program(CLANG_TIDY NAMES "clang-tidy-${CLANG_TIDY_VERSION}" "clang-tidy")
    if (CLANG_TIDY)
        message(STATUS "Found ${CLANG_TIDY}, adding linting targets")

        set(CMAKE_CXX_CLANG_TIDY
                clang-tidy;
                -header-filter=.;)
    else ()
        message(STATUS "Clang-tidy not found!")
    endif ()
endif ()

# Compile AGE with rom logging features?
# This is mostly used to analyse test roms and not suited
# for casual use (e.g. gaming) as the generated log files
# can grow really big really fast.
if (COMPILE_LOGGER)
    message(STATUS "Compiling AGE with logging enabled")
    add_definitions(-DAGE_COMPILE_LOGGER)
else ()
    message(STATUS "Compiling AGE with logging disabled")
endif ()

# Compile AGE with forced fifo rendering enabled?
if (FORCE_FIFO_RENDERER)
    message(STATUS "Compiling AGE with forced fifo rendering enabled")
    add_definitions(-DAGE_FORCE_FIFO_RENDERER)
endif ()



###############################################################################
# libraries used to conditionally add subdirectories

# macos:
# don't mix libpng headers and libraries of different versions,
# e.g. brew-installs and frameworks like Mono
#
# see also:
#   https://cmake.org/cmake/help/latest/variable/CMAKE_FIND_FRAMEWORK.html
#   https://stackoverflow.com/a/64054469
set(CMAKE_FIND_FRAMEWORK LAST)

# age_qt_gui requires Qt
# (condition: Qt6Core_FOUND)
find_package(Qt6 QUIET COMPONENTS Core)

# age_test_runner requires libpng
# (condition: PNG_FOUND)
find_package(PNG QUIET)
if (PNG_FOUND)
    add_compile_definitions(LIBPNG_FOUND) # used by age_common (optional)
endif ()



###############################################################################
# targets

# include common code:
#   - utility code
#   - emulator core
add_subdirectory(age_common)
add_subdirectory(age_emulator_gb)

# age_qt_gui requires Qt
if (Qt6Core_FOUND)
    message(STATUS "Qt6 found => configuring age_qt_gui")
    add_subdirectory(age_qt_gui)
else ()
    message(STATUS "Qt6 not found => age_qt_gui not available")
endif ()

# age_test_runner requires libpng
if (PNG_FOUND)
    message(STATUS "PNG found => configuring age_test_runner")
    add_subdirectory(age_test_runner)
else ()
    message(STATUS "PNG not found => age_test_runner not available")
endif ()

# age_wasm requires emscripten
if (EMSCRIPTEN)
    message(STATUS "EMSCRIPTEN=${EMSCRIPTEN} => configuring age_wasm")
    add_subdirectory(age_wasm)
else ()
    message(STATUS "EMSCRIPTEN not set => age_wasm not available")
endif ()



###############################################################################
# Google Test

# deactivate link time optimizations for Google Test
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)

# include Google Test, see also:
# https://google.github.io/googletest/quickstart-cmake.html
# (commit b9d2e1f62f3d93c1c3197900ba7170fa5177ad23 created on 2022-06-25)
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/b9d2e1f62f3d93c1c3197900ba7170fa5177ad23.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

# silence clang-tidy for googletest
# https://discourse.cmake.org/t/how-to-prevent-clang-tidy-to-check-sources-added-with-fetchcontents/2871/12
set_target_properties(gtest PROPERTIES CXX_CLANG_TIDY "")
set_target_properties(gtest_main PROPERTIES CXX_CLANG_TIDY "")
set_target_properties(gmock PROPERTIES CXX_CLANG_TIDY "")
set_target_properties(gmock_main PROPERTIES CXX_CLANG_TIDY "")

# age google test executable
add_executable(
        age_gtest
        age_emulator_gb/common/age_gb_events.test.cpp
        age_emulator_gb/lcd/palettes/age_gb_lcd_palettes_cgb.test.cpp
        age_test_runner/modules/age_tr_module.cpp
        age_test_runner/modules/age_tr_module.test.cpp
)
target_link_libraries(age_gtest age_emulator_gb age_common gtest_main)
target_include_directories(age_gtest PUBLIC api)
gtest_discover_tests(age_gtest)
