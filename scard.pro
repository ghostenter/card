#-------------------------------------------------
#
# Project created by QtCreator 2013-11-18T16:15:59
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = scard
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    smartcardmanage.cpp

HEADERS += \
    smartcardmanage.h


LIBS += -LD:\Qt\Qt5.2.0\Tools\mingw48_32\i686-w64-mingw32\lib -lwinscard
