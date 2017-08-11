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

#include "artik_bluetooth.hh"

artik::Bluetooth::Bluetooth() {
  m_module = reinterpret_cast<artik_bluetooth_module*>(
      artik_request_api_module("bluetooth"));
}

artik::Bluetooth::~Bluetooth() {
  artik_release_api_module(reinterpret_cast<void*>(this->m_module));
}

artik_error artik::Bluetooth::start_scan() {
  return m_module->start_scan();
}

artik_error artik::Bluetooth::stop_scan() {
  return m_module->stop_scan();
}

artik_error artik::Bluetooth::get_devices(artik_bt_device **devices,
    int *num_devices) {
  return m_module->get_devices(devices, num_devices);
}

artik_error artik::Bluetooth::get_paired_devices(artik_bt_device **devices,
    int *num_devices) {
  return m_module->get_paired_devices(devices, num_devices);
}

artik_error artik::Bluetooth::get_connected_devices(artik_bt_device **devices,
    int *num_devices) {
  return m_module->get_connected_devices(devices, num_devices);
}

artik_error artik::Bluetooth::start_bond(const char* addr) {
  return m_module->start_bond(addr);
}

artik_error artik::Bluetooth::stop_bond(const char* addr) {
  return m_module->stop_bond(addr);
}

artik_error artik::Bluetooth::connect(const char* addr) {
  return m_module->connect(addr);
}

artik_error artik::Bluetooth::disconnect(const char* addr) {
  return m_module->disconnect(addr);
}

artik_error artik::Bluetooth::free_devices(artik_bt_device *device_list,
    int count) {
  return m_module->free_devices(device_list, count);
}

artik_error artik::Bluetooth::set_callback(artik_bt_event event,
    artik_bt_callback user_callback, void *user_data) {
  return m_module->set_callback(event, user_callback, user_data);
}

artik_error artik::Bluetooth::unset_callback(artik_bt_event event) {
  return m_module->unset_callback(event);
}

artik_error artik::Bluetooth::remove_unpaired_devices() {
  return m_module->remove_unpaired_devices();
}

artik_error artik::Bluetooth::remove_device(const char *remote_address) {
  return m_module->remove_device(remote_address);
}

artik_error artik::Bluetooth::set_scan_filter(artik_bt_scan_filter *filter) {
  return m_module->set_scan_filter(filter);
}

artik_error artik::Bluetooth::set_alias(const char *alias) {
  return m_module->set_alias(alias);
}

artik_error artik::Bluetooth::set_discoverable(bool discoverable) {
  return m_module->set_discoverable(discoverable);
}

artik_error artik::Bluetooth::set_pairable(bool pairable) {
  return m_module->set_pairable(pairable);
}

artik_error artik::Bluetooth::set_pairableTimeout(unsigned int timeout) {
  return m_module->set_pairableTimeout(timeout);
}

artik_error artik::Bluetooth::set_discoverableTimeout(unsigned int timeout) {
  return m_module->set_discoverableTimeout(timeout);
}

bool artik::Bluetooth::is_scanning(void) {
  return m_module->is_scanning();
}

artik_error artik::Bluetooth::get_device_property(const char *addr,
    const char *property, char **value) {
  return m_module->get_device_property(addr, property, value);
}

artik_error artik::Bluetooth::get_adapter_info(artik_bt_adapter *adapter) {
  return m_module->get_adapter_info(adapter);
}

artik_error artik::Bluetooth::remove_devices(void) {
  return m_module->remove_devices();
}

artik_error artik::Bluetooth::connect_profile(const char *addr,
    const char *uuid) {
  return m_module->connect_profile(addr, uuid);
}

artik_error artik::Bluetooth::set_trust(const char *addr) {
  return m_module->set_trust(addr);
}

artik_error artik::Bluetooth::unset_trust(const char *addr) {
  return m_module->unset_trust(addr);
}

artik_error artik::Bluetooth::set_block(const char *addr) {
  return m_module->set_block(addr);
}

artik_error artik::Bluetooth::unset_block(const char *addr) {
  return m_module->unset_block(addr);
}

artik_error artik::Bluetooth::free_device(artik_bt_device *device) {
  return m_module->free_device(device);
}

bool artik::Bluetooth::is_paired(const char *addr) {
  return m_module->is_paired(addr);
}

bool artik::Bluetooth::is_connected(const char *addr) {
  return m_module->is_connected(addr);
}

bool artik::Bluetooth::is_trusted(const char *addr) {
  return m_module->is_trusted(addr);
}

