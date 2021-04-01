add_executable(
        age_test_runner
        main.cpp
)

target_link_libraries(age_test_runner age_emulator_gb age_common)
