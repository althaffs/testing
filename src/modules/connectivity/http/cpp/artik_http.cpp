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

#include "artik_http.hh"

artik::Http::Http() {
  m_module = reinterpret_cast<artik_http_module*>(
      artik_request_api_module("http"));
}

artik::Http::~Http() {
  artik_release_api_module(reinterpret_cast<void*>(this->m_module));
}

artik_error artik::Http::get_stream(const char* url,
    artik_http_headers* headers, int *status,
    artik_http_stream_callback callback, void *user_data,
    artik_ssl_config *ssl) {
  return m_module->get_stream(url, headers, status, callback, user_data, ssl);
}

artik_error artik::Http::get(const char* url, artik_http_headers* headers,
  char** response, int *status, artik_ssl_config *ssl) {
  return m_module->get(url, headers, response, status, ssl);
}

artik_error artik::Http::post(const char* url, artik_http_headers* headers,
  const char* body, char** response, int *status, artik_ssl_config *ssl) {
  return m_module->post(url, headers, body, response, status, ssl);
}

artik_error artik::Http::put(const char* url, artik_http_headers* headers,
  const char* body, char** response, int *status, artik_ssl_config *ssl) {
  return m_module->put(url, headers, body, response, status, ssl);
}

artik_error artik::Http::del(const char* url, artik_http_headers* headers,
    char** response, int *status, artik_ssl_config *ssl) {
  return m_module->del(url, headers, response, status, ssl);
}
