QT      += core gui widgets

TARGET   = ttf-explorer
TEMPLATE = app

CONFIG  += c++17

# Fixes ttfcore linking error on Linux.
unix:!macx:LIBS += -ldl -fPIC

# Link core
CONFIG(release, debug|release): LIBS += -L$$PWD/core/target/release -lttfcore
else:CONFIG(debug, debug|release): LIBS += -L$$PWD/core/target/debug -lttfcore
INCLUDEPATH += $$PWD/core
DEPENDPATH += $$PWD/core

DEFINES += QT_NO_CAST_FROM_ASCII

SOURCES += \
    src/hexview.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/treemodel.cpp \
    src/ttfcorepp.cpp

HEADERS += \
    src/hexview.h \
    src/mainwindow.h \
    src/range.h \
    src/treemodel.h \
    src/ttfcorepp.h
