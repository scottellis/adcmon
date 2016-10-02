cache

TEMPLATE = app

TARGET = adcmon

QT += core gui widgets

CONFIG += release

unix {
    target.path = /usr/bin
    INSTALLS += target
}

INCLUDEPATH += GeneratedFiles

MOC_DIR = GeneratedFiles
OBJECTS_DIR = Objects
UI_DIR = GeneratedFiles
RCC_DIR = GeneratedFiles

include(adcmon.pri)
