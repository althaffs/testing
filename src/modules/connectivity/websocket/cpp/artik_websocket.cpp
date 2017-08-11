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

#include "artik_websocket.hh"

artik::Websocket::Websocket() {
  m_module = reinterpret_cast<artik_websocket_module*>(
      artik_request_api_module("websocket"));
}

artik::Websocket::Websocket(const char* uri, artik_ssl_config *ssl_config) {
  this->m_module = reinterpret_cast<artik_websocket_module*>(
      artik_request_api_module("websocket"));
  this->m_handle = NULL;
  this->m_config.uri = strndup(uri, strlen(uri));
  memcpy(&this->m_config.ssl_config, ssl_config,
            sizeof(this->m_config.ssl_config));
}

artik::Websocket::~Websocket() {
  artik_release_api_module(reinterpret_cast<void*>(this->m_module));
}

artik_error artik::Websocket::request() {
  return this->m_module->websocket_request(&this->m_handle, &this->m_config);
}

artik_error artik::Websocket::open_stream() {
  return this->m_module->websocket_open_stream(this->m_handle);
}

artik_error artik::Websocket::set_connection_callback(
    artik_websocket_callback callback, void *user_data) {
  return this->m_module->websocket_set_connection_callback(this->m_handle,
      callback, user_data);
}

artik_error artik::Websocket::write_stream(char* message) {
  return this->m_module->websocket_write_stream(this->m_handle, message);
}

artik_error artik::Websocket::set_receive_callback(
    artik_websocket_callback callback, void *user_data) {
  return this->m_module->websocket_set_receive_callback(this->m_handle,
      callback, user_data);
}

artik_error artik::Websocket::close_stream() {
  return this->m_module->websocket_close_stream(this->m_handle);
}

