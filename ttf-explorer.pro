QT      += core gui widgets

TARGET   = ttf-explorer
TEMPLATE = app

CONFIG  += c++17
CONFIG  += sdk_no_version_check

equals(QMAKE_CXX, clang++) {
    QMAKE_CXXFLAGS += -Wextra -Wpedantic -Wimplicit-fallthrough -Wconversion
}

# required to make C++17 work on macOS
mac:QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

SOURCES += \
    src/hexview.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/parser.cpp \
    src/tables/avar.cpp \
    src/tables/cbdt.cpp \
    src/tables/cblc.cpp \
    src/tables/cff.cpp \
    src/tables/cff2.cpp \
    src/tables/cmap.cpp \
    src/tables/feat.cpp \
    src/tables/fvar.cpp \
    src/tables/gdef.cpp \
    src/tables/glyf.cpp \
    src/tables/gvar.cpp \
    src/tables/head.cpp \
    src/tables/hhea.cpp \
    src/tables/hmtx.cpp \
    src/tables/hvar.cpp \
    src/tables/kern.cpp \
    src/tables/loca.cpp \
    src/tables/maxp.cpp \
    src/tables/mvar.cpp \
    src/tables/name.cpp \
    src/tables/os2.cpp \
    src/tables/post.cpp \
    src/tables/sbix.cpp \
    src/tables/stat.cpp \
    src/tables/svg.cpp \
    src/tables/vhea.cpp \
    src/tables/vmtx.cpp \
    src/tables/vorg.cpp \
    src/tables/vvar.cpp \
    src/treemodel.cpp \
    src/truetype.cpp \
    src/utils.cpp

HEADERS += \
    src/algo.h \
    src/app.h \
    src/hexview.h \
    src/mainwindow.h \
    src/parser.h \
    src/range.h \
    src/tables/cff.h \
    src/tables/name.h \
    src/tables/tables.h \
    src/treemodel.h \
    src/truetype.h \
    src/utils.h
