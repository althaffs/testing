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

#include "artik_lwm2m.hh"

artik::Lwm2m::Lwm2m() {
  m_module = reinterpret_cast<artik_lwm2m_module*>(
      artik_request_api_module("lwm2m"));
}

artik::Lwm2m::~Lwm2m() {
  this->m_module->client_disconnect(this->m_handle);
  artik_release_api_module(reinterpret_cast<void*>(this->m_module));
}

artik_error artik::Lwm2m::client_connect(artik_lwm2m_config *params) {
  return this->m_module->client_connect(&this->m_handle, params);
}

artik_error artik::Lwm2m::client_disconnect() {
  return this->m_module->client_disconnect(this->m_handle);
}

artik_error artik::Lwm2m::client_write_resource(const char *uri,
    unsigned char *buffer, int length) {
  return this->m_module->client_write_resource(this->m_handle, uri, buffer,
      length);
}

artik_error artik::Lwm2m::client_read_resource(const char *uri,
    unsigned char *buffer, int* length) {
  return this->m_module->client_read_resource(this->m_handle, uri, buffer,
      length);
}

artik_error artik::Lwm2m::set_callback(artik_lwm2m_event_t event,
    artik_lwm2m_callback user_callback, void *user_data) {
  return this->m_module->set_callback(this->m_handle, event, user_callback,
      user_data);
}

artik_error artik::Lwm2m::unset_callback(artik_lwm2m_event_t event) {
  return this->m_module->unset_callback(this->m_handle, event);
}

artik_lwm2m_object* artik::Lwm2m::create_device_object(const char* manufacturer,
    const char* model, const char* serial, const char* fw_version,
    const char* hw_version, const char *sw_version, const char* device_type,
    int power_source, int power_volt, int power_current, int battery_level,
    int memory_total, int memory_free, const char* time_zone,
    const char *utc_offset, const char* binding) {
  return this->m_module->create_device_object(manufacturer, model, serial,
      fw_version, hw_version, sw_version, device_type, power_source, power_volt,
      power_current, battery_level, memory_total, memory_free, time_zone,
      utc_offset, binding);
}

artik_lwm2m_object *artik::Lwm2m::create_connectivity_monitoring_object(
    int netbearer, int avlnetbearer, int signalstrength, int linkquality,
    int lenip, const char **ipaddr, int lenroute, const char **routeaddr,
    int linkutilization, const char *apn, int cellid, int smnc, int smcc) {
  return this->m_module->create_connectivity_monitoring_object(netbearer,
      avlnetbearer, signalstrength, linkquality, lenip, ipaddr, lenroute,
      routeaddr, linkutilization, apn, cellid, smnc, smcc);
}

void artik::Lwm2m::free_object(artik_lwm2m_object* object) {
  return this->m_module->free_object(object);
}

artik_error artik::Lwm2m::serialize_tlv_int(int *data, int size,
    unsigned char **buffer, int *bufferlen) {
  return this->m_module->serialize_tlv_int(data, size, buffer, bufferlen);
}

artik_error artik::Lwm2m::serialize_tlv_string(char **data, int size,
    unsigned char **buffer, int *bufferlen) {
  return this->m_module->serialize_tlv_string(data, size, buffer, bufferlen);
}
