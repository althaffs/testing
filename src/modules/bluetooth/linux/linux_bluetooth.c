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

#include "../os_bluetooth.h"
#include "adapter.h"
#include "bt.h"
#include "device.h"
#include "gatt.h"
#include "a2dp.h"
#include "avrcp.h"
#include "pan.h"
#include "spp.h"
#include "ftp.h"
#include "advertisement.h"
#include "agent.h"

artik_error os_bt_set_scan_filter(artik_bt_scan_filter *filter)
{
	return bt_set_scan_filter(filter);
}

artik_error os_bt_set_alias(const char *alias)
{
	return bt_set_alias(alias);
}

artik_error os_bt_set_discoverable(bool discoverable)
{
	return bt_set_discoverable(discoverable);
}

artik_error os_bt_set_pairable(bool pairable)
{
	return bt_set_pairable(pairable);
}

artik_error os_bt_set_pairableTimeout(unsigned int timeout)
{
	return bt_set_pairableTimeout(timeout);
}

artik_error os_bt_set_discoverableTimeout(unsigned int timeout)
{
	return bt_set_discoverableTimeout(timeout);
}

bool os_bt_is_scanning(void)
{
	return bt_is_scanning();
}

artik_error os_bt_get_device_property(const char *path,
		const char *property, char **value)
{
	return bt_get_device_property(path, property, value);
}

artik_error os_bt_get_adapter_info(artik_bt_adapter *adapter)
{
	return bt_get_adapter_info(adapter);
}

artik_error os_bt_start_scan(void)
{
	return bt_start_scan();
}

artik_error os_bt_stop_scan(void)
{
	return bt_stop_scan();
}

artik_error os_bt_get_devices(artik_bt_device **devices,
		int *num_devices)
{
	return bt_get_devices(devices, num_devices);
}

artik_error os_bt_get_paired_devices(artik_bt_device **devices,
		int *num_devices)
{
	return bt_get_paired_devices(devices, num_devices);
}

artik_error os_bt_get_connected_devices(artik_bt_device **devices,
		int *num_devices)
{
	return bt_get_connected_devices(devices, num_devices);
}

artik_error os_bt_start_bond(const char *addr)
{
	return bt_start_bond(addr);
}

artik_error os_bt_stop_bond(const char *addr)
{
	return bt_stop_bond(addr);
}

artik_error os_bt_connect(const char *addr)
{
	return bt_connect(addr);
}

artik_error os_bt_disconnect(const char *addr)
{
	return bt_disconnect(addr);
}

artik_error os_bt_free_devices(artik_bt_device *device_list, int count)
{
	return bt_free_devices(device_list, count);
}

artik_error os_bt_set_callback(artik_bt_event event,
		artik_bt_callback user_callback, void *user_data)
{
	return bt_set_callback(event, user_callback, user_data);
}

artik_error os_bt_unset_callback(artik_bt_event event)
{
	return bt_unset_callback(event);
}

artik_error os_bt_remove_unpaired_devices(void)
{
	return bt_remove_unpaired_devices();
}

artik_error os_bt_remove_device(const char *remote_address)
{
	return bt_remove_device(remote_address);
}

artik_error os_bt_remove_devices(void)
{
	return bt_remove_devices();
}

artik_error os_bt_connect_profile(const char *addr, const char *uuid)
{
	return bt_connect_profile(addr, uuid);
}

artik_error os_bt_set_trust(const char *addr)
{
	return bt_set_trust(addr);
}

artik_error os_bt_unset_trust(const char *addr)
{
	return bt_unset_trust(addr);
}

artik_error os_bt_set_block(const char *addr)
{
	return bt_set_block(addr);
}

artik_error os_bt_unset_block(const char *addr)
{
	return bt_unset_block(addr);
}

artik_error os_bt_free_device(artik_bt_device *device)
{
	return bt_free_device(device);
}

bool os_bt_is_paired(const char *addr)
{
	return bt_is_paired(addr);
}

bool os_bt_is_connected(const char *addr)
{
	return bt_is_connected(addr);
}

bool os_bt_is_trusted(const char *addr)
{
	return bt_is_trusted(addr);
}

bool os_bt_is_blocked(const char *addr)
{
	return bt_is_blocked(addr);
}

artik_error os_bt_gatt_add_service(artik_bt_gatt_service svc, int *id)
{
	return bt_gatt_add_service(svc, id);
}

