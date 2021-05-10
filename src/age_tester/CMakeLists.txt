add_executable(
        age_tester
        age_tester_arguments.cpp
        age_tester_arguments.hpp
        age_tester_main.cpp
        age_tester_tasks.cpp
        age_tester_tasks.hpp
        age_tester_tasks_gambatte.cpp
        age_tester_tasks_mooneye_gb.cpp
        age_tester_thread_pool.hpp
)

target_link_libraries(age_tester age_emulator_gb age_common)
