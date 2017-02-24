#!/bin/sh

SOURCE_DIR=`dirname $0`

# Write license header to all .cpp and .hpp source files.
# (for each file execute the write-header-script)
find $SOURCE_DIR/.. -regex ".*\.hpp\|.*\.cpp\|.*\.pro\|.*\.txt" -type f -exec dos2unix {} \;
