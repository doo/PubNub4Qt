#pragma once

#include <QTest>

class QPubNubTests : public QObject {
  Q_OBJECT

private slots:
  void should_encrypt_message();
  void should_decrypt_message();
  void should_not_blow_up_on_empty_array();
  void should_decrypt_an_array_with_a_single_message();
  void should_not_blow_up_on_empty_object();
  void should_decrypt_an_object_with_a_single_message();
};