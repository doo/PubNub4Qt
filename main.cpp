#include <QtCore/QCoreApplication>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QTimer>

#include "QPubNubPublisher.h"
#include "QPubNubSubscription.h"
#pragma comment(lib, "libeay32")

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QNetworkAccessManager networkAccessManager;
#if 1
    QPubNubPublisher publisher(&networkAccessManager, "demo", "demo", "hello_world");
    publisher.signMessages("test");
    publisher.encryptMessages("test");
    QTimer timer;
    timer.connect(&timer, &QTimer::timeout, [&] {
      publisher.publish(QJsonDocument::fromJson("{\"msg\":\"hi\"}").object());
    });
    //publisher.publish("{\"msg\":\"hi\"}");
    timer.start(1000);
#endif
    QPubNubSubscriber subscriber(&networkAccessManager, "demo", "hello_world");
    subscriber.decryptMessages("test");
    subscriber.subscribe();
    QObject::connect(&subscriber, &QPubNubSubscriber::updateReceived, [&](const QJsonArray& messages) {
      qDebug() << "Got update" << messages;
    });
    return a.exec();
}
