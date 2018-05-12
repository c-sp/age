#!/bin/sh

# This script expects:
#   - the environment variable GB_REPO_MOONEYE_GB to contain the path to the mooneye-gb repository.
#   - the age_qt_emu_test executable to be available in the same directory as this script.

SOURCE_DIR=`dirname $0`
$SOURCE_DIR/../build/artifacts/test/age_qt_emu_test --type mooneye --ignore-list $SOURCE_DIR/tests_to_ignore.txt $GB_REPO_MOONEYE_GB/tests/build

