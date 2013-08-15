#include "QPubNub.h"

#include <QNetworkRequest>
#include <QDebug>
#include <QMetaMethod>

static const QString defaultOrigin("qt.pubnub.com");

QPubNub::QPubNub(QNetworkAccessManager* networkAccessManager, QObject* parent) : 
  QObject(parent), 
  m_trace(0),
  m_networkAccessManager(networkAccessManager),
  m_origin(defaultOrigin),
  m_ssl(true),
  m_timeToken("0"),
  m_resumeOnReconnect(false) {
}

QNetworkReply* QPubNub::sendRequest(const QString& url) {
  QNetworkRequest request(url);
#ifndef _DEBUG
  static bool originChecked = false;
  if (!originChecked) {
    originChecked = true;
    if (request.url().host() == defaultOrigin) {
      qWarning() << "Before running in production, please contact support@pubnub.com for your custom origin."
        << "\nPlease set the origin from qt.pubnub.com to <your custom origin>.pubnub.com to remove this warning.";
    }
  }
#endif
  request.setHeader(QNetworkRequest::UserAgentHeader, "Qt/1.0");
  request.setRawHeader("V", "3.4");
  if (m_trace) {
    emit trace(request.url().toString());
  }
  return m_networkAccessManager->get(request);
}

bool QPubNub::handleResponse(QNetworkReply* reply) const {
  reply->deleteLater();
  if (reply->error() != QNetworkReply::NoError) {
    return true;
  }

  const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (statusCode / 100 != 2) {
    emit error(reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString(), statusCode);
    return true;
  }
  return false;
}

void QPubNub::time() {
  if (m_origin.isEmpty()) {
    return emit error("Origin not set", 0);
  }
  auto reply = sendRequest(m_origin + "/time/0");
  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
  connect(reply, &QNetworkReply::finished, this, &QPubNub::onTimeFinished);
}

void QPubNub::onError(QNetworkReply::NetworkError) {
  auto reply = qobject_cast<QNetworkReply*>(sender());
  reply->deleteLater();
  emit error(reply->errorString(), 0);
}

void QPubNub::onTimeFinished() {
  auto reply = qobject_cast<QNetworkReply*>(sender());
  if (handleResponse(reply)) {
    return;
  }
  auto data(reply->readAll());
  const QJsonDocument doc(QJsonDocument::fromJson(data));
  // Response must be an array and first element must also be an array
  if (doc.isArray()) {
    emit timeResponse(doc.array()[0].toDouble());
  } else {
    emit error("Response is not an JSON array", 0);
  }
}

void QPubNub::resetOrigin() {
  m_origin = defaultOrigin;
}


QByteArray QPubNub::signature(const QByteArray& message, const QString& channel) const {
  QCryptographicHash hash(QCryptographicHash::Md5);
  hash.addData(m_publishKey);
  hash.addData("/", 1);
  hash.addData(m_subscribeKey);
  hash.addData("/", 1);
  hash.addData(m_secretKey);
  hash.addData("/", 1);
  hash.addData(channel.toLatin1());
  hash.addData("/", 1);
  hash.addData(message);
  hash.addData("", 1);
  return hash.result().toHex();
}

static QByteArray toByteArray(const QJsonValue& value) {
  switch (value.type()) {
  case QJsonValue::String:
    return value.toString().toLocal8Bit();
  case QJsonValue::Array:
    return QJsonDocument(value.toArray()).toJson(QJsonDocument::Compact);
  case QJsonValue::Object:
    return QJsonDocument(value.toObject()).toJson(QJsonDocument::Compact);
  case QJsonValue::Double:
    return QString::number(value.toDouble()).toLocal8Bit();
  }
  return QByteArray();
}

QString QPubNub::baseUrl() const {
  return QString("http%1://%2").arg(m_ssl ? "s" : "").arg(m_origin);
}

QString QPubNub::publishUrl(const QByteArray& message, const QString& channel) const {
  return QString("%1/publish/%2/%3/%4/%5/0/%6")
    .arg(baseUrl())
    .arg(m_publishKey.constData())
    .arg(m_subscribeKey.constData())
    .arg(m_secretKey.isEmpty() ? "0" : signature(message, channel).constData())
    .arg(channel)
    .arg(message.constData());
}

