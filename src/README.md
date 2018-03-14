
# Code Quality

AGE code has to stick to the following rules.

## C++

1. **Don't put `using namespace` statements in header files.**
    All consumers of this header file would be forced to live with
    that namespace usage as it cannot be undone.
1. **Avoid `using namespace`** statements in cpp files.
1. Do not violate the **single responsibility principle**.
    Unrelated functionality must not be grouped together
    (i.e. in the same file).

### includes

1. **Use include guards** in every header file.
    Use the file's name converted to "screaming snake case" for it's
    include guard,
    e.g. use the include guard `AGE_FOO_HPP` for a file named
    `age_foo.hpp`.
1. **Do not include more than a file needs.**
    Code required just for cpp files must not bloat header files.
1. **Include everything a file needs.**
    Don't rely on transitive includes or the include order in cpp files.


