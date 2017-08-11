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

#ifndef CONNECTIVITY_CPP_ARTIK_HTTP_HH_
#define CONNECTIVITY_CPP_ARTIK_HTTP_HH_

#include <artik_http.h>
#include <artik_module.h>

/*! \file artik_http.hh
 *
 *  \brief C++ Wrapper to the HTTP module
 *
 *  This is a class encapsulation of the C
 *  HTTP module API \ref artik_http.h
 */

namespace artik {
/*!
 *  \brief Http module C++ Class
 */
class Http {
 private:
  artik_http_module* m_module;

 public:
  Http();
  ~Http();

  artik_error get_stream(const char* url, artik_http_headers* headers,
      int *status, artik_http_stream_callback callback, void *user_data,
      artik_ssl_config *ssl);
  artik_error get(const char* url, artik_http_headers* headers, char** response,
      int *status, artik_ssl_config *ssl);
  artik_error post(const char* url, artik_http_headers* headers,
      const char* body, char** response, int *, artik_ssl_config *ssl);
  artik_error put(const char* url, artik_http_headers* headers,
      const char* body, char** response, int *status, artik_ssl_config *ssl);
  artik_error del(const char* url, artik_http_headers* headers, char** response,
      int *status, artik_ssl_config *ssl);
};

}  // namespace artik

#endif  // CONNECTIVITY_CPP_ARTIK_HTTP_HH_
