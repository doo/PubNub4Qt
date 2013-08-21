TEMPLATE = subdirs
SUBDIRS = PubNub4QtLib PubNub4QtTests PubNub4QtExample


PubNub4QtTests.depends = PubNub4QtLib
PubNub4QtExample.depends = PubNub4QtLib
