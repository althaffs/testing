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


#include "artik_adc.hh"

artik::Adc::Adc(unsigned int pin, char *name) {
  this->m_handle = NULL;
  this->m_module = reinterpret_cast<artik_adc_module*>(
      artik_request_api_module("adc"));
  this->m_config.pin_num = pin;
  if (name)
    this->m_config.name = strndup(name, MAX_NAME_LEN);
}

artik::Adc::Adc(artik_adc_config &config) {
  this->m_handle = NULL;
  this->m_module = reinterpret_cast<artik_adc_module*>(
      artik_request_api_module("adc"));
  this->m_config.pin_num = config.pin_num;
  if (config.name)
    this->m_config.name = strndup(config.name, MAX_NAME_LEN);
}

artik::Adc::Adc(artik::Adc const &val) {
  this->m_module = val.m_module;
  this->m_handle = val.m_handle;
  this->m_config.pin_num = val.m_config.pin_num;
  if (val.m_config.name)
    this->m_config.name = strndup(val.m_config.name, MAX_NAME_LEN);
}

artik::Adc::Adc() {
  this->m_handle = NULL;
  this->m_module = reinterpret_cast<artik_adc_module*>(
      artik_request_api_module("adc"));
  memset(&this->m_config, 0, sizeof(this->m_config));
}

artik::Adc::~Adc() {
  if (this->m_config.name)
    free(this->m_config.name);
  if (this->m_handle)
    this->release();
  artik_release_api_module(reinterpret_cast<void*>(this->m_module));
}

artik::Adc &artik::Adc::operator=(Adc const &val) {
  this->m_module = val.m_module;
  this->m_handle = val.m_handle;
  this->set_name(val.get_name());
  this->set_pin_num(val.get_pin_num());
  return *this;
}

artik_error artik::Adc::request(void) {
  return this->m_module->request(&this->m_handle, &this->m_config);
}

artik_error artik::Adc::release(void) {
  return this->m_module->release(this->m_handle);
}

artik_error artik::Adc::get_value(int *val) {
  return this->m_module->get_value(this->m_handle, val);
}

unsigned int artik::Adc::get_pin_num(void) const {
  return this->m_config.pin_num;
}

char* artik::Adc::get_name(void) const {
  return this->m_config.name;
}

void artik::Adc::set_pin_num(unsigned int val) {
  this->m_config.pin_num = val;
}

void artik::Adc::set_name(char *val) {
  if (this->m_config.name)
    free(this->m_config.name);
  if (val)
    this->m_config.name = strndup(val, MAX_NAME_LEN);
}
