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

#ifndef __OS_BLUETOOTH_H
#define __OS_BLUETOOTH_H

#include <artik_bluetooth.h>
#include <artik_error.h>

#ifdef __cplusplus
extern "C" {
#endif

artik_error os_bt_set_scan_filter(artik_bt_scan_filter *filter);
artik_error os_bt_set_alias(const char *alias);
artik_error os_bt_set_discoverable(bool discoverable);
artik_error os_bt_set_pairable(bool pairable);
artik_error os_bt_set_pairableTimeout(unsigned int timeout);
artik_error os_bt_set_discoverableTimeout(unsigned int timeout);
bool os_bt_is_scanning(void);
artik_error os_bt_get_device_property(const char *addr, const char *property,
		char **value);
artik_error os_bt_get_adapter_info(artik_bt_adapter *adapter);
artik_error os_bt_start_scan(void);
artik_error os_bt_stop_scan(void);
artik_error os_bt_get_devices(artik_bt_device **devices, int *num_devices);
artik_error os_bt_get_paired_devices(artik_bt_device **devices,
		int *num_devices);
artik_error os_bt_get_connected_devices(artik_bt_device **devices,
		int *num_devices);
artik_error os_bt_start_bond(const char *addr);
artik_error os_bt_stop_bond(const char *addr);
artik_error os_bt_connect(const char *addr);
artik_error os_bt_disconnect(const char *addr);
artik_error os_bt_free_devices(artik_bt_device *device_list, int count);
artik_error os_bt_set_callback(artik_bt_event event,
		artik_bt_callback user_callback, void *user_data);
artik_error os_bt_unset_callback(artik_bt_event event);
artik_error os_bt_remove_unpaired_devices(void);
artik_error os_bt_remove_device(const char *remote_address);
artik_error os_bt_remove_devices(void);
artik_error os_bt_connect_profile(const char *addr, const char *uuid);
artik_error os_bt_set_trust(const char *addr);
artik_error os_bt_unset_trust(const char *addr);
artik_error os_bt_set_block(const char *addr);
artik_error os_bt_unset_block(const char *addr);
artik_error os_bt_free_device(artik_bt_device *device);
bool os_bt_is_paired(const char *addr);
bool os_bt_is_connected(const char *addr);
bool os_bt_is_trusted(const char *addr);
bool os_bt_is_blocked(const char *addr);
artik_error os_bt_gatt_add_service(artik_bt_gatt_service svc, int *id);
artik_error os_bt_gatt_add_characteristic(int svc_id, artik_bt_gatt_chr chr,
		int *id);
artik_error os_bt_gatt_set_chr_on_read_request(int svc_id, int char_id,
		artik_bt_gatt_req_read callback, void *user_data);
artik_error os_bt_gatt_set_chr_on_write_request(int svc_id, int char_id,
		artik_bt_gatt_req_write callback, void *user_data);
artik_error os_bt_gatt_set_chr_on_notify_request(int svc_id, int char_id,
		artik_bt_gatt_req_notify callback, void *user_data);
artik_error os_bt_gatt_add_descriptor(int service_id, int char_id,
		artik_bt_gatt_desc desc, int *id);
artik_error os_bt_gatt_set_desc_on_read_request(int svc_id, int char_id, int desc_id,
		artik_bt_gatt_req_read callback, void *user_data);
artik_error os_bt_gatt_set_desc_on_write_request(int svc_id, int char_id, int desc_id,
		artik_bt_gatt_req_write callback, void *user_data);
artik_error os_bt_gatt_register_service(int id);
artik_error os_bt_gatt_unregister_service(int id);
artik_error os_bt_gatt_get_service_list(const char *addr,
		artik_bt_uuid **uuid_list, int *len);
artik_error os_bt_gatt_get_chracteristic_list(const char *addr,
		const char *srv_uuid, artik_bt_uuid **uuid_list, int *len);
artik_error os_bt_gatt_get_descriptor_list(const char *addr, const char *srv_uuid,
		const char *char_uuid, artik_bt_uuid **uuid_list, int *len);
artik_error os_bt_gatt_char_read_value(const char *addr, const char *srv_uuid,
		const char *char_uuid, unsigned char **byte, int *byte_len);
artik_error os_bt_gatt_char_write_value(const char *addr, const char *srv_uuid,
		const char *char_uuid, const unsigned char byte[], int byte_len);
artik_error os_bt_gatt_desc_read_value(const char *addr, const char *srv_uuid,
		const char *char_uuid, const char *desc_uuid, unsigned char **byte, int *byte_len);
artik_error os_bt_gatt_desc_write_value(const char *addr, const char *srv_uuid,
		const char *char_uuid, const char *desc_uuid, const unsigned char byte[], int byte_len);
artik_error os_bt_gatt_start_notify(const char *addr, const char *srv_uuid, const char *char_uuid);
artik_error os_bt_gatt_stop_notify(const char *addr, const char *srv_uuid, const char *char_uuid);
artik_error os_bt_gatt_get_char_properties(const char *addr, const char *srv_uuid,
		const char *char_uuid, artik_bt_gatt_char_properties *properties);
artik_error os_bt_register_advertisement(
		artik_bt_advertisement * user_adv, int *id);
artik_error os_bt_unregister_advertisement(int id);
artik_error os_bt_gatt_req_set_value(artik_bt_gatt_req req, int len,
		const unsigned char *value);
artik_error os_bt_gatt_req_set_result(artik_bt_gatt_req req,
		artik_bt_gatt_req_state_type state, const char *err_msg);
artik_error os_bt_gatt_notify(int svc_id, int char_id, unsigned char *byte, int len);
artik_error os_bt_a2dp_source_register(unsigned char codec,
		bool delay_reporting, const char *path,
		const unsigned char *capabilities, int cap_size);
artik_error os_bt_a2dp_source_unregister(void);
artik_error os_bt_a2dp_source_acquire(int *fd,
		unsigned short *mtu_read, unsigned short *mtu_write);
artik_error os_bt_a2dp_source_get_properties(
		artik_bt_a2dp_source_property * *properties);
artik_error os_bt_a2dp_source_release(void);
artik_error os_bt_a2dp_source_set_callback(select_config_callback select_func,
		set_config_callback set_func,
		clear_config_callback clear_func);
artik_error os_bt_avrcp_controller_change_folder(const char *folder);
artik_error os_bt_avrcp_controller_list_item(int start_item, int end_item,
		artik_bt_avrcp_item **item_list);
artik_error os_bt_avrcp_controller_get_repeat(char **repeat_mode);
artik_error os_bt_avrcp_controller_set_repeat(const char *repeat_mode);
artik_error os_bt_avrcp_controller_is_connected(bool *is_connected);
artik_error os_bt_avrcp_controller_resume_play(void);
artik_error os_bt_avrcp_controller_pause(void);
artik_error os_bt_avrcp_controller_stop(void);
artik_error os_bt_avrcp_controller_next(void);
artik_error os_bt_avrcp_controller_previous(void);
artik_error os_bt_avrcp_controller_fast_forward(void);
artik_error os_bt_avrcp_controller_rewind(void);
artik_error os_bt_avrcp_controller_get_property(char *item,
		artik_bt_avrcp_item_property **properties);
artik_error os_bt_avrcp_controller_play_item(char *item);
artik_error os_bt_avrcp_controller_add_to_playing(char *item);
artik_error os_bt_avrcp_controller_get_name(char **name);
artik_error os_bt_avrcp_controller_get_status(char **status);
artik_error os_bt_avrcp_controller_get_subtype(char **subtype);
artik_error os_bt_avrcp_controller_get_type(char **type);
artik_error os_bt_avrcp_controller_get_browsable(bool *is_browsable);
artik_error os_bt_avrcp_controller_get_position(unsigned int *position);
artik_error os_bt_pan_register(const char *uuid, const char *bridge);
artik_error os_bt_pan_unregister(const char *uuid);
artik_error os_bt_pan_connect(const char *mac_addr,
		const char *uuid, char **network_interface);
artik_error os_bt_pan_disconnect(void);
artik_error os_bt_pan_get_connected(bool *connected);
artik_error os_bt_pan_get_interface(char **_interface);
artik_error os_bt_pan_get_UUID(char **uuid);
artik_error os_bt_spp_register_profile(artik_bt_spp_profile_option *opt);
artik_error os_bt_spp_unregister_profile(void);
artik_error os_bt_spp_set_callback(release_callback release_func,
		new_connection_callback connect_func,
		request_disconnect_callback disconnect_func,
		void *user_data);
artik_error os_bt_ftp_create_session(char *dest_addr);
artik_error os_bt_ftp_remove_session(void);
artik_error os_bt_ftp_change_folder(char *folder);
artik_error os_bt_ftp_create_folder(char *folder);
artik_error os_bt_ftp_delete_file(char *file);
artik_error os_bt_ftp_list_folder(artik_bt_ftp_file **file_list);
artik_error os_bt_ftp_get_file(char *target_file, char *source_file);
artik_error os_bt_ftp_put_file(char *source_file, char *target_file);
artik_error os_bt_ftp_cancel_transfer(void);
artik_error os_bt_ftp_resume_transfer(void);
artik_error os_bt_ftp_suspend_transfer(void);
artik_error os_bt_agent_register_capability(artik_bt_agent_capability e);
artik_error os_bt_agent_set_default(void);
artik_error os_bt_agent_unregister(void);
artik_error os_bt_agent_set_callback(artik_bt_agent_callbacks *agent_callback);
artik_error os_bt_agent_send_pincode(artik_bt_agent_request_handle handle, char *pincode);
artik_error os_bt_agent_send_passkey(artik_bt_agent_request_handle handle, unsigned int passkey);
artik_error os_bt_agent_send_error(artik_bt_agent_request_handle handle,
		artik_bt_agent_request_error e, const char *err_msg);
artik_error os_bt_agent_send_empty_response(artik_bt_agent_request_handle handle);
#ifdef __cplusplus
}
#endif
#endif	/*__OS_BLUETOOTH_H*/
