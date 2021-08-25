# git revision code generation, based on:
# https://www.mattkeeter.com/blog/2018-01-06-versioning/

execute_process(
        COMMAND git log --pretty=format:%h -n 1
        OUTPUT_VARIABLE GIT_REV
        ERROR_QUIET
)

if ("${GIT_REV}" STREQUAL "")
    # no revision information found?
    set(GIT_REV "")
    set(GIT_DATE "")
    set(GIT_TAG "")
    set(GIT_BRANCH "")

else ()
    # get further details on this revision
    execute_process(
            COMMAND git log --pretty=format:%ad --date=iso-strict -n 1
            OUTPUT_VARIABLE GIT_DATE
    )
    execute_process(
            COMMAND git describe --exact-match --tags
            OUTPUT_VARIABLE GIT_TAG ERROR_QUIET
    )
    execute_process(
            COMMAND git rev-parse --abbrev-ref HEAD
            OUTPUT_VARIABLE GIT_BRANCH
    )

    string(STRIP "${GIT_REV}" GIT_REV)
    string(STRIP "${GIT_DATE}" GIT_DATE)
    string(STRIP "${GIT_TAG}" GIT_TAG)
    string(STRIP "${GIT_BRANCH}" GIT_BRANCH)
endif ()

# create C++ code with revision constants
set(VERSION "constexpr const char* GIT_REV=\"${GIT_REV}\";
constexpr const char* GIT_DATE=\"${GIT_DATE}\";
constexpr const char* GIT_TAG=\"${GIT_TAG}\";
constexpr const char* GIT_BRANCH=\"${GIT_BRANCH}\";
")

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/api/git_revision.hpp)
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/api/git_revision.hpp VERSION_OLD)
else ()
    set(VERSION_OLD "")
endif ()

if (NOT "${VERSION}" STREQUAL "${VERSION_OLD}")
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/api/git_revision.hpp "${VERSION}")
endif ()


# age_common library contents

add_library(
        age_common
        STATIC
        api/gfx/age_png.hpp
        api/git_revision.hpp
        age_debug.cpp
        age_downsampler.cpp
        age_pcm_ring_buffer.cpp
        age_screen_buffer.cpp
        age_utilities.cpp
)

target_include_directories(age_common PUBLIC api)
