#pragma once

#include <QNetworkReply>
#include <QJSonArray>

#ifdef Q_PUBNUB_CRYPT
#include "CipherContext.h"
#include <QCryptographicHash>
#endif // Q_PUBNUB_CRYPT

#include "QPubNubBase.h"

class QPubNubSubscription : public QPubNubBase {
  Q_OBJECT
signals:
  void connected();
  void updateReceived(const QJsonArray& messages);
  
public:
  QPubNubSubscription(
    QNetworkAccessManager* networkAccessManager, 
    const QString& subscribeKey, 
    const QString& channel, 
    const QString& uuid = QString(), 
    QObject* parent = nullptr);

#ifdef Q_PUBNUB_CRYPT
  void decryptMessages(const QByteArray& cipherKey);
#endif

public slots:
  void subscribe();

private slots:
  void onError(QNetworkReply::NetworkError code);

  void onSubscribeReadyRead();
  
#ifdef Q_PUBNUB_CRYPT
  bool decrypt(const QJsonArray& messages, QJsonArray& decryptedMessages);
#endif // Q_PUBNUB_CRYPT

private:
  const QString m_url;
  const QString m_uuid;
  QString m_timeToken;
#ifdef Q_PUBNUB_CRYPT
  QByteArray m_cipherKey;
#endif
};
