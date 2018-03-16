#!/bin/sh

SOURCE_DIR=`dirname $0`

# Convert Windows line endings (CRLF) to Unix line endings (LF).
# (for each file execute the "dos2unix" command)
find $SOURCE_DIR/.. -regex ".*\.hpp\|.*\.cpp\|.*\.pro\|.*\.pri\|.*\.txt|.*\.yml|.*\.md|.*\.adoc|.*\.sh" -type f -exec dos2unix {} \;
