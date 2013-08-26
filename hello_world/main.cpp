#include <QtCore/QCoreApplication>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QJSonDocument>
#include <QJsonObject>
#include <QDebug>

#include "../QPubNub.h"

/**
Once you run this program, go to the PubNub console
http://www.pubnub.com/console

connect to the "hello_world" channel with the cipherkey set to "enigma" and "demo/demo" as publish/subscribe keys.

Now when you send the message "Hello from Dev Console" (including the quotation marks, as it has to be a valid JSON string in the console)
you should see this printed on the applications console:
[hello_world]:QJsonValue(string, "Hello from Dev Console")
*/
int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QNetworkAccessManager networkAccessManager;

    QPubNub pubnub(&networkAccessManager);
    if (a.arguments().contains("-trace")) {
      pubnub.connect(&pubnub, &QPubNub::trace, [](QString message) {
        qDebug() << "Trace:" << qPrintable(message);
      });
    }
    pubnub.time();
    pubnub.setPublishKey("demo");
    pubnub.setSubscribeKey("demo");
    pubnub.setCipherKey("enigma");
    pubnub.connect(&pubnub, &QPubNub::error, [](QString message, int /* code */) {
      qDebug() << "error:" << qPrintable(message);
    });
    pubnub.connect(&pubnub, &QPubNub::message, [](QJsonValue value, QString timeToken, QString channel) {
      qDebug().nospace() << "[" << qPrintable(channel) << "]:" << value;
    });
    pubnub.subscribe("hello_qtworld");
    pubnub.publish("hello_qtworld", QJsonValue(QString("Hello from Qt World")));

    return a.exec();
}
