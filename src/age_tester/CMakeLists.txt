add_executable(
        age_tester
        modules/age_tester_module.cpp
        modules/age_tester_module_same_suite.cpp
        modules/age_tester_test.cpp
        modules/age_tester_write_log.cpp
        age_tester_arguments.cpp
        age_tester_arguments.hpp
        age_tester_main.cpp
        age_tester_run_tests.cpp
        age_tester_run_tests.hpp
        age_tester_tasks.cpp
        age_tester_tasks.hpp
        age_tester_tasks_acid2.cpp
        age_tester_tasks_age.cpp
        age_tester_tasks_blargg.cpp
        age_tester_tasks_gambatte.cpp
        age_tester_tasks_little_things.cpp
        age_tester_tasks_mealybug.cpp
        age_tester_tasks_mooneye.cpp
        age_tester_tasks_mooneye_wilbertpol.cpp
        age_tester_tasks_rtc3test.cpp
        age_tester_tasks_same_suite.cpp
        age_tester_thread_pool.hpp)

# Find pthreads on e.g. Ubuntu,
# based on:
#   https://stackoverflow.com/questions/1620918/cmake-and-libpthread
#   https://cmake.org/cmake/help/v3.1/module/FindThreads.html
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# we require libpng for screenshot based testing
find_package(PNG REQUIRED)

target_link_libraries(age_tester PRIVATE Threads::Threads PNG::PNG age_emulator_gb age_common)

# uncomment this to generate assembler listings with g++
#set_target_properties(age_tester PROPERTIES COMPILE_FLAGS "-save-temps -fverbose-asm -Wa,-adhlmn=main.lst")
