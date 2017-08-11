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

#ifndef WIFI_CPP_ARTIK_WIFI_HH_
#define WIFI_CPP_ARTIK_WIFI_HH_

#include <artik_module.h>
#include <artik_wifi.h>

/*! \file artik_wifi.hh
 *  \brief C++ Wrapper to the Wi-Fi module
 *
 *  This is a class encapsulation of the C
 *  Wi-Fi module API \ref artik_wifi.h
 */

namespace artik {
/*!
 *  \brief Wi-Fi module C++ Class
 */
class Wifi {
 private:
  artik_wifi_module* m_module;

 public:
  Wifi();
  Wifi(Wifi const &);
  ~Wifi();

  Wifi &operator=(Wifi const &);

 public:
  artik_error disconnect();
  artik_error init(artik_wifi_mode_t mode);
  artik_error deinit();
  artik_error scan_request();
  artik_error get_scan_result(artik_wifi_ap** aps, int* num_aps);
  artik_error start_ap(const char *ssid, const char *passphrase,
      unsigned int channel, unsigned int encryption_flags = 0);
  artik_error connect(const char* ssid, const char* password, bool persistent);
  artik_error set_scan_result_callback(artik_wifi_callback user_callback,
      void* user_data);
  artik_error set_connect_callback(artik_wifi_callback user_callback,
      void* user_data);
};

}  // namespace artik

#endif  // WIFI_CPP_ARTIK_WIFI_HH_
