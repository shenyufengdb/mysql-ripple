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


#include "my_crypt_key_management.h"

#include <arpa/inet.h>
#include <cstring>

#include "my_thread.h"

my_bool opt_danger_danger_use_dbug_keys = 0;

#ifndef DBUG_OFF
#ifdef HAVE_PSI_INTERFACE
PSI_rwlock_key key_LOCK_dbug_crypto_key_version;
#endif
mysql_rwlock_t LOCK_dbug_crypto_key_version;
unsigned int opt_danger_danger_dbug_crypto_key_version = 0;
#endif

/**
 * Default functions
 */
unsigned int GetLatestCryptoKeyVersionImpl();
int GetCryptoKeyImpl(unsigned int version, unsigned char* key_buffer,
                     unsigned int size);

/**
 * Function pointers for
 * - GetLatestCryptoKeyVersion
 * - GetCryptoKey
 */
static
struct CryptoKeyFuncs_t cryptoKeyFuncs = {
  GetLatestCryptoKeyVersionImpl,
  GetCryptoKeyImpl
};

extern "C"
unsigned int GetLatestCryptoKeyVersion() {
#ifndef DBUG_OFF
  if (opt_danger_danger_use_dbug_keys) {
    mysql_rwlock_rdlock(&LOCK_dbug_crypto_key_version);
    unsigned int res = opt_danger_danger_dbug_crypto_key_version;
    mysql_rwlock_unlock(&LOCK_dbug_crypto_key_version);
    return res;
  }
#endif

  return (* cryptoKeyFuncs.getLatestCryptoKeyVersionFunc)();
}

extern "C"
int GetCryptoKey(unsigned int version, unsigned char* key, unsigned int size) {
  if (opt_danger_danger_use_dbug_keys) {
    memset(key, 0, size);
    // Just don't support tiny keys, no point anyway.
    if (size < sizeof(version)) {
      return 1;
    }
    version = htonl(version);
    memcpy(key, &version, sizeof(version));
    return 0;
  }

  return (* cryptoKeyFuncs.getCryptoKeyFunc)(version, key, size);
}

extern "C"
void
InstallCryptoKeyFunctions(const struct CryptoKeyFuncs_t* _cryptoKeyFuncs)
{
  if (_cryptoKeyFuncs == NULL)
  {
    /* restore defaults when called with NULL argument */
    cryptoKeyFuncs.getLatestCryptoKeyVersionFunc =
        GetLatestCryptoKeyVersionImpl;
    cryptoKeyFuncs.getCryptoKeyFunc =
        GetCryptoKeyImpl;
  }
  else
  {
    cryptoKeyFuncs = *_cryptoKeyFuncs;
  }
}
