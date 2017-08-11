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

#include "artik_mqtt.hh"
#include "artik_error.hh"

artik::Mqtt::Mqtt(artik_mqtt_config const &config)
    : m_config(config) {
  artik_error err = S_OK;

  m_module = reinterpret_cast<artik_mqtt_module*>(
      artik_request_api_module("mqtt"));

  if (!m_module)
    artik_throw(artik::ArtikSupportException());

  if ((err = m_module->create_client(&m_client, &m_config)) != S_OK)
    artik_throw(artik::ArtikInitException());

  m_config.handle = this;
}

artik::Mqtt::~Mqtt() {
  m_module->destroy_client(m_client);
  artik_release_api_module(reinterpret_cast<void*>(m_module));
}

artik_mqtt_config &artik::Mqtt::config(void) {
  return m_config;
}

artik_mqtt_config const &artik::Mqtt::config(void) const {
  return m_config;
}

artik_error artik::Mqtt::set_willmsg(const char *willtopic, const char *willmsg,
    int qos, bool retain) {
  return m_module->set_willmsg(&m_config, willtopic, willmsg, qos, retain);
}

artik_error artik::Mqtt::free_willmsg(void) {
  return m_module->free_willmsg(&m_config);
}

artik_error artik::Mqtt::clear_willmsg(void) {
  return m_module->clear_willmsg(m_client);
}

artik_error artik::Mqtt::set_connect(connect_callback cb, void *data) {
  return m_module->set_connect(m_client, cb, data);
}

artik_error artik::Mqtt::set_disconnect(disconnect_callback cb, void *data) {
  return m_module->set_disconnect(m_client, cb, data);
}

artik_error artik::Mqtt::set_subscribe(subscribe_callback cb, void *data) {
  return m_module->set_subscribe(m_client, cb, data);
}

artik_error artik::Mqtt::set_unsubscribe(unsubscribe_callback cb, void *data) {
  return m_module->set_unsubscribe(m_client, cb, data);
}

artik_error artik::Mqtt::set_publish(publish_callback cb, void *data) {
  return m_module->set_publish(m_client, cb, data);
}

artik_error artik::Mqtt::set_message(message_callback cb, void *data) {
  return m_module->set_message(m_client, cb, data);
}

artik_error artik::Mqtt::connect(const char *host, int port) {
  return m_module->connect(m_client, host, port);
}

artik_error artik::Mqtt::disconnect() {
  return m_module->disconnect(m_client);
}

artik_error artik::Mqtt::subscribe(int qos, const char *msgtopic) {
  return m_module->subscribe(m_client, qos, msgtopic);
}

artik_error artik::Mqtt::unsubscribe(const char *msgtopic) {
  return m_module->unsubscribe(m_client, msgtopic);
}

artik_error artik::Mqtt::publish(int qos, bool retain, const char *msg_topic,
    int payload_len, const char *msg_content) {
  return m_module->publish(m_client, qos, retain, msg_topic, payload_len,
      msg_content);
}
