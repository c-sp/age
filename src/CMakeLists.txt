project(AGE)

cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



add_subdirectory(age_common)
add_subdirectory(age_emulator_gb)
add_subdirectory(age_tester)

if (EMSCRIPTEN)
    message("EMSCRIPTEN=${EMSCRIPTEN} => configuring age_wasm")
    add_subdirectory(age_wasm)

else ()
    message("EMSCRIPTEN not set => age_wasm not available")
endif ()
