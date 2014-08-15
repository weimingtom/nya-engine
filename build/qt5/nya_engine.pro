# WARNING: do not forget to configure qt creator kit (under projects tab) to build in appropiate directory
# this .pro file is tested under clang64 and mingw32 specs

TARGET = nya_engine
TEMPLATE = lib
CONFIG += staticlib
CONFIG -= qt
QT =


CONFIG(debug, debug|release) {
    CONF = debug
} else:CONFIG(release, debug|release) {
    CONF = release
}
PLATFORM = $$first(QMAKE_PLATFORM)
android {
    PLATFORM = $$PLATFORM"_"$$ANDROID_TARGET_ARCH
} else:mingw {
    PLATFORM = mingw
}
BIN_DIR_NAME = "qtc_"$$PLATFORM"_"$$CONF

DESTDIR = $$shell_path($$PWD/../../bin/$$BIN_DIR_NAME)

include( nya_sources.pri )
