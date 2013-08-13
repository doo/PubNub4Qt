#pragma once

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>

class QPubNubBase : public QObject {
  Q_OBJECT

signals:
  void error(const QString& message) const;

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

  bool handleResponse(QNetworkReply* reply) const {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      return true;
    }

    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode / 100 != 2) {
      emit error(reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString());
      return true;
    }
    return false;
  }

private:
  QNetworkAccessManager* m_networkAccessManager;
};