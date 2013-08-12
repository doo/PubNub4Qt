
#include <QtCore/QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QTimer>
#define PUBNUB_CRYPT
#include "QPubNub.h"

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

    QPubNubPublisher publisher("demo", "demo", "hello_world");
    QTimer timer;
    timer.connect(&timer, &QTimer::timeout, [&] {
      publisher.publish("{\"msg\":\"hi\"}");
    });
    timer.start(3000);

    QPubNubSubscriber subscriber("demo", "hello_world");
    subscriber.subscribe();
    QObject::connect(&subscriber, &QPubNubSubscriber::updateReceived, [&](const QJsonArray& messages) {
      auto first = messages.at(0);
      if (first.isObject()) {
        qDebug() << "Got update" << first.toObject();
      }
    });
    return a.exec();
}