artik_error os_bt_gatt_add_characteristic(int svc_id, artik_bt_gatt_chr chr,
		int *id)
{
	return bt_gatt_add_characteristic(svc_id, chr, id);
}

artik_error os_bt_gatt_set_chr_on_read_request(int svc_id, int char_id,
		artik_bt_gatt_req_read callback, void *user_data)
{
	return bt_gatt_set_chr_on_read_request(svc_id, char_id, callback, user_data);
}

artik_error os_bt_gatt_set_chr_on_write_request(int svc_id, int char_id,
		artik_bt_gatt_req_write callback, void *user_data)
{
	return bt_gatt_set_chr_on_write_request(svc_id, char_id, callback, user_data);
}

artik_error os_bt_gatt_set_chr_on_notify_request(int svc_id, int char_id,
		artik_bt_gatt_req_notify callback, void *user_data)
{
	return bt_gatt_set_chr_on_notify_request(svc_id, char_id, callback, user_data);
}

artik_error os_bt_gatt_add_descriptor(int service_id, int char_id,
		artik_bt_gatt_desc desc, int *id)
{
	return bt_gatt_add_descriptor(service_id, char_id, desc, id);
}

artik_error os_bt_gatt_set_desc_on_read_request(int svc_id, int char_id, int desc_id,
		artik_bt_gatt_req_read callback, void *user_data)
{
	return bt_gatt_set_desc_on_read_request(svc_id, char_id, desc_id, callback, user_data);
}

artik_error os_bt_gatt_set_desc_on_write_request(int svc_id, int char_id, int desc_id,
		artik_bt_gatt_req_write callback, void *user_data)
{
	return bt_gatt_set_desc_on_write_request(svc_id, char_id, desc_id, callback, user_data);
}


artik_error os_bt_gatt_register_service(int id)
{
	return bt_gatt_register_service(id);
}

artik_error os_bt_gatt_unregister_service(int id)
{
	return bt_gatt_unregister_service(id);
}

artik_error os_bt_gatt_get_service_list(const char *addr,
		artik_bt_uuid **uuid_list, int *len)
{
	return bt_gatt_get_service_list(addr, uuid_list, len);
}

artik_error os_bt_gatt_get_chracteristic_list(const char *addr,
		const char *srv_uuid, artik_bt_uuid **uuid_list, int *len)
{
	return bt_gatt_get_chracteristic_list(addr, srv_uuid, uuid_list, len);
}

artik_error os_bt_gatt_get_descriptor_list(const char *addr, const char *srv_uuid,
		const char *char_uuid, artik_bt_uuid **uuid_list, int *len)
{
	return bt_gatt_get_descriptor_list(addr, srv_uuid, char_uuid, uuid_list, len);
}

artik_error os_bt_gatt_char_read_value(const char *addr, const char *srv_uuid, const char *char_uuid,
		unsigned char **byte, int *byte_len)
{
	return bt_gatt_char_read_value(addr, srv_uuid, char_uuid, byte, byte_len);
}

artik_error os_bt_gatt_char_write_value(const char *addr, const char *srv_uuid,
		const char *char_uuid, const unsigned char byte[], int byte_len)
{
	return bt_gatt_char_write_value(addr, srv_uuid, char_uuid, byte, byte_len);
}

artik_error os_bt_gatt_desc_read_value(const char *addr, const char *srv_uuid, const char *char_uuid,
		const char *desc_uuid, unsigned char **byte, int *byte_len)
{
	return bt_gatt_desc_read_value(addr, srv_uuid, char_uuid, desc_uuid, byte, byte_len);
}

artik_error os_bt_gatt_desc_write_value(const char *addr, const char *srv_uuid,
		const char *char_uuid, const char *desc_uuid, const unsigned char byte[], int byte_len)
{
	return bt_gatt_desc_write_value(addr, srv_uuid, char_uuid, desc_uuid, byte, byte_len);
}

artik_error os_bt_gatt_start_notify(const char *addr, const char *srv_uuid, const char *char_uuid)
{
	return bt_gatt_start_notify(addr, srv_uuid, char_uuid);
}

artik_error os_bt_gatt_stop_notify(const char *addr, const char *srv_uuid, const char *char_uuid)
{
	return bt_gatt_stop_notify(addr, srv_uuid, char_uuid);
}

artik_error os_bt_gatt_get_char_properties(const char *addr, const char *srv_uuid,
		const char *char_uuid, artik_bt_gatt_char_properties *properties)
{
	return bt_gatt_get_char_properties(addr, srv_uuid, char_uuid, properties);
}

