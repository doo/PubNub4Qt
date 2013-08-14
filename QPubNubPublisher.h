#pragma once

#include <QNetworkReply>

#include "QPubNubBase.h"

class QPubNubPublisher : public QPubNubBase {
  Q_OBJECT
signals:
  void published(const QString& timeToken);
  
public:
  QPubNubPublisher(
    QNetworkAccessManager* networkAccessManager, 
    const QString& publishKey, 
    const QString& subscribeKey, 
    const QString& channel, 
    QObject* parent= nullptr);
  
#ifdef Q_PUBNUB_CRYPT
  void encryptMessages(const QByteArray& cipherKey);
#endif

  void signMessages(const QByteArray& secretKey);

public slots:
  void publish(const QJsonValue& value);

private:
  QByteArray toByteArray(const QJsonValue& value);
  QByteArray signature(const QByteArray& message);
  QString url(const QByteArray& message);

private slots:
  void onError(QNetworkReply::NetworkError code);
  void readyRead();

private:
  const QString m_publishKey;
  const QString m_subscribeKey;
  const QString m_channel;
  QByteArray m_secretKey;
#ifdef Q_PUBNUB_CRYPT
  QByteArray m_cipherKey;
#endif
};
