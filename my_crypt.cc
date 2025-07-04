// Copyright 2018 The Ripple Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "my_crypt.h"

#include <openssl/aes.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include "my_dbug.h"

const unsigned char SQL_LOG_CRYPT_MAGIC[] = {0, 0, 0, 0};

Crypto::~Crypto() {
  EVP_CIPHER_CTX_free(ctx);
}

Crypto::Crypto() {
  ctx = EVP_CIPHER_CTX_new();
}

// WARNING: It is allowed to have output == NULL, for special cases like AAD
// support in AES GCM. output_used however must never be NULL.
CryptResult Crypto::Crypt(const unsigned char* input, int input_size,
                          unsigned char* output, int* output_used) {
  DBUG_ASSERT(input != NULL);
  DBUG_ASSERT(output_used != NULL);
  if (input == NULL || input_size == 0) {
    fprintf(stderr, "Crypt input was null or empty, which would reset AES GCM; aborting!\n");
    abort();
  }
  if (!EVP_CipherUpdate(ctx, output, output_used, input, input_size)) {
    char error_buf[1024];
    unsigned long error= ERR_get_error();
    ERR_error_string_n(error, error_buf, sizeof(error_buf));
    fprintf(stderr, "EVP_CipherUpdate failed: %s (%lu)\n", error_buf, error);
    return CRYPT_OPENSSL_ERROR;
  }

  return CRYPT_OK;
}

CryptResult Aes128CtrCrypto::Init(const unsigned char* key,
                                  const unsigned char* iv,
                                  int iv_size) {
  if (iv_size != 16) {
    DBUG_ASSERT(false);
    return CRYPT_BAD_IV;
  }

  if (!EVP_CipherInit_ex(ctx, EVP_aes_128_ctr(), NULL, key, iv, mode())) {
    return CRYPT_OPENSSL_ERROR;
  }

  return CRYPT_OK;
}

CryptResult Aes128CtrEncrypter::Encrypt(const unsigned char* plaintext,
                                        int plaintext_size,
                                        unsigned char* ciphertext,
                                        int* ciphertext_used) {
  CryptResult res = Crypt(plaintext, plaintext_size, ciphertext,
                          ciphertext_used);
  DBUG_ASSERT(*ciphertext_used == plaintext_size);
  return res;
}

CryptResult Aes128CtrDecrypter::Decrypt(const unsigned char* ciphertext,
                                        int ciphertext_size,
                                        unsigned char* plaintext,
                                        int* plaintext_used) {
  CryptResult res = Crypt(ciphertext, ciphertext_size, plaintext,
                          plaintext_used);
  DBUG_ASSERT(*plaintext_used == ciphertext_size);
  return res;
}

CryptResult Aes128GcmCrypto::Init(const unsigned char* key,
                                  const unsigned char* iv,
                                  int iv_size) {
  char error_buf[1024];
  if (!EVP_CipherInit_ex(ctx, EVP_aes_128_gcm(), NULL, NULL, NULL, mode())) {
    unsigned long error= ERR_get_error();
    ERR_error_string_n(error, error_buf, sizeof(error_buf));
    fprintf(stderr, "EVP_CipherInit_ex with EVP_aes_128_gcm failed: %s (%lu)\n", error_buf, error);
    return CRYPT_OPENSSL_ERROR;
  }
  if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv_size, NULL)) {
    unsigned long error= ERR_get_error();
    ERR_error_string_n(error, error_buf, sizeof(error_buf));
    fprintf(stderr, "EVP_CIPHER_CTX_ctrl failed: %s (%lu)\n", error_buf, error);
    return CRYPT_OPENSSL_ERROR;
  }
  if (!EVP_CipherInit_ex(ctx, NULL, NULL, key, iv, mode())) {
    unsigned long error= ERR_get_error();
    ERR_error_string_n(error, error_buf, sizeof(error_buf));
    fprintf(stderr, "EVP_CipherInit_ex with NULL failed: %s (%lu)\n", error_buf, error);
    return CRYPT_OPENSSL_ERROR;
  }
  return CRYPT_OK;
}

