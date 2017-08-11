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

#ifndef MQTT_CPP_ARTIK_MQTT_HH_
#define MQTT_CPP_ARTIK_MQTT_HH_

#include <string.h>
#include <stdlib.h>

#include <artik_module.h>
#include <artik_mqtt.h>

/*! \file artik_mqtt.hh
 *
 *  \brief C++ Wrapper to the MQTT module
 *
 *  This is a class encapsulation of the C
 *  MQTT module API \ref artik_mqtt.h
 */

namespace artik {
/*!
 *  \brief Mqtt module C++ Class
 */
class Mqtt {
 private:
  artik_mqtt_module *m_module;
  artik_mqtt_module *m_loop;
  artik_mqtt_config m_config;
  artik_mqtt_handle m_client;

 public:
  explicit Mqtt(artik_mqtt_config const &config);
  ~Mqtt();

  artik_mqtt_config &config(void);
  artik_mqtt_config const &config(void) const;

  artik_error set_willmsg(const char *willtopic, const char *willmsg, int qos,
      bool retain);
  artik_error free_willmsg(void);
  artik_error clear_willmsg(void);

  artik_error set_connect(connect_callback cb, void *data);
  artik_error set_disconnect(disconnect_callback cb, void *data);
  artik_error set_subscribe(subscribe_callback cb, void *data);
  artik_error set_unsubscribe(unsubscribe_callback cb, void *data);
  artik_error set_publish(publish_callback cb, void *data);
  artik_error set_message(message_callback cb, void *data);

  artik_error connect(const char *host, int port);
  artik_error disconnect(void);
  artik_error subscribe(int qos, const char *msgtopic);
  artik_error unsubscribe(const char *msgtopic);
  artik_error publish(int qos, bool retain, const char *msg_topic,
      int payload_len, const char *msg_content);
};

}  // namespace artik

#endif  // MQTT_CPP_ARTIK_MQTT_HH_
