
# enable c14 features
CONFIG -= c++11
CONFIG *= c++14

# define DEBUG (or maybe don't)
CONFIG(debug, debug|release) {
    DEFINES *= DEBUG
} else {
    DEFINES -= DEBUG
}



# configure GCC optimizations
# (details at https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html)

QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE *= -O3
QMAKE_LFLAGS_RELEASE *= -O3

# enable link time optimization only for applications (not for libraries)
equals(TEMPLATE, app) {
    QMAKE_CXXFLAGS_RELEASE *= -flto
    QMAKE_LFLAGS_RELEASE *= -flto
}



# add dependencies declared in DEPENDENCIES to LIBS, INCLUDEPATH and DEPENDPATH

win32:CONFIG(release, debug|release):    LIB_PATH_SUFFIX = release/
else:win32:CONFIG(debug, debug|release): LIB_PATH_SUFFIX = debug/

for(_DEP, DEPENDENCIES) {
    INCLUDEPATH += $$PWD/$$_DEP
    DEPENDPATH += $$PWD/$$_DEP
    LIBS += -L$$OUT_PWD/../$$_DEP/$$LIB_PATH_SUFFIX -l$$_DEP
}
