#!/bin/sh -e

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
