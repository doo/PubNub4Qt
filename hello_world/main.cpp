#include <QtCore/QCoreApplication>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QJSonDocument>
#include <QJsonObject>
#include <QDebug>

#include "../QPubNub.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QNetworkAccessManager networkAccessManager;

    QPubNub pubnub(&networkAccessManager);
    pubnub.setPublishKey("demo");
    pubnub.setSubscribeKey("demo");
    pubnub.connect(&pubnub, &QPubNub::error, [](QString message, int code) {
      qDebug() << "error:" << message;
    });
    pubnub.setCipherKey("enigma");
    pubnub.publish("qwertz", QJsonValue(QString("test")));
    pubnub.subscribe("qwertz,qwertz2");
    pubnub.connect(&pubnub, &QPubNub::message, [](QJsonValue value, QString timeToken, QString channel) {
      qDebug() << "[" << channel << "]:" << value;
    });

    pubnub.connect(&pubnub, &QPubNub::trace, [](QString message) {
      qDebug() << "Trace:" << message;
    });
    return a.exec();
}
