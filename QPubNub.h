#pragma once

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJSonDocument>
#include <QJSonArray>
#include <QJSonValue>
#include <QDebug>
#include <QTimer>

#ifdef PUBNUB_CRYPT
#include <openssl/evp.h>
#include <openssl/sha.h>
#pragma comment(lib, "libeay32")
#endif // PUBNUB_CRYPT

class QPubNubSubscriber : public QObject {
  Q_OBJECT
signals:
  void connected();
  void updateReceived(const QJsonArray& messages);
  void error(const QString& message);

public:
  QPubNubSubscriber(const QString& subscribeKey, const QString& channel, const QString& uuid = QString(), QObject* parent = nullptr) : 
    QObject(parent), 
    m_timeToken("0"),
    m_uuid(uuid),
    m_url(QString("https://pubsub.pubnub.com/subscribe/%1/%2/0/").arg(subscribeKey).arg(channel)) {
  }

public slots:
  void subscribe() {
    QString url(m_url + m_timeToken);
    if (!m_uuid.isEmpty()) {
      url += "?uuid=" + m_uuid;
    }
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "Qt/1.0");
    request.setRawHeader("V", "3.4");
    qDebug() << request.url();
    QNetworkReply* reply = m_networkAccessManager.get(request);
    connect(reply, &QNetworkReply::readyRead, this, &QPubNubSubscriber::onSubscribeReadyRead);
    // This can't be connected using the new syntax, cause the signal and error property have the same name "error"
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
  }

private slots:
  void onError(QNetworkReply::NetworkError code) {
    auto reply = qobject_cast<QNetworkReply*>(sender());
    reply->deleteLater();
    QTimer::singleShot(5000, this, SLOT(subscribe));
    emit error(reply->errorString());
  }

  void onSubscribeReadyRead() {
    auto reply = qobject_cast<QNetworkReply*>(sender());
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      return;
    }

    auto data(reply->readAll());
    qDebug() << data.data();
    const QJsonDocument doc(decrypt(data));
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
            emit updateReceived(messages);
          } else {
            emit connected();
          }
          subscribe();
        } else {
          emit error("Second element of response is not a string");
        }
      } else {
        emit error("First element of response is not an JSON array");
      }
    } else {
      emit error("Response is not an JSON array");
    }
  }

  QJsonDocument decrypt(const QByteArray& rawData) {
#ifdef PUBNUB_CRYPT
  if (m_cipherKey.isEmpty()) {
#endif // PUBNUB_CRYPT
    return QJsonDocument::fromJson(rawData);
#ifdef PUBNUB_CRYPT
  } else {
    EVP_CIPHER_CTX aes256;
    unsigned char iv[] = "0123456789012345";
    EVP_CIPHER_CTX_init(&aes256);
    if (!EVP_DecryptInit_ex(&aes256, EVP_aes_256_cbc(), nullptr, (const unsigned char*)m_cipherKey.constData(), iv)) {
      emit error("DecryptInit error");
      return QJsonDocument();
    }

    QByteArray data(QByteArray::fromBase64(rawData));
    QByteArray decodedData;
    decodedData.reserve(data.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()) + 1);
    int message_len = 0;
    if (!EVP_DecryptUpdate(&aes256, (unsigned char *) decodedData.data(), &message_len, (unsigned char*)data.constData(), data.size())) {
      error("DecryptUpdate error");
      return QJsonDocument();
    }
    int message_flen;
    if (!EVP_DecryptFinal_ex(&aes256, (unsigned char *) decodedData.data() + message_len, &message_flen)) {
      error("DecryptFinal error");
      return QJsonDocument();
    }
    EVP_CIPHER_CTX_cleanup(&aes256);
    message_len += message_flen;
    return QJsonDocument::fromJson(decodedData);
  }
#endif
  }
private:
  const QString m_url;
  const QString m_uuid;
  const QString m_cipherKey;
  QString m_timeToken;
  QNetworkAccessManager m_networkAccessManager;
};

class QPubNubPublisher : public QObject {
  Q_OBJECT
signals:
  void error(const QString& message);

public:
  QPubNubPublisher(const QString& publishKey, const QString& subscribeKey, const QString& channel, QObject* parent= nullptr) : QObject(parent), m_publishKey(publishKey),
    m_url(QString("https://pubsub.pubnub.com/publish/%1/%2/0/%3/0/").arg(publishKey).arg(subscribeKey).arg(channel)) {
  }

public slots:
  void publish(const QString& text) {
    if (m_publishKey.isEmpty()) {
      emit error("No publish key set");
    } else {
      QNetworkRequest request(QUrl(m_url + text));
      QNetworkReply* reply = m_networkAccessManager.get(request);
      // This can't be connected using the new syntax, cause the signal and error property have the same name "error"
      connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
    }
  }

private slots:
  void onError(QNetworkReply::NetworkError code) {
    auto reply = qobject_cast<QNetworkReply*>(sender());
    reply->deleteLater();
    QTimer::singleShot(5000, this, SLOT(subscribe));
    emit error(reply->errorString());
  }

private:
  const QString m_publishKey;
  const QString m_url;
  QNetworkAccessManager m_networkAccessManager;
};
