include($$PWD/../hof.pri)
include($$PWD/../src/hof.pri)

TEMPLATE = app
TARGET = hoftests
DESTDIR = $$OUTPUT_DIR/bin
QT += testlib

DEPENDPATH += .
INCLUDEPATH += .

HEADERS += testhof.h

SOURCES += main_tests.cpp \
           testhof.cpp