void QPubNub::publish(const QString& channel, const QJsonValue& value) {
  if (m_publishKey.isEmpty()) {
    return emit error("No publish key set", 0);
  }

  QByteArray message;
#ifdef Q_PUBNUB_CRYPT
  if (!m_cipherKey.isEmpty()) {
    QPubNubCrypt crypt(m_cipherKey);
    int errorCode = 0;
    message = crypt.encrypt(value, errorCode);
    if (errorCode != 0) {
      char errorString[1024+1];
      ERR_error_string_n(errorCode, errorString, 1024);
      emit error(errorString, errorCode);
      return;
    }
  } else {
#endif // Q_PUBNUB_CRYPT
    message = toByteArray(value);
  }

  auto reply = sendRequest(publishUrl("\""+message+"\"", channel));
  // This can't be connected using the new syntax, cause the signal and error property have the same name "error"
  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
  connect(reply, &QNetworkReply::readyRead, this, &QPubNub::publishFinished);
}

void QPubNub::publishFinished() {
  auto reply = qobject_cast<QNetworkReply*>(sender());
  if (handleResponse(reply)) {
    return;
  }
  auto data(reply->readAll());
  const QJsonDocument doc(QJsonDocument::fromJson(data));
  // Response must be an array and first element must also be an array
  if (doc.isArray()) {
    const QJsonArray array(doc.array());
    if (array.size() >= 2) {
      const QJsonValue statusCode(array[0]);
      if ((int)statusCode.toDouble() == 0) {
        emit error(array[1].toString(), 0);
      } else {
        emit published(array[2].toString());
      }
    } else {
      emit error("Response array is too small", 0);
    }
  } else {
    emit error("Response is not an JSON array", 0);
  }
}

QString QPubNub::subscribeUrl(const QString& channel) const {
  return QString("%1/subscribe/%2/%3/0/").arg(baseUrl()).arg(m_subscribeKey.constData()).arg(channel);
}

void QPubNub::subscribe(const QString& channel) {
  m_channels.insert(channel);
  QString url(subscribeUrl(channel) + m_timeToken);
  if (!m_uuid.isNull()) {
    url += "?uuid=" + m_uuid;
  }
  QNetworkReply* reply = sendRequest(url);
  connect(reply, &QNetworkReply::readyRead, this, &QPubNub::onSubscribeReadyRead);
  // This can't be connected using the new syntax, cause the signal and error property have the same name "error"
  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
}

void QPubNub::onSubscribeReadyRead() {
  auto reply = qobject_cast<QNetworkReply*>(sender());
  if (handleResponse(reply)) {
    return;
  }

  auto data(reply->readAll());
  //qDebug() << data.data();
  const QJsonDocument doc(QJsonDocument::fromJson(data));
  // Response must be an array and first element must also be an array
  if (doc.isArray()) {
    const QJsonArray array(doc.array());
    auto firstElement(array.at(0));
    if (firstElement.isArray()) {
      auto messages = firstElement.toArray();
      auto timeTokenElement = array.at(1);
      if (timeTokenElement.isString()) {
        m_timeToken = timeTokenElement.toString();
        // TODO:: Extract channels
        auto channelListString = array.at(2);
        QStringList channels;
        if (channelListString.isString()) {
          channels = channelListString.toString().split(',');
        }
        if (!messages.isEmpty()) {
#ifdef Q_PUBNUB_CRYPT
          if (m_cipherKey.isEmpty()) {
            for (int i=0,len=messages.size();i<len;++i) {
              emit message(messages[i], m_timeToken, channels[i]);
            }
          } else {
            decrypt(messages, channels);
          }
#else
          for (int i=0,len=messages.size();i<len;++i) {
            emit message(messages[i], m_timeToken, channels[i]);
          }
#endif // Q_PUBNUB_CRYPT
        } else {
          emit connected();
        }
      } else {
        emit error("Second element of response is not a string", 0);
      }
    } else {
      emit error("First element of response is not an JSON array", 0);
    }
  } else {
    emit error("Response is not an JSON array", 0);
  }
    
  subscribe(m_channels.values()[0]);
}
  
#ifdef Q_PUBNUB_CRYPT
bool QPubNub::decrypt(const QJsonArray& messages, const QStringList& channels) {
  QPubNubCrypt crypt(m_cipherKey);
  for (int i=0,len=messages.size();i<len;++i) {
    int errorCode = 0;
    auto value(crypt.decrypt(messages[i].toString().toLocal8Bit(), errorCode));
    if (errorCode != 0) {
      char errorString[1024+1];
      ERR_error_string_n(errorCode, errorString, 1024);
      emit error(errorString, errorCode);
    } else {
      emit message(value, m_timeToken, channels[i]);
    }
  }
  return true;
}
#endif // Q_PUBNUB_CRYPT

void QPubNub::connectNotify(const QMetaMethod& signal) {
  if (signal == QMetaMethod::fromSignal(&QPubNub::trace)) {
    ++m_trace;
  }
}

void QPubNub::disconnectNotify(const QMetaMethod& signal) {
  if (signal == QMetaMethod::fromSignal(&QPubNub::trace)) {
    --m_trace;
  }
}