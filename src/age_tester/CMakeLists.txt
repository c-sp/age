add_executable(
        age_tester
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
        age_tester_tasks_mooneye_gb.cpp
        age_tester_tasks_same_suite.cpp
        age_tester_thread_pool.hpp
        age_tester_write_log.cpp
)

# Find pthreads on e.g. Ubuntu,
# based on:
#   https://stackoverflow.com/questions/1620918/cmake-and-libpthread
#   https://cmake.org/cmake/help/v3.1/module/FindThreads.html
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# we require libpng for screenshot based testing
find_package(PNG REQUIRED)

target_link_libraries(age_tester PRIVATE Threads::Threads PNG::PNG age_emulator_gb age_common)
