# PubNub4Qt project file

QT        += core network
QT        -= gui

TEMPLATE  = lib

DEFINES   += Q_PUBNUB_CRYPT

HEADERS   = ../../QPubNub.h
SOURCES   = ../../QPubNub.cpp

contains(QMAKE_TARGET.arch, x86_64):{
  INCLUDEPATH += "C:\OpenSSL-Win64\include"
  LIBPATH += "C:\OpenSSL-Win64\lib"
}

contains(QMAKE_TARGET.arch, x86):{
  INCLUDEPATH += "C:\OpenSSL-Win32\include"
  LIBPATH += "C:\OpenSSL-Win32\lib"
}

LIBS      += -llibeay32
