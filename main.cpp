#include <QtCore/QCoreApplication>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QTimer>
#define Q_PUBNUB_CRYPT
#include "QPubNubPublisher.h"
#include "QPubNubSubscription.h"
#pragma comment(lib, "libeay32")

/*

#include <winsock2.h>
#include <netioapi.h>
#pragma comment(lib, "Iphlpapi")
class QNetworkState : public QObject {
  Q_OBJECT
signals:
  void onlineStateChanged(bool online);

public:
  QNetworkState(QObject* parent = nullptr) : QObject(parent) {
    HANDLE handle;
    ::NotifyIpInterfaceChange(AF_UNSPEC, callback, this, false, &handle);
  }

private:
  static void callback(void* context, PMIB_IPINTERFACE_ROW row, MIB_NOTIFICATION_TYPE type) {
    if (type != MibInitialNotification) {
      GetIpInterfaceEntry(row);
      reinterpret_cast<QNetworkState*>(context)->update(row, type);
    }
  }

  void update(PMIB_IPINTERFACE_ROW row, MIB_NOTIFICATION_TYPE type) {
  }
};
*/


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
    //publisher.publish("{\"msg\":\"hi\"}");
    timer.start(1000);

    QPubNubSubscriber subscriber(&networkAccessManager, "demo", "hello_world");
    subscriber.subscribe();
    subscriber.setCipherKey("test");
    QObject::connect(&subscriber, &QPubNubSubscriber::updateReceived, [&](const QJsonArray& messages) {
      auto first = messages.at(0);
      qDebug() << "Got update" << first;
    });
    return a.exec();
}
