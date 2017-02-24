#!/bin/sh

# Check if the user passed exactly one parameter.
if [ $# != 1 ]; then
    echo "Usage: $0 <file>"
    exit 1
fi

# Build a new file containing the license header and the contents of
# the specified file beginning with the first empty line.
#
# The sed parameter below consists of three parts:
#   /^\s*$/   match from the first empty line onwards
#   ,$        match until the end of the file
#   p         print the matched lines
FILE=$1
TEMP_FILE=$FILE.tmp
SOURCE_DIR=`dirname $0`

cat $SOURCE_DIR/license_header.txt > $TEMP_FILE
cat $FILE | sed -n '/^\s*$/,$p' >> $TEMP_FILE

# Replace the original file with the newly built one.
echo "writing license header to $FILE"
mv $TEMP_FILE $FILE
