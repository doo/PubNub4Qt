#pragma once

#include <openssl/evp.h>
#include <openssl/err.h>

class CipherContext {
public:
  CipherContext() {
    EVP_CIPHER_CTX_init(&context);
  }

  ~CipherContext() {
    EVP_CIPHER_CTX_cleanup(&context);
  }

  QByteArray aesDecrypt(const QByteArray& cipherKey, const QByteArray& iv, const QByteArray& message, int& error) {
    return decrypt(EVP_aes_256_cbc(), cipherKey, iv, message, error);
  }

  QByteArray aesEncrypt(const QByteArray& cipherKey, const QByteArray& iv, const QByteArray& message, int& error) {
    return encrypt(EVP_aes_256_cbc(), cipherKey, iv, message, error);
  }

  QByteArray decrypt(const EVP_CIPHER *cipher, const QByteArray& cipherKey, const QByteArray& iv, const QByteArray& message, int& error) {
    return perform(
      EVP_DecryptInit_ex, EVP_DecryptUpdate, EVP_DecryptFinal_ex,
      cipher, cipherKey, iv, message, error);
  }

  QByteArray encrypt(const EVP_CIPHER *cipher, const QByteArray& cipherKey, const QByteArray& iv, const QByteArray& message, int& error) {
    return perform(
      EVP_EncryptInit_ex, EVP_EncryptUpdate, EVP_EncryptFinal_ex,
      cipher, cipherKey, iv, message, error);
  }

  static QByteArray emptyResult(int& error) {
    error = ERR_get_error();
    return QByteArray();
  }

protected:
  QByteArray perform(
    int (*init)(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *cipher, ENGINE *impl, const unsigned char *key, const unsigned char *iv),
    int	(*update)(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl, const unsigned char *in, int inl),
    int	(*final)(EVP_CIPHER_CTX *ctx, unsigned char *outm, int *outl),
    const EVP_CIPHER *cipher, const QByteArray& cipherKey, const QByteArray& iv, const QByteArray& sourceData,
    int& error) {
    error = 0;
    if (!init(&context, cipher, nullptr, (const unsigned char*)cipherKey.constData(), (const unsigned char*)iv.constData())) {
      return emptyResult(error);
    }

    QByteArray resultData;
    resultData.resize(sourceData.size() + EVP_CIPHER_block_size(cipher));
    int len = 0;
    if (!update(&context, (unsigned char *)resultData.data(), &len, (unsigned char*)sourceData.constData(), sourceData.size())) {
      return emptyResult(error);
    }
    int finalLen;
    if (!final(&context, (unsigned char *)resultData.data() + len, &finalLen)) {
      return emptyResult(error);
    }
    resultData.resize(len + finalLen);
    return resultData;
  }

  operator EVP_CIPHER_CTX* () { return &context; }

private:
  EVP_CIPHER_CTX context;
};