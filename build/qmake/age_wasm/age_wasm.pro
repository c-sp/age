
#
# This QMake Project file is not used for building AGE-Wasm.
# It's only purpose is to make the AGE-Wasm source code available
# in Qt Creator.
#
# Note that you have set the EMSCRIPTEN environment variable
# to point to an emscripten release,
# as $EMSCRIPTEN/system/include is added to INCLUDEPATH to allow
# for emscripten header file evaluation in Qt Creator.
#

QT -= core gui

TARGET = age_wasm
TEMPLATE = lib
CONFIG += staticlib

DEPENDENCIES = age_common age_emulator_gb
include($$PWD/../age.pri)


# if emscripten is installed and the environment
# has been set up accordingly,
# add the emscripten include path and define EMSCRIPTEN
_EMSCRIPTEN = $$(EMSCRIPTEN)
!isEmpty(_EMSCRIPTEN) {
    INCLUDEPATH += $$(EMSCRIPTEN)/system/include
    DEFINES += EMSCRIPTEN
}


SOURCES += \
    ../../../src/age_wasm/age_wasm.cpp

HEADERS += \


unix {
    target.path = /usr/lib
    INSTALLS += target
}
