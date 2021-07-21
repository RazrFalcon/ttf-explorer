QT      += core gui widgets

TARGET   = ttf-explorer
TEMPLATE = app

CONFIG  += c++17

# Fixes ttfcore linking error on Linux.
unix:!mac:LIBS += -ldl -fPIC

# required to make C++17 work
mac:QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

# Link core
CONFIG(release, debug|release): LIBS += -L$$PWD/target/release -lttfexplorer
else:CONFIG(debug, debug|release): LIBS += -L$$PWD/target/debug -lttfexplorer

DEFINES += QT_NO_CAST_FROM_ASCII

SOURCES += \
    ui/hexview.cpp \
    ui/main.cpp \
    ui/mainwindow.cpp \
    ui/treemodel.cpp \
    ui/ttfcorepp.cpp

HEADERS += \
    ui/app.h \
    ui/hexview.h \
    ui/mainwindow.h \
    ui/range.h \
    ui/treemodel.h \
    ui/ttfcorepp.h
