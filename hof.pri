lessThan(QT_MAJOR_VERSION, 5) {
   error(Qt version 5.2 or greater is required)
}

equals(QT_MAJOR_VERSION, 5) {
    lessThan(QT_MINOR_VERSION, 2) {
        error(Qt version 5.2 or greater is required)
    }
}

OUTPUT_DIR = $$(OUTPUT_DIR)
isEmpty(OUTPUT_DIR):OUTPUT_DIR=$$PWD/build

QT += core
QT -= gui

CONFIG += qt warn_on c++11

DEBUG_MODE = $$(DEBUG_MODE)
contains(DEBUG_MODE, 1) {
  CONFIG += debug
}

OBJECTS_DIR = tmp
MOC_DIR = tmp

TOPLEVELDIR = $$PWD
