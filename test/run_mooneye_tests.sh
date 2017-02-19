#!/bin/sh

# This script expects:
#   - the environment variable GB_REPO_MOONEYE_GB to contain the path to the mooneye-gb repository.
#   - the age_emulator_test executable to be available.

SOURCE_DIR=`dirname $0`
$SOURCE_DIR/age_emulator_test --type mooneye --ignore-list $SOURCE_DIR/tests_to_ignore.txt $GB_REPO_MOONEYE_GB/tests/build

