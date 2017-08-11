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

#include "artik_network.hh"

artik::Network::Network() {
  m_module = reinterpret_cast<artik_network_module*>(
      artik_request_api_module("network"));
}

artik::Network::~Network() {
  if (this->m_dhcp_client_handle)
    this->m_module->dhcp_client_stop(this->m_dhcp_client_handle);
  if (this->m_dhcp_server_handle)
    this->m_module->dhcp_server_stop(this->m_dhcp_server_handle);
  artik_release_api_module(reinterpret_cast<void*>(this->m_module));
}

artik_error artik::Network::set_network_config(
    artik_network_config* config,
    artik_network_interface_t interface) {
  return m_module->set_network_config(config, interface);
}

artik_error artik::Network::get_network_config(
    artik_network_config* config,
    artik_network_interface_t interface) {
  return m_module->get_network_config(config, interface);
}

artik_error artik::Network::get_current_public_ip(artik_network_ip* ip) {
  return m_module->get_current_public_ip(ip);
}

artik_error artik::Network::dhcp_client_start(
    artik_network_interface_t interface) {
  return m_module->dhcp_client_start(&this->m_dhcp_client_handle, interface);
}

artik_error artik::Network::dhcp_client_stop() {
  return m_module->dhcp_client_stop(this->m_dhcp_client_handle);
}

artik_error artik::Network::dhcp_server_start(
    artik_network_dhcp_server_config *config) {
  return m_module->dhcp_server_start(&this->m_dhcp_server_handle, config);
}

artik_error artik::Network::dhcp_server_stop() {
  return m_module->dhcp_server_stop(this->m_dhcp_server_handle);
}

artik_error artik::Network::get_online_status(bool *online_status) {
  return m_module->get_online_status(online_status);
}

artik_error artik::Network::add_watch_online_status(
    watch_online_status_handle* handle,
    watch_online_status_callback app_callback, void *user_data) {
  return m_module->add_watch_online_status(handle, app_callback, user_data);
}

artik_error artik::Network::remove_watch_online_status(
    watch_online_status_handle handle) {
  return m_module->remove_watch_online_status(handle);
}
