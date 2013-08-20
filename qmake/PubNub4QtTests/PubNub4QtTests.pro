QT       += testlib network
QT       -= gui

DEFINES   += Q_PUBNUB_CRYPT

TARGET = QPubNubTests
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


HEADERS   = ../../tests/QPubNubTests.h
SOURCES   = ../../tests/QPubNubTests.cpp

contains(QMAKE_TARGET.arch, x86_64):{
  INCLUDEPATH += "C:\OpenSSL-Win64\include"
  LIBPATH += "C:\OpenSSL-Win64\lib"
}

contains(QMAKE_TARGET.arch, x86):{
  INCLUDEPATH += "C:\OpenSSL-Win32\include"
  LIBPATH += "C:\OpenSSL-Win32\lib"
}

LIBS      += -llibeay32
