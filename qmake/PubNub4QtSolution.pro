TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS += PubNub4QtLib PubNub4QtExample PubNub4QtTests

PubNub4QtTests.depends = PubNub4QtLib
PubNub4QtExample.depends = PubNub4QtLib
