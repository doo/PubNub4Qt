#pragma once

#include <QNetworkReply>
#include <QDebug>
#include <QTimer>
#include <QCryptographicHash>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#ifdef Q_PUBNUB_CRYPT
#include "CipherContext.h"
#endif

#include "QPubNubBase.h"

class QPubNubPublisher : public QPubNubBase {
  Q_OBJECT
signals:
  void published(const QString& timeToken);
  
public:
  QPubNubPublisher(QNetworkAccessManager* networkAccessManager, const QString& publishKey, const QString& subscribeKey, const QString& channel, QObject* parent= nullptr) 
    : QPubNubBase(networkAccessManager, parent), 
    m_publishKey(publishKey), 
    m_subscribeKey(subscribeKey), 
    m_channel(channel) {
  }
  
#ifdef Q_PUBNUB_CRYPT
  void encryptMessages(const QByteArray& cipherKey) {
    m_cipherKey = QCryptographicHash::hash(cipherKey, QCryptographicHash::Sha256).toHex();
  }
#endif

  void signMessages(const QByteArray& secretKey) {
    m_secretKey = secretKey;
  }

public slots:
  void publish(const QJsonValue& value) {
    if (m_publishKey.isEmpty()) {
      emit error("No publish key set");
    } else {
      QByteArray message(toByteArray(value));
#ifdef Q_PUBNUB_CRYPT
      if (!m_cipherKey.isEmpty()) {
        const unsigned char iv[] = "0123456789012345";
        CipherContext aes256;
        if (!EVP_EncryptInit_ex(aes256, EVP_aes_256_cbc(), nullptr, (const unsigned char*)m_cipherKey.constData(), iv)) {
          emit error("EncryptInit error");
          return;
        }

        QByteArray cipher_data;
        cipher_data.resize(message.length() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
        int cipherLen = 0;
        if (!EVP_EncryptUpdate(aes256, (unsigned char *)cipher_data.data(), &cipherLen, (unsigned char *)message.constData(), message.length())) {
          emit error("EncryptUpdate error");
          return;
        }
        int finalCipherLen;
        if (!EVP_EncryptFinal_ex(aes256, (unsigned char*)cipher_data.data() + cipherLen, &finalCipherLen)) {
          emit error("EncryptFinal error");
          return ;
        }
        cipher_data.resize(finalCipherLen);
        message = QString("\"" + QString(cipher_data.toBase64()) + "\"").toLocal8Bit();
      }
#endif // Q_PUBNUB_CRYPT
      auto reply = sendRequest(url(message));
      // This can't be connected using the new syntax, cause the signal and error property have the same name "error"
      connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
      connect(reply, &QNetworkReply::readyRead, this, &QPubNubPublisher::readyRead);
    }
  }

private:
  QByteArray toByteArray(const QJsonValue& value) {
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

  QByteArray signature(const QByteArray& message) {
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(m_publishKey.toLatin1());
    hash.addData("/", 1);
    hash.addData(m_subscribeKey.toLatin1());
    hash.addData("/", 1);
    hash.addData(m_secretKey);
    hash.addData("/", 1);
    hash.addData(m_channel.toLatin1());
    hash.addData("/", 1);
    hash.addData(message);
    hash.addData("", 1);
    return hash.result().toHex();
  }

  QString url(const QByteArray& message) {
    return QString("https://pubsub.pubnub.com/publish/%1/%2/%3/%4/0/%5")
      .arg(m_publishKey)
      .arg(m_subscribeKey)
      .arg(m_secretKey.isEmpty() ? "0" : signature(message).constData())
      .arg(m_channel)
      .arg(message.constData());
  }

private slots:
  void onError(QNetworkReply::NetworkError code) {
    auto reply = qobject_cast<QNetworkReply*>(sender());
    reply->deleteLater();
    emit error(reply->errorString());
  }

  void readyRead() {
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
          emit error(array[1].toString());
        } else {
          emit published(array[2].toString());
        }
      } else {
        emit error("Response array is too small");
      }
    } else {
      emit error("Response is not an JSON array");
    }
  }

private:
  const QString m_publishKey;
  const QString m_subscribeKey;
  const QString m_channel;
  QByteArray m_secretKey;
#ifdef Q_PUBNUB_CRYPT
  QByteArray m_cipherKey;
#endif
};