bool artik::Bluetooth::is_blocked(const char *addr) {
  return m_module->is_blocked(addr);
}

artik_error artik::Bluetooth::gatt_add_service(artik_bt_gatt_service svc,
    int *id) {
  return m_module->gatt_add_service(svc, id);
}

artik_error artik::Bluetooth::gatt_add_characteristic(int svc_id,
    artik_bt_gatt_chr chr, int *id) {
  return m_module->gatt_add_characteristic(svc_id, chr, id);
}

artik_error artik::Bluetooth::gatt_set_char_on_read_request(int svc_id,
    int char_id, artik_bt_gatt_req_read callback, void *user_data) {
  return m_module->gatt_set_char_on_read_request(svc_id, char_id, callback,
      user_data);
}

artik_error artik::Bluetooth::gatt_set_char_on_write_request(int svc_id,
    int char_id, artik_bt_gatt_req_write callback, void *user_data) {
  return m_module->gatt_set_char_on_write_request(svc_id, char_id, callback,
      user_data);
}

artik_error artik::Bluetooth::gatt_set_char_on_notify_request(int svc_id,
    int char_id, artik_bt_gatt_req_notify callback, void *user_data) {
  return m_module->gatt_set_char_on_notify_request(svc_id, char_id, callback,
      user_data);
}

artik_error artik::Bluetooth::gatt_add_descriptor(int service_id, int char_id,
    artik_bt_gatt_desc desc, int *id) {
  return m_module->gatt_add_descriptor(service_id, char_id, desc, id);
}

artik_error artik::Bluetooth::gatt_set_desc_on_read_request(int svc_id,
    int char_id, int desc_id, artik_bt_gatt_req_read callback,
    void *user_data) {
  return m_module->gatt_set_desc_on_read_request(svc_id, char_id, desc_id,
      callback, user_data);
}
artik_error artik::Bluetooth::gatt_set_desc_on_write_request(int svc_id,
    int char_id, int desc_id, artik_bt_gatt_req_write callback,
    void *user_data) {
  return m_module->gatt_set_desc_on_write_request(svc_id, char_id, desc_id,
      callback, user_data);
}

artik_error artik::Bluetooth::gatt_register_service(int id) {
  return m_module->gatt_register_service(id);
}

artik_error artik::Bluetooth::gatt_unregister_service(int id) {
  return m_module->gatt_unregister_service(id);
}

artik_error artik::Bluetooth::gatt_get_service_list(const char *addr,
    artik_bt_uuid **uuid_list, int *len) {
  return m_module->gatt_get_service_list(addr, uuid_list, len);
}

artik_error artik::Bluetooth::gatt_get_characteristic_list(const char* addr,
    const char *srv_uuid, artik_bt_uuid **uuid_list, int *len) {
  return m_module->gatt_get_characteristic_list(addr, srv_uuid, uuid_list,
      len);
}

artik_error artik::Bluetooth::gatt_get_descriptor_list(const char *addr,
    const char *srv_uuid, const char *char_uuid, artik_bt_uuid **uuid_list,
    int *len) {
  return m_module->gatt_get_descriptor_list(addr, srv_uuid, char_uuid,
      uuid_list, len);
}

artik_error artik::Bluetooth::gatt_char_read_value(const char *addr,
    const char *srv_uuid, const char *char_uuid, unsigned char **byte,
    int *byte_len) {
  return m_module->gatt_char_read_value(addr, srv_uuid, char_uuid, byte,
      byte_len);
}

artik_error artik::Bluetooth::gatt_char_write_value(const char *addr,
    const char *srv_uuid, const char *char_uuid, const unsigned char byte[],
    int byte_len) {
  return m_module->gatt_char_write_value(addr, srv_uuid, char_uuid, byte,
      byte_len);
}

artik_error artik::Bluetooth::gatt_desc_read_value(const char *addr,
    const char *srv_uuid, const char *char_uuid, const char *desc_uuid,
    unsigned char **byte, int *byte_len) {
  return m_module->gatt_desc_read_value(addr, srv_uuid, char_uuid, desc_uuid,
      byte, byte_len);
}

artik_error artik::Bluetooth::gatt_desc_write_value(const char *addr,
    const char *srv_uuid, const char *char_uuid, const char *desc_uuid,
    const unsigned char byte[], int byte_len) {
  return m_module->gatt_desc_write_value(addr, srv_uuid, char_uuid, desc_uuid,
      byte, byte_len);
}

