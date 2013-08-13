#pragma once

#include <QNetworkReply>
#include <QJSonDocument>
#include <QJSonArray>
#include <QJSonValue>
#include <QJSonObject>
#include <QDebug>
#include <QTimer>

#ifdef Q_PUBNUB_CRYPT
#include "CipherContext.h"
#include <openssl/err.h>
#include <QCryptographicHash>
#endif // Q_PUBNUB_CRYPT


#include "QPubNubBase.h"

class QPubNubSubscriber : public QPubNubBase {
  Q_OBJECT
signals:
  void connected();
  void updateReceived(const QJsonArray& messages);
  void error(const QString& message);

public:
  QPubNubSubscriber(QNetworkAccessManager* networkAccessManager, const QString& subscribeKey, const QString& channel, const QString& uuid = QString(), QObject* parent = nullptr) : 
    QPubNubBase(networkAccessManager, parent), 
    m_timeToken("0"),
    m_uuid(uuid),
    m_url(QString("https://pubsub.pubnub.com/subscribe/%1/%2/0/").arg(subscribeKey).arg(channel)) {
  }

#ifdef Q_PUBNUB_CRYPT
  void setCipherKey(const QByteArray& cipherKey) {
    m_cipherKey = QCryptographicHash::hash(cipherKey, QCryptographicHash::Sha256).toHex();
  }
#endif

public slots:
  void subscribe() {
    QString url(m_url + m_timeToken);
    if (!m_uuid.isEmpty()) {
      url += "?uuid=" + m_uuid;
    }
    QNetworkReply* reply = sendRequest(url);
    connect(reply, &QNetworkReply::readyRead, this, &QPubNubSubscriber::onSubscribeReadyRead);
    // This can't be connected using the new syntax, cause the signal and error property have the same name "error"
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
  }

private slots:
  void onError(QNetworkReply::NetworkError code) {
    auto reply = qobject_cast<QNetworkReply*>(sender());
    reply->deleteLater();
    QTimer::singleShot(250, this, SLOT(subscribe));
    emit error(reply->errorString());
  }

  void onSubscribeReadyRead() {
    auto reply = qobject_cast<QNetworkReply*>(sender());
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      return;
    }

    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode / 100 != 2) {
      emit error(reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString());
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
  bool decrypt(const QJsonArray& messages, QJsonArray& decryptedMessages) {
    const unsigned char iv[] = "0123456789012345";
    CipherContext aes256;
        
    for (auto value : messages) {
      if (!EVP_DecryptInit_ex(aes256, EVP_aes_256_cbc(), nullptr, (const unsigned char*)m_cipherKey.constData(), iv)) {
        emit error("DecryptInit error");
        return false;
      }
      QByteArray data(QByteArray::fromBase64(value.toString().toLocal8Bit()));
      QByteArray decodedData;
      decodedData.resize(data.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
      int messageLen = 0;
      if (!EVP_DecryptUpdate(aes256, (unsigned char *)decodedData.data(), &messageLen, (unsigned char*)data.constData(), data.size())) {
        error("DecryptUpdate error");
        return false;
      }
      int finalMessageLen;
      if (!EVP_DecryptFinal_ex(aes256, (unsigned char *)decodedData.data() + messageLen, &finalMessageLen)) {
        error("DecryptFinal error");
        return false;
      }
      decodedData.resize(finalMessageLen);
      QJsonDocument doc(QJsonDocument::fromJson(decodedData));
      if (doc.isNull() || doc.isEmpty()) {
        const QString stringValue(decodedData);
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

private:
  const QString m_url;
  const QString m_uuid;
  QString m_timeToken;
#ifdef Q_PUBNUB_CRYPT
  QByteArray m_cipherKey;
#endif
};
