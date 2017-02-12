
QT += core gui widgets opengl multimedia

TARGET = age
TEMPLATE = app

DEPENDENCIES = age_common age_simulator_gb age_ui_common
include($$PWD/../age.pri)



# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    age_ui_qt.cpp \
    age_ui_qt_audio.cpp \
    age_ui_qt_gl_frame_manager.cpp \
    age_ui_qt_gl_renderer.cpp \
    age_ui_qt_gl_shader.cpp \
    age_ui_qt_gl_texture.cpp \
    age_ui_qt_main.cpp \
    age_ui_qt_main_window.cpp \
    age_ui_qt_settings_audio.cpp \
    age_ui_qt_settings_dialog.cpp \
    age_ui_qt_settings_keys.cpp \
    age_ui_qt_settings_miscellaneous.cpp \
    age_ui_qt_settings_video.cpp \
    age_ui_qt_simulation_runner.cpp \
    age_ui_qt_simulator.cpp \
    age_ui_qt_user_value_store.cpp

HEADERS  += \
    age_ui_qt.hpp \
    age_ui_qt_audio.hpp \
    age_ui_qt_gl_renderer.hpp \
    age_ui_qt_main_window.hpp \
    age_ui_qt_settings.hpp \
    age_ui_qt_simulation_runner.hpp \
    age_ui_qt_simulator.hpp \
    age_ui_qt_user_value_store.hpp
