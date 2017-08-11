/*
 *
 * Copyright 2017 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 */

#include <artik_security.hh>

#include <iostream>
#include <iomanip>
#include <algorithm>

int main(int ac, char **av) {
  try {
    artik::Security security;
    char    *cert = NULL;
    char    *key = NULL;
    unsigned char sn[ARTIK_CERT_SN_MAXLEN];
    unsigned int  lenSN = ARTIK_CERT_SN_MAXLEN;
    unsigned char randbytes[32];
    artik_error res = S_OK;

    memset(sn, 0, ARTIK_CERT_SN_MAXLEN);
    res = security.get_certificate_sn(sn, &lenSN);
    if (res == S_OK) {
      std::cout << "Certificate Serial Number: " << std::endl;
      std::cout << std::setfill('0');
      for (unsigned int i = 0; i < lenSN; ++i)
        std::cout << std::setw(2) << std::hex  << sn[i];
      std::cout << std::endl;
    } else {
      std::cout << "Unable to get the serial number of the certificate."
          << std::endl;
    }
    res = security.get_certificate(&cert);
    if ( res == S_OK)
      std::cout << "Certificate: " << std::endl << cert << std::endl;
    else
      std::cout << "Unable to get certificate." << std::endl;

    res = security.get_key_from_cert(cert, &key);
    if (res == S_OK)
      std::cout << "Key: " << std::endl << key << std::endl;
    else
      std::cout << "Unable to get key of certificate." << std::endl;

    res = security.get_random_bytes(randbytes, 32);
    if (res == S_OK) {
      std::cout << "Random bytes: " << std::endl;
      for (int i = 0; i < 32; ++i) {
        std::cout << " " << randbytes[i];
      }
      std::cout << std::endl;
    } else {
      std::cout << "Unable to generate random bytes." << std::endl;
    }
  } catch (artik::ArtikException &e) {
    std::cout << "[Exception]" << e.what() << std::endl;
  }
  return 0;
}
