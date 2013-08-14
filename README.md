PubNub4Qt
=========

Qt based PubNub implementation for publishing and subscriptions.

Encrypting support is enabled by defining ```Q_PUBNUB_CRYPT``` and calling ```QPubNubPublisher::encryptMessages``` with a cipher key.
Decrypting messages is enabled by calling ```QPubNubSubscriber::decryptMessages``` with the same cipher key respectivly.

Currently _MSVC2012 only_. Qt project files welcomed as pull-request.
