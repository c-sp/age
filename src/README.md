
# Code Quality

AGE code has to stick to the following rules.

## C++

1. Do not incorporate *using namespace* statements in header files as
    this forces all consumers of this header file to use that namespace.
1. Avoid *using namespace* statements in *cpp* files.

### includes

1. Add include guards to every header file.
    Use the file's name converted to "screaming snake case" for it's
    include guard,
    e.g. the file *age_foo.hpp* would have the include guard
    *AGE_FOO_HPP*.



