#-------------------------------------------------
#
# Project created by QtCreator 2012-11-03T00:35:02
#
#-------------------------------------------------

QT       += core gui network

TARGET = client
TEMPLATE = app

DESTDIR = ../bin
OBJECTS_DIR = ./o
MOC_DIR = ./moc


SOURCES += main.cpp\
        mainwindow.cpp \
    localmodel.cpp

HEADERS  += mainwindow.h \
    localmodel.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc
