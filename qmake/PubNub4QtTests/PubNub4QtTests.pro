# PubNub4QtTests project file

QT       += testlib network
QT       -= gui

DEFINES   += Q_PUBNUB_CRYPT

TARGET = QPubNubTests
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

HEADERS   += ../../tests/QPubNubTests.h
SOURCES   += ../../tests/QPubNubTests.cpp

include(../common.pri)

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../PubNub4QtLib/release/ -lPubNub4QtLib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../PubNub4QtLib/debug/ -lPubNub4QtLib
else:unix: LIBS += -L$$OUT_PWD/../PubNub4QtLib/ -lPubNub4QtLib

INCLUDEPATH += $$PWD/../PubNub4QtLib
DEPENDPATH += $$PWD/../PubNub4QtLib

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../PubNub4QtLib/release/PubNub4QtLib.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../PubNub4QtLib/debug/PubNub4QtLib.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../PubNub4QtLib/libPubNub4QtLib.a
