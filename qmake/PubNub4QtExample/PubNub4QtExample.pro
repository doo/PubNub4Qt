# PubNub4QtExample project file

QT       += core network

QT       -= gui

TARGET = hello_world
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += ../../QPubNub.h
LIBPATH += .

SOURCES += ../../hello_world/main.cpp

include(../common.pri)

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../PubNub4QtLib/release/ -lPubNub4QtLib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../PubNub4QtLib/debug/ -lPubNub4QtLib
else:unix: LIBS += -L$$OUT_PWD/../PubNub4QtLib/ -lPubNub4QtLib

INCLUDEPATH += $$PWD/../PubNub4QtLib
DEPENDPATH += $$PWD/../PubNub4QtLib

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../PubNub4QtLib/release/PubNub4QtLib.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../PubNub4QtLib/debug/PubNub4QtLib.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../PubNub4QtLib/libPubNub4QtLib.a
