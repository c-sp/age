#
# Qt & cmake:
# https://doc.qt.io/qt-5/cmake-manual.html
#

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Gui Widgets Multimedia REQUIRED)

add_executable(
        age_qt_gui
        age_ui_qt.cpp
        age_ui_qt.hpp
        age_ui_qt_audio.cpp
        age_ui_qt_audio.hpp
        age_ui_qt_emulation_runner.cpp
        age_ui_qt_emulation_runner.hpp
        age_ui_qt_emulator.cpp
        age_ui_qt_emulator.hpp
        age_ui_qt_main.cpp
        age_ui_qt_main_window.cpp
        age_ui_qt_main_window.hpp
        age_ui_qt_settings.hpp
        age_ui_qt_settings_audio.cpp
        age_ui_qt_settings_dialog.cpp
        age_ui_qt_settings_keys.cpp
        age_ui_qt_settings_miscellaneous.cpp
        age_ui_qt_settings_video.cpp
        age_ui_qt_user_value_store.cpp
        age_ui_qt_user_value_store.hpp
        age_ui_qt_video.cpp
        age_ui_qt_video.hpp
        age_ui_qt_video_post_processor.cpp
        age_ui_qt_video_renderer.cpp
        shaders/age_qt_gui_shaders.qrc
)

set_target_properties(age_qt_gui PROPERTIES CXX_CLANG_TIDY "")

target_link_libraries(age_qt_gui PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Multimedia age_emulator_gb age_common)
