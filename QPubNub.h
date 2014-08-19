#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QJsonValue>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSet>
#include <QStringList>

#ifdef Q_PUBNUB_CRYPT
#include <QCryptographicHash>
#include <openssl/evp.h>
#include <openssl/err.h>

#include "CipherContext.h"
#endif

static QByteArray toByteArray(const QJsonValue& value) {
  switch (value.type()) {
  case QJsonValue::String: {
    QString jsonString("\"" + value.toString() + "\"");
    return jsonString.toUtf8();
    }
  case QJsonValue::Array:
    return QJsonDocument(value.toArray()).toJson(QJsonDocument::Compact);
  case QJsonValue::Object:
    return QJsonDocument(value.toObject()).toJson(QJsonDocument::Compact);
  default:
    return QByteArray();
  }
}

#ifdef Q_PUBNUB_CRYPT
class QPubNubCrypt {
public:
  QPubNubCrypt(const QByteArray& key) : m_key(QCryptographicHash::hash(key, QCryptographicHash::Sha256).left(32).toHex()) {
  }

  QByteArray encrypt(const QString& value, int& error) {
    return encrypt(QJsonValue(value), error);
  }

  QByteArray encrypt(const QJsonValue& value, int& error) {
    switch (value.type()) {
    case QJsonValue::Array:
    case QJsonValue::Object:
    case QJsonValue::String:
      return aesEncrypt(toByteArray(value), error);
    }
    Q_ASSERT_X(false, "encrypt", "Only array, object and string values allowed");
    error = -1;
    return QByteArray();
  }

  QJsonValue decrypt(const QByteArray& source, int& error) {
    QByteArray decrypted(m_cipherContext.aesDecrypt(m_key, iv(), QByteArray::fromBase64(source), error));
    QJsonDocument doc(QJsonDocument::fromJson(decrypted));
    if (doc.isArray()) {
      return doc.array();
    } else if (doc.isObject()) {
      return doc.object();
    } else /*if (doc.isNull() || doc.isEmpty())*/ {
      QString stringValue(decrypted);
      if (stringValue.isEmpty()) {
        return QJsonValue();
      } else {
        // Chop of the surrounding ""
        stringValue = stringValue.mid(1, stringValue.length() - 2);
        // Try if its a number first
        bool ok;
        const double numberValue = stringValue.toDouble(&ok);
        if (ok) {
          return QJsonValue(numberValue);
        } else {
          return QJsonValue(stringValue);
        }
      }
    }
  }

private:
  QByteArray aesEncrypt(const QByteArray& source, int& error) {
    return m_cipherContext.aesEncrypt(m_key, iv(), source, error).toBase64();
  }
  
  static const QByteArray& iv() {
    static const QByteArray _iv("0123456789012345");
    return _iv;
  }

  CipherContext m_cipherContext;
  QByteArray m_key;
};
#endif // Q_PUBNUB_CRYPT

class QPubNub : public QObject {
  Q_OBJECT

  Q_PROPERTY(QByteArray cipherKey READ cipherKey WRITE setCipherKey)
  Q_PROPERTY(QByteArray secretKey MEMBER m_secretKey)
  Q_PROPERTY(QByteArray subscribeKey MEMBER m_subscribeKey READ subscribeKey WRITE setSubscribeKey)
  Q_PROPERTY(QByteArray publishKey MEMBER m_publishKey READ publishKey WRITE setPublishKey)
  Q_PROPERTY(QString uuid MEMBER m_uuid)

  Q_PROPERTY(bool resumeOnReconnect MEMBER m_resumeOnReconnect)
  Q_PROPERTY(QString origin MEMBER m_origin RESET resetOrigin)
  Q_PROPERTY(bool ssl MEMBER m_ssl)
Q_SIGNALS:
  void connected();
  void error(QString message, const int code) const;
  void message(QJsonValue value, QString timeToke, QString channel);
  void timeResponse(quint64 timeToken);
  void published(QString timeStamp);
  void trace(QString message) const;
public:
  QPubNub(QNetworkAccessManager* networkAccessManager, QObject* parent = 0);

  void time();
  void publish(const QString& channel, const QJsonValue& value);
  void subscribe(const QString& channel);
  /*void here_now();
  void presence();
  void history();
  void leave();*/

  QByteArray cipherKey() const { return m_cipherKey; }
  void setCipherKey(const QByteArray& value) { m_cipherKey = value; }

  QByteArray publishKey() const { return m_publishKey; }
  void setPublishKey(const QByteArray& value) { m_publishKey = value; }

  QByteArray subscribeKey() const { return m_subscribeKey; }
  void setSubscribeKey(const QByteArray& value) { m_subscribeKey = value; }
  
protected:
  QNetworkReply* sendRequest(const QString& path);

  bool handleResponse(QNetworkReply* reply, QJsonArray& response) const;

  void resetOrigin();

  QByteArray signature(const QByteArray& message, const QString& channel) const;
  QString baseUrl() const;
  QString publishUrl(const QByteArray& message, const QString& channel) const;
  QString subscribeUrl(const QString& channel) const;

  void connectNotify(const QMetaMethod& signal);
  void disconnectNotify(const QMetaMethod& signal);

private Q_SLOTS:
  void onTimeFinished();
  void publishFinished();
  void onSubscribeReadyRead();
  void onError(QNetworkReply::NetworkError);

  void subscribe();

#ifdef Q_PUBNUB_CRYPT
  bool decrypt(const QJsonArray& messages, const QStringList& channels);
#endif

private:
  QNetworkAccessManager* m_networkAccessManager;
  QByteArray m_cipherKey;
  QByteArray m_subscribeKey;
  QByteArray m_publishKey;
  QByteArray m_secretKey;
  QString m_origin;
  bool m_resumeOnReconnect;
  bool m_ssl;
  QString m_timeToken;
  QString m_uuid;
  QSet<QString> m_channels;
  QString m_channelUrlPart;
  int m_trace;
};
