project(AGE)

cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



add_subdirectory(age_common)
add_subdirectory(age_emulator_gb)

# libpng is required for age_tester
find_package(PNG)
if (PNG_FOUND)
    message("PNG found => configuring age_tester")
    add_subdirectory(age_tester)
else ()
    message("PNG not found => age_tester not available")
endif ()

# emscripten is required for age_wasm
if (EMSCRIPTEN)
    message("EMSCRIPTEN=${EMSCRIPTEN} => configuring age_wasm")
    add_subdirectory(age_wasm)
else ()
    message("EMSCRIPTEN not set => age_wasm not available")
endif ()
