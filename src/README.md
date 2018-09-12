
# AGE Source Code

## Structure

AGE source code is structured into _groups_ of coherent code.
While code within a _group_ may be tightly coupled,
code of different _groups_ is only loosely coupled (if at all).
Loose coupling is achieved by including shared C++ header files found at
`include`.

The following _groups_ exist:

* **`age_common`** contains basic code used throughout the AGE project,
    like for example AGE-specific data types.
    This _group_ does not require any library beside the
    [C++14 STL](https://en.cppreference.com/w/cpp).
* **[`age_emulator_gb`](/age_emulator_gb)** contains the actual gameboy
    emulation code.
    This _group_ does not require any library beside the
    [C++14 STL](https://en.cppreference.com/w/cpp).
* **`age_js`** contains the
    [AGE browser application](https://csprenger.gitlab.io/AGE/) written in
    [TypeScript](https://www.typescriptlang.org/index.html) and using
    [Angular](https://angular.io).
    This _group_ requires the `age_wasm` binaries to run the emulator within
    the browser.
* **`age_qt_emu_test`** contains an emulator test runner created with
    [Qt](https://www.qt.io/).
* **`age_qt_gui`** contains the AGE desktop application created with
    [Qt](https://www.qt.io/).
* **`age_wasm`** contains a [WebAssembly](https://webassembly.org/) interface
    to `age_emulator_gb`.
    It depends on [emscripten](https://kripken.github.io/emscripten-site/) for
    making specific C++ methods callable from JavaScript.


## C++ Code Quality

Do not violate the **single responsibility principle**.
    Unrelated functionality must not be grouped together
    (i.e. in the same file).

### Namespaces

1. **Don't put `using namespace` statements in header files.**
    All consumers of this header file would be forced to live with that
    namespace usage as it cannot be undone.
1. **Avoid `using namespace`** statements in cpp files.

### Includes

1. **Use include guards** in every header file.
    Use the file's name converted to "screaming snake case" for it's include
    guard,
    e.g. use the include guard `AGE_FOO_HPP` for a file named `age_foo.hpp`.
1. **Do not include more than a file needs.**
    Code required just for cpp files must not bloat header files.
1. **Include everything a file needs.**
    Don't rely on transitive includes or the include order in cpp files.

### Data Types

1. **Assume `int` to be at least 32 bits wide**.
    While the C++ standard requires an `int` to be
    [at least 16 bits wide](https://en.cppreference.com/w/cpp/language/types#Properties),
    for current data models `int` width usually is 32 bits.
    AGE static-asserts `int` to be at least 32 bits wide.
1. **Use `int` for arithmetic** until you have a specific reason not to do so.
    Arithmetic operators will cause smaller integral values
    [to be promoted to `int`](https://en.cppreference.com/w/cpp/language/implicit_conversion#Integral_promotion)
    anyway,
    so the result of most operations is an `int`.
1. **Allocate fixed width integers**.
    Don't allocate more memory than necessary.
    Allocating non-fixed width integers like `int` or `std::int_fast##_t` can
    cause more memory to be reserved than actually required.
    This might increase cache misses and thus can decrease performance.
1. **Minimize the size of class data members** while avoiding casts.
    E.g. if `int8_t` is sufficient and does not require additional casts,
    use it in favor of `int`.

**signed vs. unsigned**

1. **Avoid `unsigned`** for [ensuring that a value is non-negative](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Res-nonnegative).
1. **Use `signed`** until there is a very specific reason to not do so.
    Most arithmetic is [assumed to be `signed`](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es102-use-signed-types-for-arithmetic).
    `signed` integer overflow being undefined
    [enables several compiler optimizations](http://blog.llvm.org/2011/05/what-every-c-programmer-should-know.html)
    (`unsigned` overflow being well defined may prevent compiler optimizations
    though).

AGE code uses `unsigned` only for values representing the emulated hardware
(e.g. Gameboy CPU registers, Gameboy memory)
and when interacting with STL containers
(e.g. `size_t std::vector::size()` or `std::vector::operator[size_t]`).
