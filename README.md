![](https://github.com/c-sp/AGE/workflows/AGE%20CI/badge.svg)

# AGE - Another Game Boy Emulator

AGE is a Game Boy Emulator for Linux/Windows/macOS
[and your Browser](https://c-sp.github.io/age-online)
(see also [age-online](https://github.com/c-sp/age-online)).

AGE requires a
[C++20](https://en.cppreference.com/w/cpp/20)
compatible compiler and these libraries/frameworks:
* [Qt 5](https://www.qt.io/)
  for the desktop application
* [libpng](http://www.libpng.org/pub/png/libpng.html)
  for automated testing

Note that AGE makes use of
[Type Punning](https://blog.regehr.org/archives/959)
which according to the C++ standard actually results in undefined behavior.
You need a compiler that allows type punning (like GCC or Clang)
to build AGE.

[AGE documentation](docs/README.md)
is still in its early stages and currently consists mostly of test rom analyses.


## AGE Desktop Application Features

- run **Game Boy** and **Game Boy Color** roms
- improve **visual quality** with configurable image filters
    (custom [scale2x](https://www.scale2x.it/),
    [gaussian blur](https://en.wikipedia.org/wiki/Gaussian_blur)
    and [embossing](https://en.wikipedia.org/wiki/Image_embossing))
- improve **audio quality** with a
    [low pass filter](https://en.wikipedia.org/wiki/Low-pass_filter)
- **reduce flicker** for some Game Boy roms
- automatically store and load **save games**
    (if supported by the respective Game Boy rom)
- configure **buttons** and **hotkeys** as you like

The AGE Desktop Application stores all settings and save games in a subdirectory
called `.age_emulator` in the user's home directory.
