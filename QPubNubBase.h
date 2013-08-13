#pragma once

#include <QNetworkRequest>
#include <QNetworkAccessManager>

class QPubNubBase : public QObject {
  Q_OBJECT
protected:
  QPubNubBase(QNetworkAccessManager* networkAccessManager, QObject* parent = nullptr) : 
    QObject(parent), m_networkAccessManager(networkAccessManager) {
  }

  QNetworkReply* sendRequest(QString& url) {
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "Qt/1.0");
    request.setRawHeader("V", "3.4");
    //qDebug() << request.url();
    return m_networkAccessManager->get(request);
  }

private:
  QNetworkAccessManager* m_networkAccessManager;
};