artik_error artik::Bluetooth::gatt_start_notify(const char *addr,
    const char* srv_uuid, const char *char_uuid) {
  return m_module->gatt_start_notify(addr, srv_uuid, char_uuid);
}

artik_error artik::Bluetooth::gatt_stop_notify(const char *addr,
    const char* srv_uuid, const char *char_uuid) {
  return m_module->gatt_stop_notify(addr, srv_uuid, char_uuid);
}

artik_error artik::Bluetooth::gatt_get_char_properties(const char *addr,
    const char* srv_uuid, const char *char_uuid,
    artik_bt_gatt_char_properties *properties) {
  return m_module->gatt_get_char_properties(addr, srv_uuid, char_uuid,
      properties);
}

artik_error artik::Bluetooth::register_advertisement(
    artik_bt_advertisement *user_adv, int *id) {
  return m_module->register_advertisement(user_adv, id);
}

artik_error artik::Bluetooth::unregister_advertisement(int id) {
  return m_module->unregister_advertisement(id);
}

artik_error artik::Bluetooth::gatt_req_set_value(artik_bt_gatt_req req,
    int len, const unsigned char *value) {
  return m_module->gatt_req_set_value(req, len, value);
}

artik_error artik::Bluetooth::gatt_req_set_result(artik_bt_gatt_req request,
    artik_bt_gatt_req_state_type state, const char *err_msg) {
  return m_module->gatt_req_set_result(request, state, err_msg);
}

artik_error artik::Bluetooth::gatt_notify(int svc_id, int char_id,
    unsigned char *byte, int len) {
  return m_module->gatt_notify(svc_id, char_id, byte, len);
}

artik_error artik::Bluetooth::avrcp_controller_change_folder(
    const char* folder) {
  return m_module->avrcp_controller_change_folder(folder);
}

artik_error artik::Bluetooth::avrcp_controller_list_item(int start_item,
  int end_item, artik_bt_avrcp_item **item_list) {
  return m_module->avrcp_controller_list_item(start_item, end_item, item_list);
}

artik_error artik::Bluetooth::avrcp_controller_get_repeat_mode(
    artik_bt_avrcp_repeat_mode *repeat_mode) {
  return m_module->avrcp_controller_get_repeat_mode(repeat_mode);
}

artik_error artik::Bluetooth::avrcp_controller_set_repeat_mode(
    artik_bt_avrcp_repeat_mode mode) {
  return m_module->avrcp_controller_set_repeat_mode(mode);
}

artik_error artik::Bluetooth::avrcp_controller_is_connected(
    bool *is_connected) {
  return m_module->avrcp_controller_is_connected(is_connected);
}

artik_error artik::Bluetooth::avrcp_controller_resume_play() {
  return m_module->avrcp_controller_resume_play();
}

artik_error artik::Bluetooth::avrcp_controller_pause() {
  return m_module->avrcp_controller_pause();
}

artik_error artik::Bluetooth::avrcp_controller_stop() {
  return m_module->avrcp_controller_stop();
}

artik_error artik::Bluetooth::avrcp_controller_next() {
  return m_module->avrcp_controller_next();
}

artik_error artik::Bluetooth::avrcp_controller_previous() {
  return m_module->avrcp_controller_previous();
}

artik_error artik::Bluetooth::avrcp_controller_fast_forward() {
  return m_module->avrcp_controller_fast_forward();
}

artik_error artik::Bluetooth::avrcp_controller_rewind() {
  return m_module->avrcp_controller_rewind();
}
artik_error artik::Bluetooth::avrcp_controller_get_property(char *item,
    artik_bt_avrcp_item_property **properties) {
  return m_module->avrcp_controller_get_property(item, properties);
}

artik_error artik::Bluetooth::avrcp_controller_play_item(char *item) {
  return m_module->avrcp_controller_play_item(item);
}

artik_error artik::Bluetooth::avrcp_controller_add_to_playing(char *item) {
  return m_module->avrcp_controller_add_to_playing(item);
}

artik_error artik::Bluetooth::avrcp_controller_get_name(char **name) {
  return m_module->avrcp_controller_get_name(name);
}

artik_error artik::Bluetooth::avrcp_controller_get_status(char **status) {
  return m_module->avrcp_controller_get_status(status);
}

artik_error artik::Bluetooth::avrcp_controller_get_subtype(char **subtype) {
  return m_module->avrcp_controller_get_subtype(subtype);
}

artik_error artik::Bluetooth::avrcp_controller_get_type(char **type) {
  return m_module->avrcp_controller_get_type(type);
}

