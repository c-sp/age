project(AGE)

cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


OPTION(COMPILE_LOGGER "compile AGE with logging enabled") # disabled by default
if (COMPILE_LOGGER)
    message("compiling AGE with logging enabled")
    ADD_DEFINITIONS(-DAGE_COMPILE_LOGGER)
else ()
    message("compiling AGE with logging disabled")
endif (COMPILE_LOGGER)


add_subdirectory(age_common)
add_subdirectory(age_emulator_gb)


# find Qt before including age_qt_gui
find_package(Qt5 QUIET COMPONENTS Widgets)
if (Qt5Widgets_FOUND)
    message("Qt5 found => configuring age_qt_gui")
    add_subdirectory(age_qt_gui)
else ()
    message("Qt5 not found => age_qt_gui not available")
endif ()


# find libpng before including age_tester
find_package(PNG QUIET)
if (PNG_FOUND)
    message("PNG found => configuring age_tester")
    add_subdirectory(age_tester)
else ()
    message("PNG not found => age_tester not available")
endif ()


# find emscripten before including age_wasm
if (EMSCRIPTEN)
    message("EMSCRIPTEN=${EMSCRIPTEN} => configuring age_wasm")
    add_subdirectory(age_wasm)
else ()
    message("EMSCRIPTEN not set => age_wasm not available")
endif ()
