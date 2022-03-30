#!/usr/bin/env bash
set -e
#
# portable shebang:
# https://www.cyberciti.biz/tips/finding-bash-perl-python-portably-using-env.html
#
# bash on macos:
# https://itnext.io/upgrading-bash-on-macos-7138bd1066ba
#



###############################################################################
##
##   utility methods
##
###############################################################################

print_usage_and_exit()
{
    echo "usage:"
    echo "  builds:"
    echo "    $0 $CMD_BUILD_GTEST $CMAKE_BUILD_TYPE_DEBUG"
    echo "    $0 $CMD_BUILD_GTEST $CMAKE_BUILD_TYPE_RELEASE"
    echo "    $0 $CMD_BUILD_QT $QT_BUILD_TYPE_DEBUG"
    echo "    $0 $CMD_BUILD_QT $QT_BUILD_TYPE_RELEASE"
    echo "    $0 $CMD_BUILD_TESTER $CMAKE_BUILD_TYPE_DEBUG"
    echo "    $0 $CMD_BUILD_TESTER $CMAKE_BUILD_TYPE_RELEASE"
    echo "    $0 $CMD_BUILD_WASM $CMAKE_BUILD_TYPE_DEBUG"
    echo "    $0 $CMD_BUILD_WASM $CMAKE_BUILD_TYPE_RELEASE"
    echo "  tests:"
    echo "    $0 $CMD_RUN_GTEST"
    echo "    $0 $CMD_RUN_TESTS $TESTS_ACID2"
    echo "    $0 $CMD_RUN_TESTS $TESTS_AGE"
    echo "    $0 $CMD_RUN_TESTS $TESTS_BLARGG"
    echo "    $0 $CMD_RUN_TESTS $TESTS_FIRSTWHITE"
    echo "    $0 $CMD_RUN_TESTS $TESTS_GAMBATTE"
    echo "    $0 $CMD_RUN_TESTS $TESTS_MEALYBUG"
    echo "    $0 $CMD_RUN_TESTS $TESTS_MOONEYE"
    echo "    $0 $CMD_RUN_TESTS $TESTS_RTC3TEST"
    echo "    $0 $CMD_RUN_TESTS $TESTS_SAME_SUITE"
    echo "  miscellaneous:"
    echo "    $0 $CMD_DOXYGEN"
    exit 1
}

artifact_dir()
{
    if [ -z "$1" ]; then
        echo "artifact-dir not specified"
        exit 1
    fi
    echo "$BUILD_DIR/artifacts/$1"
}

cd_new_tmp()
{
    NEW_TMP=$(mktemp -d)
    cd "$NEW_TMP"
}

mkdir_artifact()
{
    ART_DIR=$(artifact_dir "$1")

    # remove previous build artifacts
    if [ -e "$ART_DIR" ]; then
        rm -rf "$ART_DIR"
    fi

    # create the directory
    mkdir -p "$ART_DIR"
    echo "$ART_DIR"
}

switch_to_out_dir()
{
    OUT_DIR=$(artifact_dir "$1")

    # remove previous build artifacts
    if [ -e "$OUT_DIR" ]; then
        rm -rf "$OUT_DIR"
    fi

    # create the directory and change to it
    mkdir -p "$OUT_DIR"
    cd "$OUT_DIR"
}



###############################################################################
##
##   commands
##
###############################################################################

build_gtest()
{
    case $1 in
        "${CMAKE_BUILD_TYPE_DEBUG}") ;;
        "${CMAKE_BUILD_TYPE_RELEASE}") ;;
        *) print_usage_and_exit ;;
    esac

    cd_new_tmp
    echo "running age_gtest $1 build in \"$(pwd -P)\""

    cmake -DCMAKE_BUILD_TYPE="$1" "$REPO_DIR/src"
    make -j -l 5 age_gtest

    ARTIFACT_DIR=$(mkdir_artifact age_gtest)
    cp age_gtest "$ARTIFACT_DIR"
}

build_qt()
{
    case $1 in
        "${QT_BUILD_TYPE_DEBUG}") ;;
        "${QT_BUILD_TYPE_RELEASE}") ;;
        *) print_usage_and_exit ;;
    esac

    switch_to_out_dir age_qt
    echo "running age_qt $1 build in \"$(pwd -P)\""

    qmake "CONFIG+=$1" "$REPO_DIR/src/age_qt_gui.pro"
    make -j -l 5
}

build_tester()
{
    case $1 in
        "${CMAKE_BUILD_TYPE_DEBUG}") ;;
        "${CMAKE_BUILD_TYPE_RELEASE}") ;;
        *) print_usage_and_exit ;;
    esac

    cd_new_tmp
    echo "running age_tester $1 build in \"$(pwd -P)\""

    cmake -DCMAKE_BUILD_TYPE="$1" "$REPO_DIR/src"
    make -j -l 5 age_tester

    ARTIFACT_DIR=$(mkdir_artifact age_tester)
    cp age_tester/age_tester "$ARTIFACT_DIR"
}

