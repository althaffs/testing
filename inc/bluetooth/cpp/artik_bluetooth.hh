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

#ifndef BLUETOOTH_CPP_ARTIK_BLUETOOTH_HH_
#define BLUETOOTH_CPP_ARTIK_BLUETOOTH_HH_

#include <artik_module.h>
#include <artik_bluetooth.h>

/*! \file artik_bluetooth.hh
 *
 *  \brief C++ Wrapper to the Bluetooth module
 *
 *  This is a class encapsulation of the C
 *  Bluetooth module API \ref artik_bluetooth.h
 */

namespace artik {
/*!
 *  \brief Bluetooth module C++ Class
 */
class Bluetooth {
 private:
  artik_bluetooth_module* m_module;

 public:
  Bluetooth();
  ~Bluetooth();

  artik_error start_scan();
  artik_error stop_scan();
  artik_error get_devices(artik_bt_device **devices, int *num_devices);
  artik_error get_paired_devices(artik_bt_device **devices, int *num_devices);
  artik_error get_connected_devices(artik_bt_device **devices,
      int *num_devices);
  artik_error start_bond(const char* addr);
  artik_error stop_bond(const char* addr);
  artik_error connect(const char* addr);
  artik_error disconnect(const char* addr);
  artik_error free_devices(artik_bt_device *device_list, int count);
  artik_error set_callback(artik_bt_event event,
      artik_bt_callback user_callback, void *user_data);
  artik_error unset_callback(artik_bt_event event);
  artik_error remove_unpaired_devices();
  artik_error remove_device(const char *remote_address);
  artik_error set_scan_filter(artik_bt_scan_filter *filter);
  artik_error set_alias(const char *alias);
  artik_error set_discoverable(bool discoverable);
  artik_error set_pairable(bool pairable);
  artik_error set_pairableTimeout(unsigned int timeout);
  artik_error set_discoverableTimeout(unsigned int timeout);
  bool is_scanning(void);
  artik_error get_device_property(const char *addr, const char *property,
    char **value);
  artik_error get_adapter_info(artik_bt_adapter *adapter);
  artik_error remove_devices(void);
  artik_error connect_profile(const char *addr, const char *uuid);
  artik_error set_trust(const char *addr);
  artik_error unset_trust(const char *addr);
  artik_error set_block(const char *addr);
  artik_error unset_block(const char *addr);
  artik_error free_device(artik_bt_device *device);
  bool is_paired(const char *addr);
  bool is_connected(const char *addr);
  bool is_trusted(const char *addr);
  bool is_blocked(const char *addr);
  artik_error gatt_add_service(artik_bt_gatt_service svc, int *id);
  artik_error gatt_add_characteristic(int svc_id, artik_bt_gatt_chr chr,
      int *id);
  artik_error gatt_set_char_on_read_request(int svc_id, int char_id,
      artik_bt_gatt_req_read callback, void *user_data);
  artik_error gatt_set_char_on_write_request(int svc_id, int char_id,
      artik_bt_gatt_req_write callback, void *user_data);
  artik_error gatt_set_char_on_notify_request(int svc_id, int char_id,
      artik_bt_gatt_req_notify callback, void *user_data);
  artik_error gatt_add_descriptor(int service_id, int char_id,
      artik_bt_gatt_desc desc, int *id);
  artik_error gatt_set_desc_on_read_request(int svc_id, int char_id,
      int desc_id, artik_bt_gatt_req_read callback, void *user_data);
  artik_error gatt_set_desc_on_write_request(int svc_id, int char_id,
      int desc_id, artik_bt_gatt_req_write callback, void *user_data);
  artik_error gatt_register_service(int id);
  artik_error gatt_unregister_service(int id);
  artik_error gatt_get_service_list(const char *addr,
      artik_bt_uuid **uuid_list, int *len);
  artik_error gatt_get_characteristic_list(const char* addr,
      const char* srv_uuid, artik_bt_uuid **uuid_list, int *len);
  artik_error gatt_get_descriptor_list(const char *addr, const char* srv_uuid,
      const char* char_uuid, artik_bt_uuid **uuid_list, int *len);
  artik_error gatt_char_read_value(const char *addr, const char *srv_uuid,
      const char *char_uuid, unsigned char **byte, int *byte_len);
  artik_error gatt_char_write_value(const char *addr, const char *srv_uuid,
      const char *char_uuid, const unsigned char byte[], int byte_len);
  artik_error gatt_desc_read_value(const char *addr, const char *srv_uuid,
      const char *char_uuid, const char *desc_uuuid, unsigned char **byte,
      int *byte_len);
  artik_error gatt_desc_write_value(const char *addr, const char *srv_uuid,
      const char *char_uuid, const char *desc_uuid, const unsigned char byte[],
      int byte_len);
  artik_error gatt_start_notify(const char *addr, const char *srv_uuid,
      const char *char_uuid);
  artik_error gatt_stop_notify(const char *addr, const char *srv_uuid,
      const char *char_uuid);
  artik_error gatt_get_char_properties(const char *addr, const char *srv_uuid,
      const char *char_uuid, artik_bt_gatt_char_properties *properties);
  artik_error register_advertisement(artik_bt_advertisement *user_adv, int *id);
  artik_error unregister_advertisement(int id);
  artik_error gatt_req_set_value(artik_bt_gatt_req req, int len,
      const unsigned char *value);
  artik_error gatt_req_set_result(artik_bt_gatt_req req,
        artik_bt_gatt_req_state_type state, const char *err_msg);
  artik_error gatt_notify(int svc_id, int char_id, unsigned char *byte,
      int len);
  artik_error avrcp_controller_change_folder(const char* folder);
  artik_error avrcp_controller_list_item(int start_item, int end_item,
       artik_bt_avrcp_item **item_list);
  artik_error avrcp_controller_get_repeat_mode(
      artik_bt_avrcp_repeat_mode *repeat_mode);
  artik_error avrcp_controller_set_repeat_mode(artik_bt_avrcp_repeat_mode mode);
  artik_error avrcp_controller_is_connected(bool *is_connected);
  artik_error avrcp_controller_resume_play();
  artik_error avrcp_controller_pause();
  artik_error avrcp_controller_stop();
  artik_error avrcp_controller_next();
  artik_error avrcp_controller_previous();
  artik_error avrcp_controller_fast_forward();
  artik_error avrcp_controller_rewind();
  artik_error avrcp_controller_get_property(char *item,
      artik_bt_avrcp_item_property **properties);
  artik_error avrcp_controller_play_item(char *item);
  artik_error avrcp_controller_add_to_playing(char *item);
  artik_error avrcp_controller_get_name(char **name);
  artik_error avrcp_controller_get_status(char **status);
  artik_error avrcp_controller_get_subtype(char **subtype);
  artik_error avrcp_controller_get_type(char **type);
  artik_error avrcp_controller_get_browsable(bool *browsable);
  artik_error avrcp_controller_get_position(unsigned int *position);
  artik_error pan_register(const char *uuid, const char *bridge);
  artik_error pan_unregister(const char *uuid);
  artik_error pan_connect(const char *mac_addr, const char *uuid,
      char **network_interface);
  artik_error pan_disconnect();
  artik_error pan_get_connected(bool *connected);
  artik_error pan_get_interface(char **interface);
  artik_error pan_get_UUID(char **uuid);
  artik_error spp_register_profile(artik_bt_spp_profile_option *opt);
  artik_error spp_unregister_profile();
  artik_error spp_set_callback(
    release_callback relase_func,
    new_connection_callback connect_func,
    request_disconnect_callback disconnect_func,
    void *user_data);
  artik_error ftp_create_session(char *dest_addr);
  artik_error ftp_remove_session();
  artik_error ftp_change_folder(char *folder);
  artik_error ftp_create_folder(char *folder);
  artik_error ftp_delete_file(char *file);
  artik_error ftp_list_folder(artik_bt_ftp_file **file_list);
  artik_error ftp_get_file(char *target_file, char *source_file);
  artik_error ftp_put_file(char *source_file, char *target_file);
  artik_error ftp_resume_transfer();
  artik_error ftp_suspend_transfer();
  artik_error agent_register_capability(artik_bt_agent_capability e);
  artik_error agent_set_default();
  artik_error agent_unregister();
  artik_error agent_set_callback(artik_bt_agent_callbacks *agent_callback);
  artik_error agent_send_pincode(artik_bt_agent_request_handle handle,
      char *pincode);
  artik_error agent_send_passkey(artik_bt_agent_request_handle handle,
      unsigned int passkey);
  artik_error agent_send_error(artik_bt_agent_request_handle handle,
      artik_bt_agent_request_error e, const char *err_msg);
  artik_error agent_send_empty_response(artik_bt_agent_request_handle handle);
};

}  // namespace artik

#endif  // BLUETOOTH_CPP_ARTIK_BLUETOOTH_HH_
