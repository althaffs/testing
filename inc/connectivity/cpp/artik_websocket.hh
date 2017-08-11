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

#ifndef CONNECTIVITY_CPP_ARTIK_WEBSOCKET_HH_
#define CONNECTIVITY_CPP_ARTIK_WEBSOCKET_HH_

#include <string.h>
#include <stdlib.h>

#include <artik_module.h>
#include <artik_websocket.h>

/*! \file artik_websocket.hh
 *
 *  \brief C++ Wrapper to the WEBSOCKET module
 *
 *  This is a class encapsulation of the C
 *  WEBSOCKET module API \ref artik_websocket.h
 */

namespace artik {
/*!
 *  \brief Websocket module C++ Class
 */
class Websocket {
 private:
  artik_websocket_module* m_module;
  artik_websocket_config m_config;
  artik_websocket_handle m_handle;

 public:
  Websocket();
  Websocket(const char* uri, artik_ssl_config *ssl_config);
  ~Websocket();

  artik_error request();
  artik_error open_stream();
  artik_error set_connection_callback(artik_websocket_callback callback,
      void *user_data);
  artik_error write_stream(char* message);
  artik_error set_receive_callback(artik_websocket_callback callback,
      void *user_data);
  artik_error close_stream();
};

}  // namespace artik

#endif  // CONNECTIVITY_CPP_ARTIK_WEBSOCKET_HH_
