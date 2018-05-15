#!/bin/sh -e

BUILD_TYPE_DEBUG=debug
BUILD_TYPE_RELEASE=release

CMD_QT=qt
CMD_WASM=wasm
CMD_DOXYGEN=doxygen

BUILD_OUT=build_out


##
#####   utility methods
##

print_usage_and_exit()
{
    echo "usage:"
    echo "  $0 $CMD_QT <build_type>"
    echo "  $0 $CMD_WASM <build_type>"
    echo "  $0 $CMD_DOXYGEN"
    echo "with:"
    echo "  <build_type> = $BUILD_TYPE_DEBUG | $BUILD_TYPE_RELEASE"
    exit 1
}

is_valid_build_type()
{
    case $1 in
        $BUILD_TYPE_DEBUG) ;;
        $BUILD_TYPE_RELEASE) ;;
        *) print_usage_and_exit ;;
    esac
}

switch_to_out_dir()
{
    OUT_DIR=$SCRIPT_DIR/$1/$BUILD_OUT
    echo "build output dir: $OUT_DIR"

    # create the build directory if it does not exist
    if [ -e $OUT_DIR ]; then
        echo "deleting existing build output dir"
        rm -rf $OUT_DIR
    fi
    mkdir -p $OUT_DIR

    cd $OUT_DIR
}


##
#####   build age-qt
##

build_age_qt()
{
    is_valid_build_type $1
    switch_to_out_dir qt
    qmake "CONFIG+=$1" $SCRIPT_DIR/qt/age.pro
    make -j $NUM_CORES
}


##
#####   check arguments
##

# get the directory of this script
# (used to determine the target directories of builds)
SCRIPT_DIR=`dirname $0`
SCRIPT_DIR=`cd $SCRIPT_DIR && pwd -P`
echo "script dir: $SCRIPT_DIR"

# get the number of CPU cores
# (used for e.g. make)
NUM_CORES=`grep -c '^processor' /proc/cpuinfo`
echo "number of cpu cores: $NUM_CORES"

# check the command in the first parameter
case $1 in
    $CMD_QT) build_age_qt $2 ;;
    $CMD_WASM) ;;
    $CMD_DOXYGEN) ;;
    *) print_usage_and_exit ;;
esac

exit 0




#
# The emscripten environment has to be set up first.
# (emsdk/emsdk_env.sh)
#

# set the build type (debug or release)
BUILD_TYPE=Release
if [ "$1" = "debug" ]; then
    BUILD_TYPE=Debug
fi

# store the current directory
PWD=`pwd -P`

# get the absolute directory of the CMakeLists.txt file
# (used as cmake parameter after changing to the build directory)
SCRIPT_DIR=`dirname $0`
SCRIPT_DIR=`cd $SCRIPT_DIR && pwd -P`
BUILD_DIR="$SCRIPT_DIR/../artifacts/wasm"

echo "current directory: $PWD"
echo "script directory:  $SCRIPT_DIR"
echo "build directory:   $BUILD_DIR"

# create the build directory if it does not exist
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p "$BUILD_DIR"
fi

# switch to the build directory and run cmake
cd $BUILD_DIR
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_TOOLCHAIN_FILE=$EMSCRIPTEN/cmake/Modules/Platform/Emscripten.cmake $SCRIPT_DIR
make

# restore the pwd
cd $PWD
