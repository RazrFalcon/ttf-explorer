QT      += core gui widgets

TARGET   = ttf-explorer
TEMPLATE = app

CONFIG  += c++17

equals(QMAKE_CXX, clang++) {
    QMAKE_CXXFLAGS += -Wextra -Wpedantic -Wimplicit-fallthrough -Wconversion
}

SOURCES += \
    src/tables/avar.cpp \
    src/tables/cff.cpp \
    src/tables/cff2.cpp \
    src/tables/cmap.cpp \
    src/tables/fvar.cpp \
    src/tables/gdef.cpp \
    src/hexview.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/parser.cpp \
    src/tables/glyf.cpp \
    src/tables/gvar.cpp \
    src/tables/head.cpp \
    src/tables/hhea.cpp \
    src/tables/hmtx.cpp \
    src/tables/loca.cpp \
    src/tables/maxp.cpp \
    src/tables/mvar.cpp \
    src/tables/name.cpp \
    src/tables/os2.cpp \
    src/tables/post.cpp \
    src/tables/stat.cpp \
    src/tables/vhea.cpp \
    src/tables/vmtx.cpp \
    src/tables/vorg.cpp \
    src/treemodel.cpp

HEADERS += \
    3rdparty/gsl-lite.hpp \
    src/algo.h \
    src/hexview.h \
    src/mainwindow.h \
    src/parser.h \
    src/tables/tables.h \
    src/treemodel.h
