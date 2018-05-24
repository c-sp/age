#!/bin/sh -e



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
    echo "    $0 $CMD_WASM $WASM_BUILD_TYPE_DEBUG"
    echo "    $0 $CMD_WASM $WASM_BUILD_TYPE_RELEASE"
    echo "    $0 $CMD_JS $JS_BUILD"
    echo "    $0 $CMD_JS $JS_TEST"
    echo "    $0 $CMD_JS $JS_LINT"
    echo "    $0 $CMD_PAGES <gitlab-pages-subdir>"
    echo "  tests:"
    echo "    $0 $CMD_TEST $TESTS_GAMBATTE <path-to-gambatte-tests>"
    echo "    $0 $CMD_TEST $TESTS_MOONEYE <path-to-mooneye-tests>"
    echo "  miscellaneous:"
    echo "    $0 $CMD_DOXYGEN"
    exit 1
}

out_dir()
{
    echo "$BUILD_DIR/$AGE_ARTIFACTS_SUBDIR/$1"
}

switch_to_out_dir()
{
    OUT_DIR=$(out_dir $1)

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
        ${QT_BUILD_TYPE_DEBUG}) ;;
        ${QT_BUILD_TYPE_RELEASE}) ;;
        *) print_usage_and_exit ;;
    esac

    switch_to_out_dir qt
    echo "running AGE qt $1 build in \"`pwd -P`\" using $NUM_CORES cores"

    qmake "CONFIG+=$1" "$BUILD_DIR/qt/age.pro"
    make -j ${NUM_CORES}
}

build_age_wasm()
{
    case $1 in
        ${WASM_BUILD_TYPE_DEBUG}) ;;
        ${WASM_BUILD_TYPE_RELEASE}) ;;
        *) print_usage_and_exit ;;
    esac

    if ! [ -n "$EMSCRIPTEN" ]; then
        echo "EMSCRIPTEN is not set!"
        echo "We require this variable to point to the emscripten repository."
        exit 1
    fi

    TOOLCHAIN_FILE="$EMSCRIPTEN/cmake/Modules/Platform/Emscripten.cmake"
    if ! [ -f "$TOOLCHAIN_FILE" ]; then
        echo "The emscripten toolchain file could not be found:"
        echo "$TOOLCHAIN_FILE"
        exit 1
    fi

    switch_to_out_dir wasm
    echo "running AGE wasm $1 build in \"`pwd -P`\" using $NUM_CORES cores"

    cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$1 -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" "$BUILD_DIR/wasm"
    make -j ${NUM_CORES}
}

age_js()
{
    PARAMS=""
    case $1 in
        ${JS_BUILD}) PARAMS="--prod --output-path $(out_dir js)" ;;
        ${JS_TEST}) PARAMS=--watch=false ;;
        ${JS_LINT}) ;;
        *) print_usage_and_exit ;;
    esac

    AGE_JS_DIR=`cd "$BUILD_DIR/../src/age_js" && pwd -P`
    cd "$AGE_JS_DIR"
    echo "running AGE-JS $1 in \"`pwd -P`\""

    # always run npm install to make sure node_modules exists and is up to date
    npm install

    # run the requested NG command
    npm run $1 -- ${PARAMS}
}

assemble_pages()
{
    # exit if the output path has not been specified
    if ! [ -n "$1" ]; then
        print_usage_and_exit
    fi

    PAGES_DIR=`cd "$BUILD_DIR/.." && pwd -P`
    PAGES_DIR="$PAGES_DIR/$1"
    ASSETS_DIR="$PAGES_DIR/assets"

    WASM_DIR="$(out_dir wasm)"
    JS_DIR="$(out_dir js)"

    # remove previous pages
    if [ -e "$PAGES_DIR" ]; then
        rm -rf "$PAGES_DIR"
    fi

    # create the directory and change to it
    mkdir -p "$ASSETS_DIR"
    echo "assembling pages in \"$PAGES_DIR\""

    # copy artifacts
    cp "$WASM_DIR/age_wasm.js" "$ASSETS_DIR"
    cp "$WASM_DIR/age_wasm.wasm" "$ASSETS_DIR"
    cp "$JS_DIR/"* "$PAGES_DIR"
}

run_doxygen()
{
    # make sure that the doxygen out-dir exists and is empty
    switch_to_out_dir doxygen

    # The doxygen configuration contains several paths that doxygen interprets
    # based on the current directory.
    # Thus we have to change into the doxygen configuration directory for
    # doxygen to work properly.
    OUT_DIR=`pwd -P`
    cd "$BUILD_DIR/doxygen"
    echo "running doxygen in \"`pwd -P`\""

    doxygen doxygen_config

    # move the doxygen output
    mv html "$OUT_DIR"
}

run_tests()
{
    case $1 in
        ${TESTS_GAMBATTE}) ;;
        ${TESTS_MOONEYE}) ;;
        *) print_usage_and_exit ;;
    esac

    # exit if the test file path has not been specified
    if ! [ -n "$2" ]; then
        print_usage_and_exit
    fi

    # the executable file must exist
    TEST_EXEC="$(out_dir qt)/age_qt_emu_test/age_qt_emu_test"
    if ! [ -x "$TEST_EXEC" ]; then
        echo "The AGE test executable could not be found at:"
        echo "$TEST_EXEC"
        exit 1
    fi

    # run the tests
    ${TEST_EXEC} --type $1 --ignore-list "$BUILD_DIR/tests_to_ignore.txt" $2
}



###############################################################################
##
##   script starting point
##
###############################################################################

QT_BUILD_TYPE_DEBUG=debug
QT_BUILD_TYPE_RELEASE=release

WASM_BUILD_TYPE_DEBUG=Debug
WASM_BUILD_TYPE_RELEASE=Release

JS_BUILD=build
JS_TEST=test
JS_LINT=lint

TESTS_GAMBATTE=gambatte
TESTS_MOONEYE=mooneye

CMD_QT=qt
CMD_WASM=wasm
CMD_JS=js
CMD_PAGES=pages
CMD_DOXYGEN=doxygen
CMD_TEST=test

# get the AGE build directory based on the path of this script
# (used to determine the target directories of builds)
BUILD_DIR=`dirname $0`
BUILD_DIR=`cd "$BUILD_DIR" && pwd -P`

# set the artifacts subdirectory,
# if it has not been set yet (e.g. as environment variable)
if ! [ -n "$AGE_ARTIFACTS_SUBDIR" ]; then
    AGE_ARTIFACTS_SUBDIR=artifacts
fi

# get the number of CPU cores
# (used for e.g. make)
NUM_CORES=`grep -c '^processor' /proc/cpuinfo`
if [ ${NUM_CORES} -lt 4 ]; then
    NUM_CORES=4
fi

# check the command in the first parameter
case $1 in
    ${CMD_QT})      build_age_qt $2 ;;
    ${CMD_WASM})    build_age_wasm $2 ;;
    ${CMD_JS})      age_js $2 ;;
    ${CMD_PAGES})   assemble_pages $2 ;;
    ${CMD_DOXYGEN}) run_doxygen ;;
    ${CMD_TEST})    run_tests $2 $3 ;;

    *) print_usage_and_exit ;;
esac
