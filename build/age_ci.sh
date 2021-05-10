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
    echo "usages:"
    echo "  builds:"
    echo "    $0 $CMD_QT $QT_BUILD_TYPE_DEBUG"
    echo "    $0 $CMD_QT $QT_BUILD_TYPE_RELEASE"
    echo "    $0 $CMD_TESTER $CMAKE_BUILD_TYPE_DEBUG"
    echo "    $0 $CMD_TESTER $CMAKE_BUILD_TYPE_RELEASE"
    echo "    $0 $CMD_WASM $CMAKE_BUILD_TYPE_DEBUG"
    echo "    $0 $CMD_WASM $CMAKE_BUILD_TYPE_RELEASE"
    echo "  tests:"
    echo "    $0 $CMD_TEST $TESTS_BLARGG"
    echo "    $0 $CMD_TEST $TESTS_GAMBATTE"
    echo "    $0 $CMD_TEST $TESTS_MOONEYE_GB"
    echo "  miscellaneous:"
    echo "    $0 $CMD_DOXYGEN"
    exit 1
}

out_dir()
{
    if [ -z "$1" ]; then
        echo "out-dir not specified"
        exit 1
    fi
    echo "$BUILD_DIR/artifacts/$1"
}

switch_to_out_dir()
{
    OUT_DIR=$(out_dir "$1")

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

build_age_qt()
{
    case $1 in
        "${QT_BUILD_TYPE_DEBUG}") ;;
        "${QT_BUILD_TYPE_RELEASE}") ;;
        *) print_usage_and_exit ;;
    esac

    switch_to_out_dir qt
    echo "running AGE qt $1 build in \"$(pwd -P)\""

    qmake "CONFIG+=$1" "$REPO_DIR/src/age.pro"
    make -j -l 5
}

build_age_tester()
{
    case $1 in
        "${CMAKE_BUILD_TYPE_DEBUG}") ;;
        "${CMAKE_BUILD_TYPE_RELEASE}") ;;
        *) print_usage_and_exit ;;
    esac

    switch_to_out_dir age_tester
    echo "running AGE age_tester $1 build in \"$(pwd -P)\""

    cmake -DCMAKE_BUILD_TYPE="$1" "$REPO_DIR/src"
    make -j -l 5 age_tester
}

build_age_wasm()
{
    case $1 in
        "${CMAKE_BUILD_TYPE_DEBUG}") ;;
        "${CMAKE_BUILD_TYPE_RELEASE}") ;;
        *) print_usage_and_exit ;;
    esac

    switch_to_out_dir wasm
    echo "running AGE wasm $1 build in \"$(pwd -P)\""

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

run_tests()
{
    case $1 in
        "${TESTS_BLARGG}") ;;
        "${TESTS_GAMBATTE}") ;;
        "${TESTS_MOONEYE_GB}") ;;
        *) print_usage_and_exit ;;
    esac

    # the executable file must exist
    TEST_EXEC="$(out_dir qt)/age_qt_emu_test/age_qt_emu_test"
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
    SUITE_DIR="$(out_dir test-suites)/$1"
    if ! [ -e "$SUITE_DIR" ]; then
        echo "test suite not found at: $SUITE_DIR"
        echo "downloading test suites zip file"
        switch_to_out_dir test-suites
        wget -q https://github.com/c-sp/gameboy-test-roms/releases/download/v1.1/gameboy-test-roms-v1.1.zip
        unzip -q gameboy-test-roms-v1.1.zip
    fi

    # run the tests
    ${TEST_EXEC} --ignore-list "$BUILD_DIR/tests_to_ignore.txt" "$1" "$SUITE_DIR"
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

TESTS_BLARGG=blargg
TESTS_GAMBATTE=gambatte
TESTS_MOONEYE_GB=mooneye-gb

CMD_QT=qt
CMD_TESTER=tester
CMD_WASM=wasm
CMD_DOXYGEN=doxygen
CMD_TEST=test

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
    "${CMD_QT}") build_age_qt "$@" ;;
    "${CMD_TESTER}") build_age_tester "$@" ;;
    "${CMD_WASM}") build_age_wasm "$@" ;;
    "${CMD_DOXYGEN}") run_doxygen ;;
    "${CMD_TEST}") run_tests "$@" ;;

    *) print_usage_and_exit ;;
esac
