# hello_world project file

QT       += core network

QT       -= gui

TARGET = hello_world
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += ../../QPubNub.h
LIBPATH += .

SOURCES += ../../hello_world/main.cpp

LIBS += -L../build
LIBS += -lPubNub4Qt

include(../common.pri)