artik_error artik::Bluetooth::avrcp_controller_get_browsable(bool *browsable) {
  return m_module->avrcp_controller_get_browsable(browsable);
}

artik_error artik::Bluetooth::avrcp_controller_get_position(
    unsigned int *position) {
  return m_module->avrcp_controller_get_position(position);
}

artik_error artik::Bluetooth::pan_register(const char *uuid,
    const char *bridge) {
  return m_module->pan_register(uuid, bridge);
}

artik_error artik::Bluetooth::pan_unregister(const char *uuid) {
  return m_module->pan_unregister(uuid);
}

artik_error artik::Bluetooth::pan_connect(const char *mac_addr,
    const char *uuid, char **network_interface) {
  return m_module->pan_connect(mac_addr, uuid, network_interface);
}

artik_error artik::Bluetooth::pan_disconnect() {
  return m_module->pan_disconnect();
}

artik_error artik::Bluetooth::pan_get_connected(bool *connected) {
  return m_module->pan_get_connected(connected);
}

artik_error artik::Bluetooth::pan_get_interface(char **interface) {
  return m_module->pan_get_interface(interface);
}

artik_error artik::Bluetooth::pan_get_UUID(char **uuid) {
  return m_module->pan_get_UUID(uuid);
}

artik_error artik::Bluetooth::spp_register_profile(
    artik_bt_spp_profile_option *opt) {
  return m_module->spp_register_profile(opt);
}

artik_error artik::Bluetooth::spp_unregister_profile() {
  return m_module->spp_unregister_profile();
}

artik_error artik::Bluetooth::spp_set_callback(
  release_callback release_func,
  new_connection_callback connect_func,
  request_disconnect_callback disconnect_func,
  void *user_data) {
  return m_module->spp_set_callback(release_func, connect_func,
      disconnect_func, user_data);
}

artik_error artik::Bluetooth::ftp_create_session(char *dest_addr) {
  return m_module->ftp_create_session(dest_addr);
}

artik_error artik::Bluetooth::ftp_remove_session() {
  return m_module->ftp_remove_session();
}

artik_error artik::Bluetooth::ftp_change_folder(char *folder) {
  return m_module->ftp_change_folder(folder);
}

artik_error artik::Bluetooth::ftp_create_folder(char *folder) {
  return m_module->ftp_create_folder(folder);
}

artik_error artik::Bluetooth::ftp_delete_file(char *file) {
  return m_module->ftp_delete_file(file);
}

artik_error artik::Bluetooth::ftp_list_folder(artik_bt_ftp_file **file_list) {
  return m_module->ftp_list_folder(file_list);
}

artik_error artik::Bluetooth::ftp_get_file(char *target_file,
    char *source_file) {
  return m_module->ftp_get_file(target_file, source_file);
}

artik_error artik::Bluetooth::ftp_put_file(char *source_file,
    char *target_file) {
  return m_module->ftp_put_file(source_file, target_file);
}

artik_error artik::Bluetooth::ftp_resume_transfer() {
  return m_module->ftp_resume_transfer();
}

artik_error artik::Bluetooth::ftp_suspend_transfer() {
  return m_module->ftp_suspend_transfer();
}

artik_error artik::Bluetooth::agent_register_capability(
    artik_bt_agent_capability e) {
  return m_module->agent_register_capability(e);
}

artik_error artik::Bluetooth::agent_set_default() {
  return m_module->agent_set_default();
}

artik_error artik::Bluetooth::agent_unregister() {
  return m_module->agent_unregister();
}

artik_error artik::Bluetooth::agent_set_callback(
    artik_bt_agent_callbacks *agent_callback) {
  return m_module->agent_set_callback(agent_callback);
}

artik_error artik::Bluetooth::agent_send_pincode(
    artik_bt_agent_request_handle handle, char *pincode) {
  return m_module->agent_send_pincode(handle, pincode);
}

artik_error artik::Bluetooth::agent_send_passkey(
    artik_bt_agent_request_handle handle, unsigned int passkey) {
  return m_module->agent_send_passkey(handle, passkey);
}

artik_error artik::Bluetooth::agent_send_error(
    artik_bt_agent_request_handle handle, artik_bt_agent_request_error e,
    const char *err_msg) {
  return m_module->agent_send_error(handle, e, err_msg);
}

artik_error artik::Bluetooth::agent_send_empty_response(
    artik_bt_agent_request_handle handle) {
  return m_module->agent_send_empty_response(handle);
}
