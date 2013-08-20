# hello_world project file

QT       += core network

QT       -= gui

TARGET = hello_world
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += ../QPubNub.h
LIBPATH += .

SOURCES += ../hello_world/main.cpp

# You have to set the location of the PubNub4Qt.lib
# LIBS += -L<PATH_FOR_PUBNUB4QT_LIB>
# LIBS += -lPubNub4Qt

include(common.pri)