build_wasm()
{
    case $1 in
        "${CMAKE_BUILD_TYPE_DEBUG}") ;;
        "${CMAKE_BUILD_TYPE_RELEASE}") ;;
        *) print_usage_and_exit ;;
    esac

    switch_to_out_dir age_wasm
    echo "running age_wasm $1 build in \"$(pwd -P)\""

    emcmake cmake -DCMAKE_BUILD_TYPE="$1" "$REPO_DIR/src"
    make -j -l 5 age_wasm
}

run_doxygen()
{
    # make sure that the doxygen out-dir exists and is empty
    switch_to_out_dir doxygen

    # The doxygen configuration contains several paths that doxygen interprets
    # based on the current directory.
    # Thus we have to change into the doxygen configuration directory for
    # doxygen to work properly.
    OUT_DIR=$(pwd -P)
    cd "$BUILD_DIR/doxygen"
    echo "running doxygen in \"$(pwd -P)\""

    doxygen doxygen_config

    # move the doxygen output
    mv html "$OUT_DIR"
}

run_gtest()
{
    # the executable file must exist
    TEST_EXEC="$(artifact_dir age_gtest)/age_gtest"
    if ! [ -e "$TEST_EXEC" ]; then
        echo "The AGE Google Test executable could not be found at:"
        echo "$TEST_EXEC"
        exit 1
    fi
    # set +x on the executable as GitHub currently breaks file permissions
    # when up/downloading artifacts:
    # https://github.com/actions/upload-artifact/issues/38
    # TODO remove setting +x once GitHub artifact file permissions work
    chmod +x "$TEST_EXEC"

    # run the tests
    ${TEST_EXEC} "$@"
}

run_tests()
{
    case $1 in
        "${TESTS_ACID2}") ;;
        "${TESTS_AGE}") ;;
        "${TESTS_BLARGG}") ;;
        "${TESTS_FIRSTWHITE}") ;;
        "${TESTS_GAMBATTE}") ;;
        "${TESTS_MEALYBUG}") ;;
        "${TESTS_MOONEYE}") ;;
        "${TESTS_RTC3TEST}") ;;
        "${TESTS_SAME_SUITE}") ;;
        *) print_usage_and_exit ;;
    esac

    # the executable file must exist
    TEST_EXEC="$(artifact_dir age_tester)/age_tester"
    if ! [ -e "$TEST_EXEC" ]; then
        echo "The AGE test executable could not be found at:"
        echo "$TEST_EXEC"
        exit 1
    fi
    # set +x on the executable as GitHub currently breaks file permissions
    # when up/downloading artifacts:
    # https://github.com/actions/upload-artifact/issues/38
    # TODO remove setting +x once GitHub artifact file permissions work
    chmod +x "$TEST_EXEC"

    # check test suite path
    SUITE_DIR="$(artifact_dir test-suites)"
    if ! [ -e "$SUITE_DIR" ]; then
        echo "test suite not found at: $SUITE_DIR"
        echo "downloading test suites zip file"
        switch_to_out_dir test-suites
        wget -q https://github.com/c-sp/gameboy-test-roms/releases/download/v4.0/gameboy-test-roms-v4.0.zip
        unzip -q gameboy-test-roms-v4.0.zip
    fi

    # run the tests
    ${TEST_EXEC} --print-failed --print-passed --blacklist "$BUILD_DIR/test-blacklist.txt" "--$1" "$SUITE_DIR"
}



###############################################################################
##
##   script starting point
##
###############################################################################

QT_BUILD_TYPE_DEBUG=debug
QT_BUILD_TYPE_RELEASE=release

CMAKE_BUILD_TYPE_DEBUG=Debug
CMAKE_BUILD_TYPE_RELEASE=Release

TESTS_ACID2=acid2
TESTS_AGE=age
TESTS_BLARGG=blargg
TESTS_FIRSTWHITE=firstwhite
TESTS_GAMBATTE=gambatte
TESTS_MEALYBUG=mealybug
TESTS_MOONEYE=mooneye
TESTS_RTC3TEST=rtc3test
TESTS_SAME_SUITE=same-suite

CMD_BUILD_GTEST=build-gtest
CMD_BUILD_QT=build-qt
CMD_BUILD_TESTER=build-tester
CMD_BUILD_WASM=build-wasm
CMD_DOXYGEN=doxygen
CMD_RUN_GTEST=run-gtest
CMD_RUN_TESTS=run-tests

# get the AGE build directory based on the path of this script
# (used to determine the target directories of builds)
BUILD_DIR=$(dirname "$0")
BUILD_DIR=$(cd "$BUILD_DIR" && pwd -P)
REPO_DIR=$(cd "$BUILD_DIR/.." && pwd -P)

# check the command in the first parameter
CMD=$1
if [ -n "$CMD" ]; then
    shift
fi

case ${CMD} in
    "${CMD_BUILD_GTEST}") build_gtest "$@" ;;
    "${CMD_BUILD_QT}") build_qt "$@" ;;
    "${CMD_BUILD_TESTER}") build_tester "$@" ;;
    "${CMD_BUILD_WASM}") build_wasm "$@" ;;
    "${CMD_DOXYGEN}") run_doxygen ;;
    "${CMD_RUN_GTEST}") run_gtest "$@" ;;
    "${CMD_RUN_TESTS}") run_tests "$@" ;;

    *) print_usage_and_exit ;;
esac
