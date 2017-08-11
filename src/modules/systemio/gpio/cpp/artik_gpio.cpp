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

#include "artik_gpio.hh"

artik::Gpio::Gpio(artik_gpio_id id, char* name, artik_gpio_dir_t dir,
    artik_gpio_edge_t edge, int initial_value) {
  m_module = reinterpret_cast<artik_gpio_module*>(
      artik_request_api_module("gpio"));
  m_config.id = id;
  if (name)
    m_config.name = strndup(name, MAX_NAME_LEN);
  m_config.dir = dir;
  m_config.edge = edge;
  m_config.initial_value = initial_value;
  m_handle = NULL;
}

artik::Gpio::~Gpio() {
  if (m_handle)
    release();

  free(m_config.name);

  artik_release_api_module(reinterpret_cast<void*>(this->m_module));
}

artik_error artik::Gpio::request() {
  return m_module->request(&m_handle, &m_config);
}

artik_error artik::Gpio::release() {
  artik_error err = m_module->release(m_handle);
  m_handle = NULL;
  return err;
}

int artik::Gpio::read() {
  return m_module->read(m_handle);
}

artik_error artik::Gpio::write(int value) {
  return m_module->write(m_handle, value);
}

char* artik::Gpio::get_name() {
  char *name = m_module->get_name(m_handle);

  return name ? name : m_config.name;
}

artik_gpio_dir_t artik::Gpio::get_direction() {
  artik_gpio_dir_t dir = m_module->get_direction(m_handle);

  return (dir != GPIO_DIR_INVALID) ? dir : m_config.dir;
}

artik_gpio_id artik::Gpio::get_id() {
  artik_gpio_id id = m_module->get_id(m_handle);

  return (static_cast<int>(id) >= 0) ? id : m_config.id;
}

artik_error artik::Gpio::set_change_callback(artik_gpio_callback callback,
    void* user_data) {
  return m_module->set_change_callback(m_handle, callback, user_data);
}

void artik::Gpio::unset_change_callback() {
  return m_module->unset_change_callback(m_handle);
}
