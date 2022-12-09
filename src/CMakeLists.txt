cmake_minimum_required(VERSION 3.14)
project(AGE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Avoid warning about missing DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24,
# see also: https://cmake.org/cmake/help/latest/policy/CMP0135.html
if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.24.0)
    cmake_policy(SET CMP0135 NEW)
endif ()



###############################################################################
# AGE features

# Compile AGE with rom logging features?
# This is mostly used to analyse test roms and not suited
# for casual use (e.g. gaming) as the generated log files
# can grow really big really fast.
OPTION(COMPILE_LOGGER "compile AGE with logging enabled") # disabled by default
if (COMPILE_LOGGER)
    message("compiling AGE with logging enabled")
    ADD_DEFINITIONS(-DAGE_COMPILE_LOGGER)
else ()
    message("compiling AGE with logging disabled")
endif (COMPILE_LOGGER)

# Compile AGE with forced fifo rendering enabled?
OPTION(FORCE_FIFO_RENDERER "compile AGE with forced fifo rendering enabled") # disabled by default
if (FORCE_FIFO_RENDERER)
    message("compiling AGE with forced fifo rendering enabled")
    ADD_DEFINITIONS(-DAGE_FORCE_FIFO_RENDERER)
endif (FORCE_FIFO_RENDERER)



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
# (condition: Qt5Widgets_FOUND)
find_package(Qt5 QUIET COMPONENTS Widgets)

# age_tester requires libpng
# (condition: PNG_FOUND)
find_package(PNG QUIET)
if (PNG_FOUND)
    add_compile_definitions(LIBPNG_FOUND) # used by age_common (optional)
endif ()



###############################################################################
# code

# include common code:
#   - utility code
#   - emulator core
add_subdirectory(age_common)
add_subdirectory(age_emulator_gb)

# age_qt_gui requires Qt
if (Qt5Widgets_FOUND)
    message("Qt5 found => configuring age_qt_gui")
    add_subdirectory(age_qt_gui)
else ()
    message("Qt5 not found => age_qt_gui not available")
endif ()

# age_tester requires libpng
if (PNG_FOUND)
    message("PNG found => configuring age_tester")
    add_subdirectory(age_tester)
else ()
    message("PNG not found => age_tester not available")
endif ()

# age_wasm requires emscripten
if (EMSCRIPTEN)
    message("EMSCRIPTEN=${EMSCRIPTEN} => configuring age_wasm")
    add_subdirectory(age_wasm)
else ()
    message("EMSCRIPTEN not set => age_wasm not available")
endif ()



###############################################################################
# Google Test

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

# age google test executable
add_executable(
        age_gtest
        age_emulator_gb/common/age_gb_events.test.cpp
        age_emulator_gb/lcd/palettes/age_gb_lcd_palettes_cgb.test.cpp
)
target_link_libraries(age_gtest age_emulator_gb age_common gtest_main)
target_include_directories(age_gtest PUBLIC api)
gtest_discover_tests(age_gtest)
