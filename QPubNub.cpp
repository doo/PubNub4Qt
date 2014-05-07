#include "QPubNub.h"

#include <QNetworkRequest>
#include <QDebug>
#include <QMetaMethod>
#include <QCryptographicHash>

static const QString defaultOrigin("qt.pubnub.com");

QPubNub::QPubNub(QNetworkAccessManager* networkAccessManager, QObject* parent) : 
  QObject(parent), 
  m_networkAccessManager(networkAccessManager),
  m_origin(defaultOrigin),
  m_resumeOnReconnect(false),
  m_ssl(true),
  m_timeToken("0"),
  m_trace(0)
{
}

QNetworkReply* QPubNub::sendRequest(const QString& path) {
  QNetworkRequest request(baseUrl() + path);
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

  // Only emit signal if there are connections currently.
  if (m_trace) {
    emit trace(request.url().toString());
  }
  return m_networkAccessManager->get(request);
}

bool QPubNub::handleResponse(QNetworkReply* reply, QJsonArray& response) const {
  reply->deleteLater();
  if (reply->error() != QNetworkReply::NoError) {
    return true;
  }

  const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (statusCode / 100 != 2) {
    emit error(reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString(), statusCode);
    return true;
  }

  QByteArray data(reply->readAll());
  if (m_trace) {
    emit trace(QString("Response for %1:\n%2").arg(reply->request().url().toString()).arg(data.constData()));
  }    
  const QJsonDocument doc(QJsonDocument::fromJson(data));
  // Response must be an array and first element must also be an array
  if (doc.isArray()) {
    response = doc.array();
  } else {
    emit error("Response is not an JSON array", 0);
    return true;
  }
  return false;
}

void QPubNub::time() {
  if (m_origin.isEmpty()) {
    emit error("Origin not set", 0);
    return;
  }
  QNetworkReply * reply = sendRequest("/time/0");
  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
  connect(reply, &QNetworkReply::finished, this, &QPubNub::onTimeFinished);
}

void QPubNub::onError(QNetworkReply::NetworkError) {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  reply->deleteLater();
  emit error(reply->errorString(), 0);
}

void QPubNub::onTimeFinished() {
  QNetworkReply * reply = qobject_cast<QNetworkReply*>(sender());
  QJsonArray result;
  if (handleResponse(reply, result)) {
    return;
  }
  emit timeResponse((quint64)(result[0].toDouble()));
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

QString QPubNub::baseUrl() const {
  return QString("http%1://%2").arg(m_ssl ? "s" : "").arg(m_origin);
}

QString QPubNub::publishUrl(const QByteArray& message, const QString& channel) const {
  return QString("/publish/%1/%2/%3/%4/0/%5")
    .arg(m_publishKey.constData())
    .arg(m_subscribeKey.constData())
    .arg(m_secretKey.isEmpty() ? "0" : signature(message, channel).constData())
    .arg(channel)
    .arg(message.constData());
}

void QPubNub::publish(const QString& channel, const QJsonValue& value) {
  if (m_publishKey.isEmpty()) {
    emit error("No publish key set", 0);
    return ;
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
#ifdef Q_PUBNUB_CRYPT
  }
#endif

  QNetworkReply * reply = sendRequest(publishUrl("\""+message+"\"", channel));
  // This can't be connected using the new syntax, cause the signal and error property have the same name "error"
  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
  connect(reply, &QNetworkReply::finished, this, &QPubNub::publishFinished);
}

void QPubNub::publishFinished() {
  QNetworkReply * reply = qobject_cast<QNetworkReply*>(sender());
  QJsonArray response;
  if (handleResponse(reply, response)) {
    return;
  }
  if (response.size() >= 2) {
    const QJsonValue statusCode(response[0]);
    if ((int)statusCode.toDouble() == 0) {
      emit error(response[1].toString(), 0);
    } else {
      emit published(response[2].toString());
    }
  } else {
    emit error("Response array is too small", 0);
  }
}

QString QPubNub::subscribeUrl(const QString& channel) const {
  return QString("/subscribe/%1/%2/0/").arg(m_subscribeKey.constData()).arg(channel);
}

void QPubNub::subscribe(const QString& channel) {
  if (m_channels.contains(channel)) {
    return;
  }
  m_channels.insert(channel);
  if (m_channelUrlPart.isEmpty()) {
    m_channelUrlPart = channel;
  } else {
    m_channelUrlPart += "," + channel;
  }

  // subscribe uses the QNetworkAccessManager so make sure it runs on the correct thread
  // by invoking it through the Qt message loop
  QMetaObject::invokeMethod(this, "subscribe", Qt::QueuedConnection);
}

void QPubNub::subscribe() {
  QString url(subscribeUrl(m_channelUrlPart) + m_timeToken);
  if (!m_uuid.isNull()) {
    url += "?uuid=" + m_uuid;
  }
  QNetworkReply* reply = sendRequest(url);
  connect(reply, &QNetworkReply::finished, this, &QPubNub::onSubscribeReadyRead);
  // This can't be connected using the new syntax, cause the signal and error property have the same name "error"
  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
}

void QPubNub::onSubscribeReadyRead() {
  QNetworkReply * reply = qobject_cast<QNetworkReply*>(sender());
  QJsonArray response;
  if (handleResponse(reply, response)) {
    return;
  }
  QJsonValue firstElement(response.at(0));
  if (!firstElement.isArray()) {
    emit error("First element of response is not an JSON array", 0);
    subscribe();
  }
  
  QJsonValue timeTokenElement = response.at(1);
  if (!timeTokenElement.isString()) {
    emit error("Second element of response is not a string", 0);
    subscribe();
  }
  m_timeToken = timeTokenElement.toString();
  QJsonValue channelListString = response.at(2);
  QStringList channels;
  QJsonArray messages = firstElement.toArray();
  if (channelListString.isString()) {
    channels = channelListString.toString().split(',');
  } else {
    int len = messages.isEmpty() ? 0 : messages.size();
    for (int i = 0; i < len; i++)
    {
        channels << m_channelUrlPart;
    }
  }
  if (messages.isEmpty()) {
    emit connected();
  } else {
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
  }
  subscribe();
}
  
#ifdef Q_PUBNUB_CRYPT
bool QPubNub::decrypt(const QJsonArray& messages, const QStringList& channels) {
  QPubNubCrypt crypt(m_cipherKey);
  for (int i=0,len=messages.size();i<len;++i) {
    int errorCode = 0;
    QJsonValue value(crypt.decrypt(messages[i].toString().toLocal8Bit(), errorCode));
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
