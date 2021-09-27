cmake_minimum_required(VERSION 3.14)
project(AGE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



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

# include common code:
#   - utility code
#   - emulator core
add_subdirectory(age_common)
add_subdirectory(age_emulator_gb)

# age_qt_gui requires Qt
find_package(Qt5 QUIET COMPONENTS Widgets)
if (Qt5Widgets_FOUND)
    message("Qt5 found => configuring age_qt_gui")
    add_subdirectory(age_qt_gui)
else ()
    message("Qt5 not found => age_qt_gui not available")
endif ()

# age_cgb_lut and age_tester require libpng
find_package(PNG QUIET)
if (PNG_FOUND)
    message("PNG found => configuring age_cgb_lut and age_tester")
    add_compile_definitions(LIBPNG_FOUND)
    add_subdirectory(age_cgb_lut)
    add_subdirectory(age_tester)
else ()
    message("PNG not found => age_cgb_lut and age_tester not available")
endif ()

# age_wasm requires emscripten
if (EMSCRIPTEN)
    message("EMSCRIPTEN=${EMSCRIPTEN} => configuring age_wasm")
    add_subdirectory(age_wasm)
else ()
    message("EMSCRIPTEN not set => age_wasm not available")
endif ()



# include Google Test, see also:
# https://google.github.io/googletest/quickstart-cmake.html
# (commit 8d51ffdfab10b3fba636ae69bc03da4b54f8c235 created on 2021-07-09)
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/8d51ffdfab10b3fba636ae69bc03da4b54f8c235.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

# age google test executable
add_executable(
        age_gtest
        age_emulator_gb/common/age_gb_events.test.cpp
        age_emulator_gb/lcd/age_gb_lcd_palettes_cgb.test.cpp
)
target_link_libraries(age_gtest age_emulator_gb age_common gtest_main)
target_include_directories(age_gtest PUBLIC api)
gtest_discover_tests(age_gtest)
