
TEMPLATE = subdirs

SUBDIRS += \
    age_common \
    age_emulator_gb \
    age_qt_emu_test \
    age_qt_gui \
    age_wasm



# sub-project dependencies
# (tells QMake about the correct build order)

age_emulator_gb.depends = age_common
age_qt_emu_test.depends = age_emulator_gb
age_qt_gui.depends = age_emulator_gb
age_wasm.depends = age_emulator_gb
