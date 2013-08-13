#pragma once

#include <openssl/evp.h>

class CipherContext  {
public:
  CipherContext() {
    EVP_CIPHER_CTX_init(&context);
  }

  ~CipherContext() {
    EVP_CIPHER_CTX_cleanup(&context);
  }

  operator EVP_CIPHER_CTX* () { return &context; }

private:
  EVP_CIPHER_CTX context;
};