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
    pubnub.connect(&pubnub, &QPubNub::trace, [](QString message) {
      qDebug() << "Trace:" << qPrintable(message);
    });
    pubnub.time();
    pubnub.setPublishKey("demo");
    pubnub.setSubscribeKey("demo");
    pubnub.connect(&pubnub, &QPubNub::error, [](QString message, int /* code */) {
      qDebug() << "error:" << qPrintable(message);
    });
    pubnub.setCipherKey("enigma");
    pubnub.publish("qwertz", QJsonValue(QString("test")));
    pubnub.subscribe("qwertz");
    pubnub.connect(&pubnub, &QPubNub::message, [](QJsonValue value, QString timeToken, QString channel) {
      qDebug().nospace() << "[" << qPrintable(channel) << "]:" << value;
    });

    return a.exec();
}
