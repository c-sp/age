project(AGE)

cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 17)



add_subdirectory(age_common)
add_subdirectory(age_emulator_gb)
add_subdirectory(age_test_runner)

if (EMSCRIPTEN)
    message("EMSCRIPTEN=${EMSCRIPTEN} => configuring age_wasm")
    add_subdirectory(age_wasm)

else ()
    message("EMSCRIPTEN not set => age_wasm not available")
endif ()
