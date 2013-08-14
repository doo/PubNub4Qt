#pragma once

#include <QNetworkReply>
#include <QNetworkAccessManager>

class QPubNubBase : public QObject {
  Q_OBJECT

signals:
  void error(const QString& message) const;

protected:
  QPubNubBase(QNetworkAccessManager* networkAccessManager, QObject* parent = nullptr);

  QNetworkReply* sendRequest(QString& url);

  bool handleResponse(QNetworkReply* reply) const;

private:
  QNetworkAccessManager* m_networkAccessManager;
};