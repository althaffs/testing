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


#include "artik_wifi.hh"

artik::Wifi::Wifi() {
  m_module = reinterpret_cast<artik_wifi_module*>(
      artik_request_api_module("wifi"));
}

artik::Wifi::Wifi(Wifi const &val) {
  this->m_module = val.m_module;
}

artik::Wifi::~Wifi() {
  artik_release_api_module(reinterpret_cast<void*>(this->m_module));
}

artik::Wifi &artik::Wifi::operator=(Wifi const &val) {
  this->m_module = val.m_module;
  return *this;
}

artik_error artik::Wifi::disconnect() {
  return m_module->disconnect();
}

artik_error artik::Wifi::init(artik_wifi_mode_t mode) {
  return m_module->init(mode);
}

artik_error artik::Wifi::deinit() {
  return m_module->deinit();
}

artik_error artik::Wifi::scan_request() {
  return m_module->scan_request();
}

artik_error artik::Wifi::get_scan_result(artik_wifi_ap** aps, int* num_aps) {
  return m_module->get_scan_result(aps, num_aps);
}

artik_error artik::Wifi::connect(const char* ssid, const char* password,
    bool persistent) {
  return m_module->connect(ssid, password, persistent);
}

artik_error artik::Wifi::set_scan_result_callback(
    artik_wifi_callback user_callback, void* user_data) {
  return m_module->set_scan_result_callback(user_callback, user_data);
}

artik_error artik::Wifi::set_connect_callback(
    artik_wifi_callback user_callback, void* user_data) {
  return m_module->set_connect_callback(user_callback, user_data);
}

artik_error artik::Wifi::start_ap(const char *ssid, const char *passphrase,
    unsigned int channel, unsigned int encryption_flags) {
  return m_module->start_ap(ssid, passphrase, channel, encryption_flags);
}
