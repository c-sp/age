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
    echo "    $0 $CMD_ASSEMBLE_PWA"
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
    if ! [ -n "$1" ]; then
        echo "out-dir not specified"
        exit 1
    fi
    echo "$BUILD_DIR/artifacts/$1"
}

age_js_src_dir()
{
    AGE_JS_DIR=`cd "$BUILD_DIR/../src/age_js" && pwd -P`
    echo "$AGE_JS_DIR"
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
    CMD=$1
    shift
    PARAMS="$@"

    case ${CMD} in
        ${JS_BUILD}) PARAMS="--prod --output-path $(out_dir js) $PARAMS" ;;
        ${JS_TEST}) PARAMS="--watch=false $PARAMS" ;;
        ${JS_LINT}) ;;
        *) print_usage_and_exit ;;
    esac

    AGE_JS_DIR=$(age_js_src_dir)
    cd "$AGE_JS_DIR"
    echo "running AGE-JS task in \"`pwd -P`\": $CMD $PARAMS"

    # make sure node_modules exists and is up to date
    npm ci

    # run the requested NG command
    npm run ${CMD} -- ${PARAMS}
}

assemble_pwa()
{
    JS_DIR=$(out_dir js)
    WASM_DIR=$(out_dir wasm)

    switch_to_out_dir pwa
    cp -r ${JS_DIR}/* .
    cp ${WASM_DIR}/age_wasm.js assets/
    cp ${WASM_DIR}/age_wasm.wasm assets/
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
        ${TESTS_BLARGG}) ;;
        ${TESTS_GAMBATTE}) ;;
        ${TESTS_MOONEYE_GB}) ;;
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
    ${TEST_EXEC} --ignore-list "$BUILD_DIR/tests_to_ignore.txt" $1 $SUITE_DIR
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

TESTS_BLARGG=blargg
TESTS_GAMBATTE=gambatte
TESTS_MOONEYE_GB=mooneye-gb

CMD_QT=qt
CMD_WASM=wasm
CMD_JS=js
CMD_ASSEMBLE_PWA=assemble-pwa
CMD_DOXYGEN=doxygen
CMD_TEST=test

# get the AGE build directory based on the path of this script
# (used to determine the target directories of builds)
BUILD_DIR=`dirname $0`
BUILD_DIR=`cd "$BUILD_DIR" && pwd -P`

# get the number of CPU cores
# (used for e.g. make)
NUM_CORES=`grep -c '^processor' /proc/cpuinfo`
if [ ${NUM_CORES} -lt 4 ]; then
    NUM_CORES=4
fi

# check the command in the first parameter
CMD=$1
if [ -n "$CMD" ]; then
    shift
fi

case ${CMD} in
    ${CMD_QT}) build_age_qt $@ ;;
    ${CMD_WASM}) build_age_wasm $@ ;;
    ${CMD_JS}) age_js $@ ;;
    ${CMD_ASSEMBLE_PWA}) assemble_pwa $@ ;;
    ${CMD_DOXYGEN}) run_doxygen ;;
    ${CMD_TEST}) run_tests $@ $@ ;;

    *) print_usage_and_exit ;;
esac
