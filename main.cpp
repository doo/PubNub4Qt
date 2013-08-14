#include <QtCore/QCoreApplication>
#include <QNetworkAccessManager>
#include <QTimer>

#include "QPubNubPublisher.h"
#include "QPubNubSubscription.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QNetworkAccessManager networkAccessManager;

    QPubNubPublisher publisher(&networkAccessManager, "demo", "demo", "hello_world");
    publisher.signMessages("test");
    publisher.encryptMessages("test");
    QTimer timer;
    timer.connect(&timer, &QTimer::timeout, [&] {
      publisher.publish(QJsonDocument::fromJson("{\"msg\":\"hi\"}").object());
    });
    timer.start(5000);

    QPubNubSubscription subscription(&networkAccessManager, "demo", "hello_world");
    subscription.decryptMessages("test");
    subscription.subscribe();
    QObject::connect(&subscription, &QPubNubSubscription::updateReceived, [&](const QJsonArray& messages) {
      qDebug() << "Got update" << messages;
    });
    return a.exec();
}