artik_error os_bt_register_advertisement(artik_bt_advertisement *user_adv,
		int *id)
{
	return bt_register_advertisement(user_adv, id);
}

artik_error os_bt_unregister_advertisement(int id)
{
	return bt_unregister_advertisement(id);
}

artik_error os_bt_gatt_req_set_value(artik_bt_gatt_req req,
		int len, const unsigned char *value)
{
	return bt_gatt_req_set_value(req, len, value);
}

artik_error os_bt_gatt_req_set_result(artik_bt_gatt_req req,
		artik_bt_gatt_req_state_type state, const char *err_msg)

{
	return bt_gatt_req_set_result(req, state, err_msg);
}

artik_error os_bt_gatt_notify(int svc_id, int char_id, unsigned char *byte, int len)
{
	return bt_gatt_notify(svc_id, char_id, byte, len);
}

artik_error os_bt_a2dp_source_register(unsigned char codec,
		bool delay_reporting, const char *path,
		const unsigned char *capabilities, int cap_size)
{
	return bt_a2dp_source_register(codec, delay_reporting,
			path, capabilities, cap_size);
}

artik_error os_bt_a2dp_source_unregister(void)
{
	return bt_a2dp_source_unregister();
}

artik_error os_bt_a2dp_source_acquire(int *fd,
		unsigned short *mtu_read, unsigned short *mtu_write)
{
	return bt_a2dp_source_acquire(fd, mtu_read, mtu_write);
}

artik_error os_bt_a2dp_source_get_properties(
		artik_bt_a2dp_source_property * *properties)
{
	return bt_a2dp_source_get_properties(properties);
}

artik_error os_bt_a2dp_source_release(void)
{
	return bt_a2dp_source_release();
}

artik_error os_bt_a2dp_source_set_callback(select_config_callback select_func,
		set_config_callback set_func,
		clear_config_callback clear_func)
{
	return bt_a2dp_source_set_callback(select_func, set_func, clear_func);
}

artik_error os_bt_avrcp_controller_change_folder(const char *folder)
{
	return bt_avrcp_controller_change_folder(folder);
}

artik_error os_bt_avrcp_controller_list_item(int start_item, int end_item,
		artik_bt_avrcp_item **item_list)
{
	return bt_avrcp_controller_list_item(start_item, end_item, item_list);
}

artik_error os_bt_avrcp_controller_get_repeat(char **repeat_mode)
{
	return bt_avrcp_controller_get_repeat(repeat_mode);
}

artik_error os_bt_avrcp_controller_set_repeat(const char *repeat_mode)
{
	return bt_avrcp_controller_set_repeat(repeat_mode);
}

artik_error os_bt_avrcp_controller_is_connected(bool *is_connected)
{
	return bt_avrcp_controller_is_connected(is_connected);
}

artik_error os_bt_avrcp_controller_resume_play(void)
{
	return bt_avrcp_controller_resume_play();
}

artik_error os_bt_avrcp_controller_pause(void)
{
	return bt_avrcp_controller_pause();
}

artik_error os_bt_avrcp_controller_stop(void)
{
	return bt_avrcp_controller_stop();
}

artik_error os_bt_avrcp_controller_next(void)
{
	return bt_avrcp_controller_next();
}

artik_error os_bt_avrcp_controller_previous(void)
{
	return bt_avrcp_controller_previous();
}

artik_error os_bt_avrcp_controller_fast_forward(void)
{
	return bt_avrcp_controller_fast_forward();
}

artik_error os_bt_avrcp_controller_rewind(void)
{
	return bt_avrcp_controller_rewind();
}

artik_error os_bt_avrcp_controller_get_property(char *item,
		artik_bt_avrcp_item_property **properties)
{
	return bt_avrcp_controller_get_property(item, properties);
}

artik_error os_bt_avrcp_controller_play_item(char *item)
{
	return bt_avrcp_controller_play_item(item);
}

artik_error os_bt_avrcp_controller_add_to_playing(char *item)
{
	return bt_avrcp_controller_add_to_playing(item);
}

artik_error os_bt_avrcp_controller_get_name(char **name)
{
	return bt_avrcp_controller_get_name(name);
}

artik_error os_bt_avrcp_controller_get_status(char **status)
{
	return bt_avrcp_controller_get_status(status);
}

