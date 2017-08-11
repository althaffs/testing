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

#include "artik_serial.hh"

#include <unistd.h>

artik::Serial::Serial(unsigned int port_, char *name_,
    artik_serial_baudrate_t baudrate_, artik_serial_parity_t parity_,
    artik_serial_data_bits_t data_, artik_serial_stop_bits_t stop_,
    artik_serial_flowcontrol_t flowctrl_) {
  this->m_handle = NULL;
  this->m_module = reinterpret_cast<artik_serial_module*>(
      artik_request_api_module("serial"));
  this->m_config.port_num = port_;
  this->m_config.name = strdup(name_);
  this->m_config.baudrate = baudrate_;
  this->m_config.parity = parity_;
  this->m_config.data_bits = data_;
  this->m_config.stop_bits = stop_;
  this->m_config.flowctrl = flowctrl_;
}

artik::Serial::Serial(artik_serial_config &config) {
  this->m_handle = NULL;
  this->m_module = reinterpret_cast<artik_serial_module*>(
      artik_request_api_module("serial"));
  memcpy(&this->m_config, &config, sizeof(this->m_config));
}

artik::Serial::Serial(Serial const &val) {
  this->m_module = val.m_module;
  this->m_handle = val.m_handle;
  memcpy(&this->m_config, &val.m_config, sizeof(this->m_config));
}

artik::Serial::Serial() {
  this->m_handle = NULL;
  this->m_module = reinterpret_cast<artik_serial_module*>(
      artik_request_api_module("serial"));
}

artik::Serial::~Serial() {
  if (this->m_handle)
    this->release();

  artik_release_api_module(reinterpret_cast<void*>(this->m_module));
}

artik::Serial &artik::Serial::operator=(Serial const &val) {
  this->set_port_num(val.get_port_num());
  this->set_name(val.get_name());
  this->set_baudrate(val.get_baudrate());
  this->set_parity(val.get_parity());
  this->set_data_bits(val.get_data_bits());
  this->set_stop_bits(val.get_stop_bits());
  this->set_flowctrl(val.get_flowctrl());
  return *this;
}

artik_error artik::Serial::request(void) {
  return this->m_module->request(&this->m_handle, &this->m_config);
}

artik_error artik::Serial::release(void) {
  return this->m_module->release(this->m_handle);
}

artik_error artik::Serial::read(unsigned char* buf, int* len) {
  return this->m_module->read(this->m_handle, buf, len);
}

artik_error artik::Serial::write(unsigned char* buf, int* len) {
  return this->m_module->write(this->m_handle, buf, len);
}

artik_error artik::Serial::set_received_callback(artik_serial_callback callback,
    void *data) {
  return m_module->set_received_callback(this->m_handle, callback,
      (data ? data : this->m_handle));
}

artik_error artik::Serial::unset_received_callback(void) {
  return m_module->unset_received_callback(this->m_handle);
}

unsigned int artik::Serial::get_port_num(void) const {
  return this->m_config.port_num;
}

char* artik::Serial::get_name(void) const {
  return this->m_config.name;
}

artik_serial_baudrate_t artik::Serial::get_baudrate(void) const {
  return m_config.baudrate;
}

artik_serial_parity_t artik::Serial::get_parity(void) const {
  return m_config.parity;
}

artik_serial_data_bits_t artik::Serial::get_data_bits(void) const {
  return m_config.data_bits;
}

artik_serial_stop_bits_t artik::Serial::get_stop_bits(void) const {
  return m_config.stop_bits;
}

artik_serial_flowcontrol_t artik::Serial::get_flowctrl(void) const {
  return m_config.flowctrl;
}

void artik::Serial::set_port_num(unsigned int val) {
  this->m_config.port_num = val;
}

void artik::Serial::set_name(char* val) {
  if (this->m_config.name != NULL)
    free(this->m_config.name);
  this->m_config.name = strdup(val);
}

void artik::Serial::set_baudrate(artik_serial_baudrate_t val) {
  this->m_config.baudrate = val;
}

void artik::Serial::set_parity(artik_serial_parity_t val) {
  this->m_config.parity = val;
}

void artik::Serial::set_data_bits(artik_serial_data_bits_t val) {
  this->m_config.data_bits = val;
}

void artik::Serial::set_stop_bits(artik_serial_stop_bits_t val) {
  this->m_config.stop_bits = val;
}

void artik::Serial::set_flowctrl(artik_serial_flowcontrol_t val) {
  this->m_config.flowctrl = val;
}
