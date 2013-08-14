#include <QtCore/QCoreApplication>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QJSonDocument>
#include <QJsonObject>
#include <QDebug>

#include "../QPubNubPublisher.h"
#include "../QPubNubSubscription.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QNetworkAccessManager networkAccessManager;

    QPubNubPublisher publisher(&networkAccessManager, "demo", "demo", "hello_world");
    publisher.signMessages("test");
#ifdef Q_PUBNUB_CRYPT
    publisher.encryptMessages("test");
#endif
    QTimer timer;
    timer.connect(&timer, &QTimer::timeout, [&] {
      publisher.publish(QJsonDocument::fromJson("{\"msg\":\"hi\"}").object());
    });
    timer.start(5000);

    QPubNubSubscription subscription(&networkAccessManager, "demo", "hello_world");
#ifdef Q_PUBNUB_CRYPT
    subscription.decryptMessages("test");
#endif
    subscription.subscribe();
    QObject::connect(&subscription, &QPubNubSubscription::updateReceived, [&](const QJsonArray& messages) {
      qDebug() << "Got update" << messages;
    });
    return a.exec();
}
