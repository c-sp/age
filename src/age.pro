
TEMPLATE = subdirs

SUBDIRS += \
    age_common \
    age_simulator_gb \
    age_emulator_test \
    age_ui_common \
    age_ui_qt



# sub-project dependencies
# (tells QMake about the correct build order)

age_simulator_gb.depends = age_common
age_emulator_test.depends = age_simulator_gb

age_ui_common.depends = age_common
age_ui_qt.depends = age_ui_common age_simulator_gb
