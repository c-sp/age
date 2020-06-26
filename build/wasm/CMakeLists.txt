
##############################################
######                                  ######
###### build the AGE WebAssembly binary ######
######                                  ######
##############################################

#
# The emscripten environment has to be set up first.
# (emsdk/emsdk_env.sh)
#

#
# CMake Links:
#
#   CMake Documentation: https://cmake.org/cmake/help/v3.10/
#   CMake Tutorial:      https://cmake.org/cmake-tutorial/
#
#   Emscripten & CMake: https://stackoverflow.com/a/45277855
#

cmake_minimum_required (VERSION 3.7.2)

project (AGE-WebAssembly)


#####   source files

set (DIR_AGE             "../../src")
set (DIR_AGE_COMMON      ${DIR_AGE}/age_common)
set (DIR_AGE_EMULATOR_GB ${DIR_AGE}/age_emulator_gb)
set (DIR_AGE_WASM        ${DIR_AGE}/age_wasm)

set (SOURCES_AGE_COMMON
    ${DIR_AGE_COMMON}/age_debug.cpp
    ${DIR_AGE_COMMON}/age_downsampler.cpp
    ${DIR_AGE_COMMON}/age_emulator.cpp
    ${DIR_AGE_COMMON}/age_pcm_ring_buffer.cpp
    ${DIR_AGE_COMMON}/age_screen_buffer.cpp
    ${DIR_AGE_COMMON}/age_utilities.cpp
)

set (SOURCES_AGE_EMULATOR_GB
    ${DIR_AGE_EMULATOR_GB}/common/age_gb_clock.cpp
    ${DIR_AGE_EMULATOR_GB}/common/age_gb_device.cpp
    ${DIR_AGE_EMULATOR_GB}/common/age_gb_events.cpp
    ${DIR_AGE_EMULATOR_GB}/common/age_gb_interrupts.cpp
    ${DIR_AGE_EMULATOR_GB}/age_gb_bus.cpp
    ${DIR_AGE_EMULATOR_GB}/age_gb_cpu.cpp
    ${DIR_AGE_EMULATOR_GB}/age_gb_cpu_opcodes.cpp
    ${DIR_AGE_EMULATOR_GB}/age_gb_div.cpp
    ${DIR_AGE_EMULATOR_GB}/age_gb_emulator.cpp
    ${DIR_AGE_EMULATOR_GB}/age_gb_emulator_impl.cpp
    ${DIR_AGE_EMULATOR_GB}/age_gb_joypad.cpp
    ${DIR_AGE_EMULATOR_GB}/age_gb_lcd.cpp
    ${DIR_AGE_EMULATOR_GB}/age_gb_lcd_ports.cpp
    ${DIR_AGE_EMULATOR_GB}/age_gb_lcd_render.cpp
    ${DIR_AGE_EMULATOR_GB}/age_gb_lcd_scanline.cpp
    ${DIR_AGE_EMULATOR_GB}/age_gb_memory.cpp
    ${DIR_AGE_EMULATOR_GB}/age_gb_memory_init.cpp
    ${DIR_AGE_EMULATOR_GB}/age_gb_serial.cpp
    ${DIR_AGE_EMULATOR_GB}/age_gb_sound.cpp
    ${DIR_AGE_EMULATOR_GB}/age_gb_sound_io_ports.cpp
    ${DIR_AGE_EMULATOR_GB}/age_gb_sound_utilities.cpp
    ${DIR_AGE_EMULATOR_GB}/age_gb_timer.cpp
    ${DIR_AGE_EMULATOR_GB}/age_gb_timer_state.cpp
)

set (SOURCES_AGE_WASM
    ${DIR_AGE_WASM}/age_wasm.cpp
)


#####   C++ options

# use C++14
set(CMAKE_CXX_STANDARD 14)

# additional include directories
include_directories (
    ${DIR_AGE}/include
    ${EMSCRIPTEN}/system/include
)


#####   AGE binaries

add_library(age_common STATIC ${SOURCES_AGE_COMMON})

add_library(age_emulator_gb STATIC ${SOURCES_AGE_EMULATOR_GB})
target_link_libraries(age_emulator_gb age_common)

add_executable(age_wasm ${SOURCES_AGE_WASM})
target_link_libraries(age_wasm age_emulator_gb)

# MODULARIZE=1     to prevent using the global scope
# EXPORT_ES6=1     to generate an ES6 module export
# ENVIRONMENT=web  to omit node/shell/web-worker code
#
set(WASM_LINK_FLAGS "\
-s WASM=1 \
-s ALLOW_MEMORY_GROWTH=1 \
-s NO_FILESYSTEM=1 \
-s MODULARIZE=1 \
-s EXPORT_ES6=1 \
-s ENVIRONMENT=web \
")
set_target_properties(age_wasm PROPERTIES LINK_FLAGS ${WASM_LINK_FLAGS})