CryptResult Aes128GcmCrypto::AddAAD(const unsigned char* aad, int aad_size) {
  int outlen;
  return Crypt(aad, aad_size, NULL, &outlen);
}

CryptResult Aes128GcmEncrypter::Encrypt(const unsigned char* plaintext,
                                        int plaintext_size,
                                        unsigned char* ciphertext,
                                        int* ciphertext_used) {
  CryptResult res = Crypt(plaintext, plaintext_size, ciphertext,
                          ciphertext_used);
  DBUG_ASSERT(*ciphertext_used == plaintext_size);
  return res;
}

CryptResult Aes128GcmEncrypter::GetTag(unsigned char* tag, int tag_size) {
  unsigned char buffer[AES_128_BLOCK_SIZE];
  char error_buf[1024];
  int buffer_used;
  if (!EVP_CipherFinal_ex(ctx, buffer, &buffer_used)) {
    unsigned long error= ERR_get_error();
    ERR_error_string_n(error, error_buf, sizeof(error_buf));
    fprintf(stderr, "EVP_CipherFinal_ex failed: %s (%lu)\n", error_buf, error);
    return CRYPT_OPENSSL_ERROR;
  }
  DBUG_ASSERT(buffer_used == 0);
  if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, tag_size, tag)) {
    unsigned long error= ERR_get_error();
    ERR_error_string_n(error, error_buf, sizeof(error_buf));
    fprintf(stderr, "EVP_CIPHER_CTX_ctrl failed: %s (%lu)\n", error_buf, error);
    return CRYPT_OPENSSL_ERROR;
  }
  return CRYPT_OK;
}

CryptResult Aes128GcmDecrypter::Decrypt(const unsigned char* ciphertext,
                                        int ciphertext_size,
                                        unsigned char* plaintext,
                                        int* plaintext_used) {
  CryptResult res = Crypt(ciphertext, ciphertext_size, plaintext,
                          plaintext_used);
  DBUG_ASSERT(*plaintext_used == ciphertext_size);
  return res;
}

CryptResult Aes128GcmDecrypter::SetTag(const unsigned char* tag, int tag_size) {
  if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, tag_size,
                           (void*) tag)) {
    return CRYPT_OPENSSL_ERROR;
  }
  return CRYPT_OK;
}

CryptResult Aes128GcmDecrypter::CheckTag() {
  unsigned char buffer[AES_128_BLOCK_SIZE];
  int buffer_used;
  if (!EVP_CipherFinal_ex(ctx, buffer, &buffer_used)) {
    return CRYPT_OPENSSL_ERROR;
  }
  DBUG_ASSERT(buffer_used == 0);
  return CRYPT_OK;
}

CryptResult Aes128EcbCrypto::Init(const unsigned char* key) {
  if (!EVP_CipherInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, NULL, mode())) {
    return CRYPT_OPENSSL_ERROR;
  }

  return CRYPT_OK;
}

CryptResult Aes128EcbEncrypter::Encrypt(const unsigned char* plaintext,
                                        int plaintext_size,
                                        unsigned char* ciphertext,
                                        int* ciphertext_used) {
  CryptResult res = Crypt(plaintext, plaintext_size,
                          ciphertext, ciphertext_used);
  DBUG_ASSERT(*ciphertext_used == plaintext_size);
  return res;
}

CryptResult Aes128EcbDecrypter::Decrypt(const unsigned char* ciphertext,
                                        int ciphertext_size,
                                        unsigned char* plaintext,
                                        int* plaintext_used) {
  CryptResult res = Crypt(ciphertext, ciphertext_size,
                          plaintext, plaintext_used);
  DBUG_ASSERT(*plaintext_used == ciphertext_size);
  return res;
}

