# PubNub4Qt project file

QT        += core network
QT        -= gui

TEMPLATE  = lib

DEFINES   += Q_PUBNUB_CRYPT

HEADERS   = ../QPubNub.h
SOURCES   = ../QPubNub.cpp

CONFIG += staticlib
CONFIG += dll

include(common.pri)
