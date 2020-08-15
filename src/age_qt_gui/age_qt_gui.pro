
QT += core gui widgets opengl multimedia

TARGET = age
TEMPLATE = app

# The dependency order is important for the linker:
# in order to resolve all age_common symbols (including
# the ones used in age_emulator_gb),
# the age_common library must be linked last and thus
# be the last dependency.
DEPENDENCIES = age_emulator_gb age_common
include($$PWD/../age.pri)



# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
#DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    age_ui_qt.cpp \
    age_ui_qt_audio.cpp \
    age_ui_qt_emulation_runner.cpp \
    age_ui_qt_emulator.cpp \
    age_ui_qt_main.cpp \
    age_ui_qt_main_window.cpp \
    age_ui_qt_settings_audio.cpp \
    age_ui_qt_settings_dialog.cpp \
    age_ui_qt_settings_keys.cpp \
    age_ui_qt_settings_miscellaneous.cpp \
    age_ui_qt_settings_video.cpp \
    age_ui_qt_user_value_store.cpp \
    age_ui_qt_video.cpp \
    age_ui_qt_video_post_processor.cpp \
    age_ui_qt_video_renderer.cpp

HEADERS  += \
    age_ui_qt.hpp \
    age_ui_qt_audio.hpp \
    age_ui_qt_emulation_runner.hpp \
    age_ui_qt_emulator.hpp \
    age_ui_qt_main_window.hpp \
    age_ui_qt_settings.hpp \
    age_ui_qt_user_value_store.hpp \
    age_ui_qt_video.hpp

RESOURCES += \
    shaders/age_qt_gui_shaders.qrc
