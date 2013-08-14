#include "QPubNubBase.h"

#include <QNetworkRequest>

QPubNubBase::QPubNubBase(QNetworkAccessManager* networkAccessManager, QObject* parent) : 
  QObject(parent), m_networkAccessManager(networkAccessManager) {
}

QNetworkReply* QPubNubBase::sendRequest(QString& url) {
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::UserAgentHeader, "Qt/1.0");
  request.setRawHeader("V", "3.4");
  //qDebug() << request.url();
  return m_networkAccessManager->get(request);
}

bool QPubNubBase::handleResponse(QNetworkReply* reply) const {
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