add_library(
        age_common
        STATIC
        age_debug.cpp
        age_downsampler.cpp
        age_pcm_ring_buffer.cpp
        age_screen_buffer.cpp
        age_utilities.cpp
)

target_include_directories(age_common PUBLIC api)
