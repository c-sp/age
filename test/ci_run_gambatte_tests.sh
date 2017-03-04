#!/bin/sh

# This script expects:
#   - the environment variable GB_REPO_GAMBATTE to contain the path to the gambatte repository.
#   - the age_emulator_test executable to be available in the same directory as this script.

SOURCE_DIR=`dirname $0`
$SOURCE_DIR/age_emulator_test --type gambatte --ignore-list $SOURCE_DIR/tests_to_ignore.txt $GB_REPO_GAMBATTE/test/hwtests

