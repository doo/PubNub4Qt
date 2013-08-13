PubNub4Qt
=========

Qt based PubNub implementation


Very simple subscribe/publish implementations. 

Encrypting support is enabled by defining ```Q_PUBNUB_CRYPT``` and calling ```QPubNubPublisher::encryptMessages``` with a cipher key.
Decrypting messages is enabled by calling ```QPubNubSubscriber::decryptMessages``` with the same cipher key respectivly.