artik_error os_bt_avrcp_controller_get_subtype(char **subtype)
{
	return bt_avrcp_controller_get_subtype(subtype);
}

artik_error os_bt_avrcp_controller_get_type(char **type)
{
	return bt_avrcp_controller_get_type(type);
}

artik_error os_bt_avrcp_controller_get_browsable(bool *is_browsable)
{
	return bt_avrcp_controller_get_browsable(is_browsable);
}

artik_error os_bt_avrcp_controller_get_position(unsigned int *position)
{
	return bt_avrcp_controller_get_position(position);
}

artik_error os_bt_pan_register(const char *uuid, const char *bridge)
{
	return bt_pan_register(uuid, bridge);
}

artik_error os_bt_pan_unregister(const char *uuid)
{
	return bt_pan_unregister(uuid);
}

artik_error os_bt_pan_connect(const char *mac_addr,
		const char *uuid, char **network_interface)
{
	return bt_pan_connect(mac_addr, uuid, network_interface);
}

artik_error os_bt_pan_disconnect(void)
{
	return bt_pan_disconnect();
}

artik_error os_bt_pan_get_connected(bool *connected)
{
	return bt_pan_get_connected(connected);
}

artik_error os_bt_pan_get_interface(char **_interface)
{
	return bt_pan_get_interface(_interface);
}

artik_error os_bt_pan_get_UUID(char **uuid)
{
	return bt_pan_get_UUID(uuid);
}

artik_error os_bt_spp_register_profile(artik_bt_spp_profile_option *opt)
{
	return bt_spp_register_profile(opt);
}

artik_error os_bt_spp_unregister_profile(void)
{
	return bt_spp_unregister_profile();
}

artik_error os_bt_spp_set_callback(release_callback release_func,
		new_connection_callback connect_func,
		request_disconnect_callback disconnect_func,
		void *user_data) {
	return bt_spp_set_callback(release_func, connect_func, disconnect_func, user_data);

}

artik_error os_bt_ftp_create_session(char *dest_addr)
{
	return bt_ftp_create_session(dest_addr);
}

artik_error os_bt_ftp_remove_session(void)
{
	return bt_ftp_remove_session();
}

artik_error os_bt_ftp_change_folder(char *folder)
{
	return bt_ftp_change_folder(folder);
}

artik_error os_bt_ftp_create_folder(char *folder)
{
	return bt_ftp_create_folder(folder);
}

artik_error os_bt_ftp_delete_file(char *file)
{
	return bt_ftp_delete_file(file);
}

artik_error os_bt_ftp_list_folder(artik_bt_ftp_file **file_list)
{
	return bt_ftp_list_folder(file_list);
}

artik_error os_bt_ftp_get_file(char *target_file, char *source_file)
{
	return bt_ftp_get_file(target_file, source_file);
}

artik_error os_bt_ftp_put_file(char *source_file, char *target_file)
{
	return bt_ftp_put_file(source_file, target_file);
}

artik_error os_bt_ftp_cancel_transfer(void)
{
	return bt_ftp_cancel_transfer();
}

artik_error os_bt_ftp_resume_transfer(void)
{
	return bt_ftp_resume_transfer();
}

artik_error os_bt_ftp_suspend_transfer(void)
{
	return bt_ftp_suspend_transfer();
}

artik_error os_bt_agent_register_capability(artik_bt_agent_capability e)
{
	return bt_agent_register_capability(e);
}

artik_error os_bt_agent_set_default(void)
{
	return bt_agent_set_default();
}

artik_error os_bt_agent_unregister(void)
{
	return bt_agent_unregister();
}

artik_error os_bt_agent_set_callback(artik_bt_agent_callbacks *agent_callback)
{
	return bt_agent_set_callback(agent_callback);
}

artik_error os_bt_agent_send_pincode(artik_bt_agent_request_handle handle, char *pincode)
{
	return bt_agent_send_pincode(handle, pincode);
}

artik_error os_bt_agent_send_passkey(artik_bt_agent_request_handle handle, unsigned int passkey)
{
	return bt_agent_send_passkey(handle, passkey);
}

artik_error os_bt_agent_send_error(artik_bt_agent_request_handle handle, artik_bt_agent_request_error e,
		const char *err_msg)
{
	return bt_agent_send_error(handle, e, err_msg);
}

artik_error os_bt_agent_send_empty_response(artik_bt_agent_request_handle handle)
{
	return bt_agent_send_empty_response(handle);
}
