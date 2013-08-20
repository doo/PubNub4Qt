# PubNub4QtTests project file

QT       += testlib network
QT       -= gui

DEFINES   += Q_PUBNUB_CRYPT

TARGET = QPubNubTests
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


HEADERS   = ../tests/QPubNubTests.h
SOURCES   = ../tests/QPubNubTests.cpp

include(common.pri)
