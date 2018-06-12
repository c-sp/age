[![build status](/../badges/master/build.svg)](https://gitlab.com/csprenger/age/commits/master)

**[You can now run AGE in your browser!](https://csprenger.gitlab.io/AGE/)**
*(it is still an early prototype though)*

# AGE - Another Gameboy Emulator

AGE is a [Gameboy](https://en.wikipedia.org/wiki/Game_Boy)
[emulator](https://en.wikipedia.org/wiki/Emulator)
aiming for accuracy over performance.

## Features

- run **Gameboy** and **Gameboy Color** roms
- improve **visual quality** with configurable image filters
    (custom [scale2x](https://www.scale2x.it/),
    [gaussian blur](https://en.wikipedia.org/wiki/Gaussian_blur)
    and [embossing](https://en.wikipedia.org/wiki/Image_embossing))
- improve **audio quality** with a
    [low pass filter](https://en.wikipedia.org/wiki/Low-pass_filter)
- **reduce flicker** for some Gameboy roms
- automatically store and load **savegames**
    (if supported by the respective Gameboy rom)
- configure **buttons** and **hotkeys** as you like
- compile for **Windows** and **Linux**
    (and maybe macOS? I did not try it yet ...)

The AGE desktop GUI stores all settings and savegames in a subdirectory called
`.age_emulator` in the user's home directory.

## Project structure

The AGE project is structured as follows:

* [src](/src) contains the actual AGE source code.
* [build](/build) is used to separate build related files from the source code.
