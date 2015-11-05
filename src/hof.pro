include($$PWD/../hof.pri)

TEMPLATE = app
TARGET = hof
DESTDIR = $$OUTPUT_DIR/bin

DEPENDPATH += .
INCLUDEPATH += .

SOURCES += main_hof.cpp

include($$PWD/hof.pri)
