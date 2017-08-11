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

#include "artik_i2c.hh"

artik::I2c::I2c(artik_i2c_id id, int frequency,
    artik_i2c_wordsize_t wordsize, unsigned int address) {
  m_module = reinterpret_cast<artik_i2c_module*>(
      artik_request_api_module("i2c"));
  m_config.id = id;
  m_config.frequency = frequency;
  m_config.wordsize = wordsize;
  m_config.address = address;
  m_handle = NULL;
}

artik::I2c::~I2c() {
  if (m_handle)
    release();

  artik_release_api_module(reinterpret_cast<void*>(this->m_module));
}

artik_error artik::I2c::request() {
  return m_module->request(&m_handle, &m_config);
}

artik_error artik::I2c::release() {
  artik_error err = m_module->release(m_handle);
  m_handle = NULL;
  return err;
}

int artik::I2c::read(char* buf, int len) {
  return m_module->read(m_handle, buf, len);
}

artik_error artik::I2c::write(char* buf, int len) {
  return m_module->write(m_handle, buf, len);
}

int artik::I2c::read_register(unsigned int addr, char* buf, int len) {
  return m_module->read_register(m_handle, addr, buf, len);
}

artik_error artik::I2c::write_register(unsigned int addr, char* buf, int len) {
  return m_module->write_register(m_handle, addr, buf, len);
}
