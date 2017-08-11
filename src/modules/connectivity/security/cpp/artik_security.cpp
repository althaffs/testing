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

#include "artik_security.hh"

artik::Security::Security() {
  m_module = reinterpret_cast<artik_security_module*>(
      artik_request_api_module("security"));
  if (!m_module || m_module->request(&m_handle) != S_OK)
    artik_throw(artik::ArtikInitException());
}

artik::Security::~Security() {
  m_module->release(m_handle);
}

artik_error artik::Security::get_certificate(char **cert) {
  if (!m_handle)
    return E_NOT_INITIALIZED;
  return m_module->get_certificate(m_handle, cert);
}

artik_error artik::Security::get_key_from_cert(const char *cert, char **key) {
  if (!m_handle)
    return E_NOT_INITIALIZED;
  return m_module->get_key_from_cert(m_handle, cert, key);
}

artik_error artik::Security::get_random_bytes(unsigned char *rand, int len) {
  if (!m_handle)
    return E_NOT_INITIALIZED;
  return m_module->get_random_bytes(m_handle, rand, len);
}

artik_error artik::Security::get_certificate_sn(unsigned char *sn,
    unsigned int *len) {
  if (!m_handle)
    return E_NOT_INITIALIZED;
  return m_module->get_certificate_sn(m_handle, sn, len);
}
