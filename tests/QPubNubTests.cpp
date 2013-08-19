#include "QPubNubTests.h"

#include "../QPubNub.h"

QTEST_APPLESS_MAIN(QPubNubTests)

static const QByteArray cipherKey("enigma");
static const QString plainString("Pubnub Messaging API 1");
static const QByteArray cipherString("f42pIQcWZ9zbTbH8cyLwByD/GsviOE0vcREIEVPARR0=");
static const QByteArray cipherObject("TTSJuy0ocG0qx8qrEJCPLPLWIhYgbxcPkB/adFr+Nos=");

void QPubNubTests::should_encrypt_message() {
  QPubNubCrypt crypt(cipherKey);
  int error = 0;
  QCOMPARE(crypt.encrypt(plainString, error), cipherString);
}

void QPubNubTests::should_decrypt_message() {
  QPubNubCrypt crypt(cipherKey);
  int error = 0;
  QCOMPARE(crypt.decrypt(cipherString, error).toString(), plainString);
}

void QPubNubTests::should_not_blow_up_on_empty_array() {
  const QByteArray cipherEmptyArray("Ns4TB41JjT2NCXaGLWSPAQ==");
  QPubNubCrypt crypt(cipherKey);
  int error = 0;
  QCOMPARE(crypt.encrypt(QJsonArray(), error), cipherEmptyArray);
  auto array(crypt.decrypt(cipherEmptyArray, error));
  QVERIFY(array.isArray());
  QCOMPARE(array.toArray().size(), 0);
}

void QPubNubTests::should_decrypt_an_array_with_a_single_message() {
  const QByteArray cipherArray("bz1jaOW3wT/MBPMN8SI0GvWPKT2PUfj2TD/Rg746jSc=");
  QPubNubCrypt crypt(cipherKey);
  int error = 0;
  QJsonArray array;
  array.append(QJsonValue(QString(plainString)));
  QCOMPARE(crypt.encrypt(array, error), cipherArray);
  auto value(crypt.decrypt(cipherArray, error));
  QVERIFY(value.isArray());
  QVERIFY(value.toArray() == array);
}

void QPubNubTests::should_not_blow_up_on_empty_object() {
  const QByteArray cipherEmptyObject("IDjZE9BHSjcX67RddfCYYg==");
  QPubNubCrypt crypt(cipherKey);
  int error = 0;
  QCOMPARE(crypt.encrypt(QJsonObject(), error), cipherEmptyObject);
  auto value(crypt.decrypt(cipherEmptyObject, error));
  QVERIFY(value.isObject());
  QVERIFY(value.toObject() == QJsonObject());
}

void QPubNubTests::should_decrypt_an_object_with_a_single_message() {
  QPubNubCrypt crypt(cipherKey);
  int error = 0;
  QJsonObject testObject,bar;
  bar.insert("bar", QJsonValue(QString("foobar")));
  testObject.insert("foo", bar);
  QCOMPARE(crypt.encrypt(testObject, error), cipherObject);
  auto value(crypt.decrypt(cipherObject, error));
  QVERIFY(value.isObject());
  QVERIFY(value.toObject() == testObject);
}