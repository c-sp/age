
# Code Quality

AGE code has to stick to the following rules.

## C++

1. **Do not incorporate *using namespace* statements** in header files.
    All consumers of this header file would be forced to live with
    that namespace usage as this cannot be undone.
1. **Avoid *using namespace* statements** in *cpp* files.
1. Do not violate the **single responsibility principle**.
    Unrelated functionality must not be grouped together.
    Consider creating new *hpp*/*cpp* files for new classes.
    Related code may be kept together in a single header file though.

### includes

1. **Use include guards** in every header file.
    Use the file's name converted to "screaming snake case" for it's
    include guard,
    e.g. the file *age_foo.hpp* would have the include guard
    *AGE_FOO_HPP*.


