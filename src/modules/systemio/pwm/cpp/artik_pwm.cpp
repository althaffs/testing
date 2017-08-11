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


#include "artik_pwm.hh"

artik::Pwm::Pwm(unsigned int pin, char *name, unsigned int period,
    artik_pwm_polarity_t polarity, unsigned int duty_cycle) {
  this->m_handle = NULL;
  this->m_module = reinterpret_cast<artik_pwm_module*>(
      artik_request_api_module("pwm"));
  this->m_config.pin_num = pin;
  this->m_config.name = strdup(name);
  this->m_config.period = period;
  this->m_config.polarity = polarity;
  this->m_config.duty_cycle = duty_cycle;
}

artik::Pwm::Pwm(artik_pwm_config &config) {
  this->m_handle = NULL;
  this->m_module = reinterpret_cast<artik_pwm_module*>(
      artik_request_api_module("pwm"));
  memcpy(&this->m_config, &config, sizeof(this->m_config));
}

artik::Pwm::Pwm(artik::Pwm const &val) {
  this->m_module = val.m_module;
  this->m_handle = val.m_handle;
  memcpy(&this->m_config, &val.m_config, sizeof(this->m_config));
}

artik::Pwm::Pwm() {
  this->m_handle = NULL;
  this->m_module = reinterpret_cast<artik_pwm_module*>(
      artik_request_api_module("pwm"));
}

artik::Pwm::~Pwm() {
  if (this->m_config.name)
    free(this->m_config.name);
  if (this->m_handle)
    this->release();

  artik_release_api_module(reinterpret_cast<void*>(this->m_module));
}

artik::Pwm &artik::Pwm::operator=(Pwm const &val) {
  this->m_module = val.m_module;
  this->m_handle = val.m_handle;
  this->set_name(val.get_name());
  this->set_pin_num(val.get_pin_num());
  this->set_period(val.get_period());
  this->set_polarity(val.get_polarity());
  return *this;
}

artik_error artik::Pwm::request(void) {
  return this->m_module->request(&this->m_handle, &this->m_config);
}

artik_error artik::Pwm::release(void) {
  return this->m_module->release(this->m_handle);
}

artik_error artik::Pwm::enable(void) {
  return this->m_module->enable(this->m_handle);
}

artik_error artik::Pwm::disable(void) {
  return this->m_module->disable(this->m_handle);
}

artik_error artik::Pwm::set_period(unsigned int val) {
  return this->m_module->set_period(this->m_handle, val);
}

artik_error artik::Pwm::set_polarity(artik_pwm_polarity_t val) {
  return this->m_module->set_polarity(this->m_handle, val);
}

artik_error artik::Pwm::set_duty_cycle(unsigned int val) {
  return this->m_module->set_duty_cycle(this->m_handle, val);
}

unsigned int artik::Pwm::get_pin_num(void) const {
  return this->m_config.pin_num;
}

char* artik::Pwm::get_name(void) const {
  return this->m_config.name;
}

unsigned int artik::Pwm::get_period(void) const {
  return this->m_config.period;
}

unsigned int artik::Pwm::get_duty_cycle(void) const {
  return this->m_config.duty_cycle;
}

artik_pwm_polarity_t artik::Pwm::get_polarity(void) const {
  return this->m_config.polarity;
}

void artik::Pwm::set_pin_num(unsigned int val) {
  this->m_config.pin_num = val;
}

void artik::Pwm::set_name(char *val) {
  if (this->m_config.name)
    free(this->m_config.name);
  this->m_config.name = strdup(val);
}
