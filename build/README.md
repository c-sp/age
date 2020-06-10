
# AGE Builds & CI

* **`artifacts`** (ignored by git) is used as default output directory for any
    build output.
* **`doxygen`** contains the configuration file and [Qt](https://www.qt.io/)
    tag files required for building the AGE HTML documentation.
* **`qt`** contains [Qt](https://www.qt.io/) project files for building the
    AGE desktop GUI and the AGE emulator test runner.
    [Qt Creator](http://doc.qt.io/qtcreator/) can open these project files for
    development.
* **`wasm`** contains the [CMake](https://cmake.org/) configuration for
    building the AGE [WebAssembly](https://webassembly.org/) artifacts used
    to run the emulator inside a browser.

## AGE build script

**`age_ci.sh`** is used to execute standard AGE build tasks.

## The AGE CI pipeline

AGE is continuously being built and tested using GitLab's
[CI pipeline](/../../pipelines).
The docker images used for AGE CI can be found in my repositories
[docker-qt-gcc](https://gitlab.com/csprenger/docker-qt-gcc) and
[docker-age-ci](https://gitlab.com/csprenger/docker-age-ci).