extern "C" {

CryptResult EncryptAes128Ctr(const unsigned char* key,
                             const unsigned char* iv, int iv_size,
                             const unsigned char* plaintext, int plaintext_size,
                             unsigned char* ciphertext, int* ciphertext_used) {
  Aes128CtrEncrypter encrypter;

  CryptResult res = encrypter.Init(key, iv, iv_size);

  if (res != CRYPT_OK)
    return res;

  return encrypter.Encrypt(plaintext, plaintext_size, ciphertext,
                           ciphertext_used);
}

CryptResult DecryptAes128Ctr(const unsigned char* key,
                             const unsigned char* iv, int iv_size,
                             const unsigned char* ciphertext,
                             int ciphertext_size,
                             unsigned char* plaintext, int* plaintext_used) {
  Aes128CtrDecrypter decrypter;

  CryptResult res = decrypter.Init(key, iv, iv_size);

  if (res != CRYPT_OK)
    return res;

  return decrypter.Decrypt(ciphertext, ciphertext_size,
                           plaintext, plaintext_used);
}

CryptResult EncryptAes128Gcm(const unsigned char* key,
                             const unsigned char* iv, int iv_size,
                             const unsigned char* aad, int aad_size,
                             const unsigned char* plaintext, int plaintext_size,
                             unsigned char* ciphertext, int* ciphertext_used,
                             unsigned char* tag, int tag_size) {
  Aes128GcmEncrypter encrypter;

  CryptResult res = encrypter.Init(key, iv, iv_size);

  if (res != CRYPT_OK)
    return res;

  if (aad != NULL && aad_size > 0) {
    res = encrypter.AddAAD(aad, aad_size);

    if (res != CRYPT_OK)
      return res;
  }

  res = encrypter.Encrypt(plaintext, plaintext_size, ciphertext, ciphertext_used);

  if (res != CRYPT_OK)
    return res;

  return encrypter.GetTag(tag, tag_size);
}

CryptResult DecryptAes128Gcm(const unsigned char* key,
                             const unsigned char* iv, int iv_size,
                             const unsigned char* aad, int aad_size,
                             const unsigned char* ciphertext, int ciphertext_size,
                             unsigned char* plaintext, int* plaintext_used,
                             const unsigned char* expected_tag, int tag_size) {
  Aes128GcmDecrypter decrypter;

  CryptResult res = decrypter.Init(key, iv, iv_size);

  if (res != CRYPT_OK)
    return res;

  res = decrypter.SetTag(expected_tag, tag_size);

  if (res != CRYPT_OK)
    return res;

  if (aad != NULL && aad_size > 0) {
    res = decrypter.AddAAD(aad, aad_size);

    if (res != CRYPT_OK)
      return res;
  }

  res = decrypter.Decrypt(ciphertext, ciphertext_size,
                          plaintext, plaintext_used);

  if (res != CRYPT_OK)
    return res;

  return decrypter.CheckTag();
}

CryptResult EncryptAes128Ecb(const unsigned char* key,
                             const unsigned char* plaintext,
                             int plaintext_size,
                             unsigned char* ciphertext,
                             int* ciphertext_used) {
  Aes128EcbEncrypter encrypter;

  CryptResult res = encrypter.Init(key);

  if (res != CRYPT_OK)
    return res;

  return encrypter.Encrypt(plaintext, plaintext_size,
                           ciphertext, ciphertext_used);
}

CryptResult DecryptAes128Ecb(const unsigned char* key,
                             const unsigned char* ciphertext,
                             int ciphertext_size,
                             unsigned char* plaintext,
                             int* plaintext_used) {
  Aes128EcbDecrypter decrypter;

  CryptResult res = decrypter.Init(key);

  if (res != CRYPT_OK)
    return res;

  return decrypter.Decrypt(ciphertext, ciphertext_size,
                           plaintext, plaintext_used);
}

CryptResult RandomBytes(unsigned char* buf, int num) {
#if 0
  RAND_METHOD* rand = RAND_OpenSSL();
  if (rand == NULL || rand->bytes(buf, num) != 1) {
    return CRYPT_OPENSSL_ERROR;
  }
#endif
  return CRYPT_OK;
}

} // extern "C"
