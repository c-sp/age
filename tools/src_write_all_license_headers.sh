#!/bin/sh

SOURCE_DIR=`dirname $0`

# Write license header to all .cpp and .hpp source files.
# (for each file execute the write-header-script)
find $SOURCE_DIR/../src -name "*.?pp" -type f -exec $SOURCE_DIR/src_write_license_header.sh {} \;
