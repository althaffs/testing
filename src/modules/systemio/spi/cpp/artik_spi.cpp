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

#include "artik_spi.hh"

artik::Spi::Spi(unsigned int bus, unsigned int cs, artik_spi_mode mode,
    unsigned int bits_per_word, unsigned int speed) {
  m_module = reinterpret_cast<artik_spi_module*>(
      artik_request_api_module("spi"));
  m_config.bus = bus;
  m_config.cs = cs;
  m_config.mode = mode;
  m_config.bits_per_word = bits_per_word;
  m_config.max_speed = speed;
  m_handle = NULL;
}

artik::Spi::~Spi() {
  if (m_handle)
    release();

  artik_release_api_module(reinterpret_cast<void*>(this->m_module));
}

artik_error artik::Spi::request() {
  return m_module->request(&m_handle, &m_config);
}

artik_error artik::Spi::release() {
  artik_error err = m_module->release(m_handle);
  m_handle = NULL;
  return err;
}

artik_error artik::Spi::read(char* buf, int len) {
  return m_module->read(m_handle, buf, len);
}

artik_error artik::Spi::write(char* buf, int len) {
  return m_module->write(m_handle, buf, len);
}

artik_error artik::Spi::read_write(char* tx_buf, char* rx_buf, int len) {
  return m_module->read_write(m_handle, tx_buf, rx_buf, len);
}

