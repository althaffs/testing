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


#include "artik_time.hh"

artik_alarm_handle *artik::Alarm::get_handle(void) {
  return reinterpret_cast<artik_alarm_handle*>(&this->m_handle);
}

artik_alarm_handle *artik::Alarm::get_handle(void) const {
  return const_cast<artik_alarm_handle*>(&this->m_handle);
}

artik::Alarm::Alarm(Alarm const &alarm) {
  *this = alarm;
}

artik::Alarm::Alarm(artik_time_zone gmt, artik_time &date,
    alarm_callback func, void *user_data, artik_time_module *module) {

  if (!module)
    module = reinterpret_cast<artik_time_module*>(
        artik_request_api_module("time"));
  this->m_handle = NULL;
  this->m_module = &(*module);
  this->m_module->create_alarm_date(gmt, date, &this->m_handle, func,
      user_data);
}

artik::Alarm::Alarm(artik_time_zone gmt, artik_msecond second,
    alarm_callback func, void *user_data, artik_time_module *module) {
  if (!module)
    module = reinterpret_cast<artik_time_module*>(
        artik_request_api_module("time"));
  this->m_handle = NULL;
  this->m_module = &(*module);
  this->m_module->create_alarm_second(gmt, second, &this->m_handle, func,
      user_data);
}

artik::Alarm::Alarm() {
  this->m_handle = NULL;
}

artik::Alarm::~Alarm() {
  this->m_module->delete_alarm(this->m_handle);
}

artik_error  artik::Alarm::get_delay(artik_msecond *msecond) {
  return this->m_module->get_delay_alarm(this->m_handle, msecond);
}

artik::Alarm &artik::Alarm::operator=(artik::Alarm const &val) {
  this->m_module = val.m_module;
  this->m_handle = val.m_handle;
  return *this;
}

artik::Time::Time(artik::Time const &val) {
  this->m_module = val.m_module;
}

artik::Time::Time() {
  this->m_module = reinterpret_cast<artik_time_module*>(
      artik_request_api_module("time"));
}

artik::Time::~Time() {
  artik_release_api_module(reinterpret_cast<void*>(this->m_module));
}

artik::Time &artik::Time::operator=(Time const &val) {
  this->m_module = val.m_module;
  return *this;
}

artik_error artik::Time::set_time(artik_time date, artik_time_zone gmt) {
  return this->m_module->set_time(date, gmt);
}

artik_error artik::Time::get_time(artik_time_zone gmt, artik_time *date) const {
  return this->m_module->get_time(gmt, date);
}

artik_error artik::Time::get_time_str(char *date_str, int size,
    char *const format, artik_time_zone gmt) const {
  return this->m_module->get_time_str(date_str, size, format, gmt);
}

artik_msecond artik::Time::get_tick(void) const {
  return this->m_module->get_tick();
}

artik_error artik::Time::sync_ntp(const char* hostname) {
  return this->m_module->sync_ntp(hostname);
}

artik::Alarm *artik::Time::create_alarm_second(artik_time_zone gmt,
    artik_msecond second, alarm_callback func, void *user_data) {
  return new Alarm(gmt, second, func, user_data, this->m_module);
}

artik::Alarm *artik::Time::create_alarm_date(artik_time_zone gmt,
    artik_time date, alarm_callback func, void *user_data) {
  return new Alarm(gmt, date, func, user_data, this->m_module);
}

int artik::Time::compare_dates(const artik_time *date1,
    const artik_time *date2) {
  return this->m_module->compare_dates(date1, date2);
}
