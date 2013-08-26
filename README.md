PubNub4Qt
=========

Qt based PubNub implementation for publishing and subscriptions.

To test it, head over to the [PubNub Console](http://www.pubnub.com/console?channel=hello_qtworld&origin=pubsub.pubnub.com&sub=demo&pub=demo) wait until it has connected to the ```hello_qtworld``` channel and then start [the example](https://github.com/doo/PubNub4Qt/blob/master/hello_world/main.cpp).
You should see "```Hello from Qt world```" in the PubNub consoles messages view.

when you send the message ```"Hello from Dev Console"``` (including the quotation marks, as it has to be a valid JSON string in the console) you should see this printed on the applications console:

```[hello_qtworld]:QJsonValue(string, "Hello from Dev Console")```


License
-------

MIT License
