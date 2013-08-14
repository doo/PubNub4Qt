#include "QPubNubSubscription.h"

QPubNubSubscription::QPubNubSubscription(
  QNetworkAccessManager* networkAccessManager, 
  const QString& subscribeKey, 
  const QString& channel, 
  const QString& uuid, 
  QObject* parent) : 
  QPubNubBase(networkAccessManager, parent), 
  m_timeToken("0"),
  m_uuid(uuid),
  m_url(QString("https://pubsub.pubnub.com/subscribe/%1/%2/0/").arg(subscribeKey).arg(channel)) {
}

#ifdef Q_PUBNUB_CRYPT
void QPubNubSubscription::decryptMessages(const QByteArray& cipherKey) {
  m_cipherKey = QCryptographicHash::hash(cipherKey, QCryptographicHash::Sha256).toHex();
}
#endif

void QPubNubSubscription::subscribe() {
  QString url(m_url + m_timeToken);
  if (!m_uuid.isEmpty()) {
    url += "?uuid=" + m_uuid;
  }
  QNetworkReply* reply = sendRequest(url);
  connect(reply, &QNetworkReply::readyRead, this, &QPubNubSubscription::onSubscribeReadyRead);
  // This can't be connected using the new syntax, cause the signal and error property have the same name "error"
  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
}

void QPubNubSubscription::onError(QNetworkReply::NetworkError code) {
  auto reply = qobject_cast<QNetworkReply*>(sender());
  reply->deleteLater();
  QTimer::singleShot(250, this, SLOT(subscribe));
  emit error(reply->errorString());
}

void QPubNubSubscription::onSubscribeReadyRead() {
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
        if (!messages.isEmpty()) {
#ifdef Q_PUBNUB_CRYPT
          if (m_cipherKey.isEmpty()) {
            emit updateReceived(messages);
          } else {
            QJsonArray decryptedMessages;
            if (decrypt(messages, decryptedMessages)) {
              emit updateReceived(decryptedMessages);
            }
          }
#else
          emit updateReceived(messages);
#endif // Q_PUBNUB_CRYPT
        } else {
          emit connected();
        }
      } else {
        emit error("Second element of response is not a string");
      }
    } else {
      emit error("First element of response is not an JSON array");
    }
  } else {
    emit error("Response is not an JSON array");
  }
    
  subscribe();
}
  
#ifdef Q_PUBNUB_CRYPT
bool QPubNubSubscription::decrypt(const QJsonArray& messages, QJsonArray& decryptedMessages) {
  static const QByteArray iv("0123456789012345");
  CipherContext aes256;
        
  for (auto value : messages) {
    QByteArray data(QByteArray::fromBase64(value.toString().toLocal8Bit()));
    int errorCode = 0;
    auto decrpytedMessage(aes256.aesDecrypt(m_cipherKey, iv, data, errorCode));
    if (errorCode != 0) {
      char errorString[1024+1];
      ERR_error_string_n(errorCode, errorString, 1024);
      emit error(errorString);
      break;
    }
    QJsonDocument doc(QJsonDocument::fromJson(decrpytedMessage));
    if (doc.isNull() || doc.isEmpty()) {
      const QString stringValue(decrpytedMessage);
      if (stringValue.isEmpty()) {
        decryptedMessages.append(QJsonValue());
      } else {
        // Try if its a number first
        bool ok;
        const double numberValue = stringValue.toDouble(&ok);
        if (ok) {
          decryptedMessages.append(QJsonValue(numberValue));
        } else {
          decryptedMessages.append(QJsonValue(stringValue));
        }
      }
    } else if (doc.isArray()) {
      decryptedMessages.append(doc.array());
    } else if (doc.isObject()) {
      decryptedMessages.append(doc.object());
    }
  }
  return true;
}
#endif // Q_PUBNUB_CRYPT