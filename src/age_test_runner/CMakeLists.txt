add_executable(
        age_test_runner
        modules/age_tr_module.cpp
        modules/age_tr_module_acid2.cpp
        modules/age_tr_module_blargg.cpp
        modules/age_tr_module_little_things.cpp
        modules/age_tr_module_mealybug.cpp
        modules/age_tr_module_rtc3test.cpp
        modules/age_tr_module_same_suite.cpp
        modules/age_tr_test.cpp
        modules/age_tr_write_log.cpp
        age_tr_arguments.cpp
        age_tr_main.cpp
        age_tr_run_tests.cpp
        age_tr_tasks.cpp
        age_tr_tasks_age.cpp
        age_tr_tasks_gambatte.cpp
        age_tr_tasks_mooneye.cpp
        age_tr_tasks_mooneye_wilbertpol.cpp
        age_tr_thread_pool.hpp)

# Find pthreads on e.g. Ubuntu,
# based on:
#   https://stackoverflow.com/questions/1620918/cmake-and-libpthread
#   https://cmake.org/cmake/help/v3.1/module/FindThreads.html
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# we require libpng for screenshot based testing
find_package(PNG REQUIRED)

target_link_libraries(age_test_runner PRIVATE Threads::Threads PNG::PNG age_emulator_gb age_common)

# uncomment this to generate assembler listings with g++
#set_target_properties(age_test_runner PROPERTIES COMPILE_FLAGS "-save-temps -fverbose-asm -Wa,-adhlmn=main.lst")
