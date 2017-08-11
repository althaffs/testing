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

#include <string.h>
#include <stdlib.h>
#include <artik_log.h>
#include <artik_bluetooth.h>
#include <bt.h>
#include "os_bluetooth.h"

static artik_error artik_bluetooth_start_scan(void);
static artik_error artik_bluetooth_stop_scan(void);
static artik_error artik_bluetooth_get_devices(artik_bt_device **devices,
		int *num_devices);
static artik_error artik_bluetooth_get_paired_devices(artik_bt_device **
		devices, int *num_devices);
static artik_error artik_bluetooth_get_connected_devices(artik_bt_device **
		devices, int *num_devices);
static artik_error artik_bluetooth_start_bond(const char *addr);
static artik_error artik_bluetooth_stop_bond(const char *addr);
static artik_error artik_bluetooth_connect(const char *addr);
static artik_error artik_bluetooth_disconnect(const char *addr);
static artik_error artik_bluetooth_free_devices(artik_bt_device *device_list,
		int count);
static artik_error artik_bluetooth_set_callback(artik_bt_event event,
		artik_bt_callback user_callback, void *user_data);
static artik_error artik_bluetooth_unset_callback(artik_bt_event event);
static artik_error artik_bluetooth_remove_unpaired_devices(void);
static artik_error artik_bluetooth_remove_device(const char *remote_address);
static artik_error artik_bluetooth_set_scan_filter(
		artik_bt_scan_filter * filter);
static artik_error artik_bluetooth_set_alias(const char *alias);
static artik_error artik_bluetooth_set_discoverable(bool discoverable);
static artik_error artik_bluetooth_set_pairable(bool pairable);
static artik_error artik_bluetooth_set_pairableTimeout(unsigned int timeout);
static artik_error artik_bluetooth_set_discoverableTimeout(
		unsigned int timeout);
static bool artik_bluetooth_is_scanning(void);
static artik_error artik_bluetooth_get_device_property(const char *addr,
		const char *property, char **value);
static artik_error artik_bluetooth_get_adapter_info(artik_bt_adapter *adapter);
static artik_error artik_bluetooth_remove_devices(void);
static artik_error artik_bluetooth_connect_profile(const char *addr,
		const char *uuid);
static artik_error artik_bluetooth_set_trust(const char *addr);
static artik_error artik_bluetooth_unset_trust(const char *addr);
static artik_error artik_bluetooth_set_block(const char *addr);
static artik_error artik_bluetooth_unset_block(const char *addr);
static artik_error artik_bluetooth_free_device(artik_bt_device *device);
static bool artik_bluetooth_is_paired(const char *addr);
static bool artik_bluetooth_is_connected(const char *addr);
static bool artik_bluetooth_is_trusted(const char *addr);
static bool artik_bluetooth_is_blocked(const char *addr);
static artik_error artik_bluetooth_gatt_add_service(artik_bt_gatt_service svc,
		int *id);
static artik_error artik_bluetooth_gatt_add_characteristic(int svc_id,
		artik_bt_gatt_chr chr, int *id);
static artik_error artik_bluetooth_gatt_set_chr_on_read_request(int svc_id, int char_id,
		artik_bt_gatt_req_read callback, void *user_data);
static artik_error artik_bluetooth_gatt_set_chr_on_write_request(int svc_id, int char_id,
		artik_bt_gatt_req_write callback, void *user_data);
static artik_error artik_bluetooth_gatt_set_chr_on_notify_request(int svc_id, int char_id,
		artik_bt_gatt_req_notify callback, void *user_data);
static artik_error artik_bluetooth_gatt_add_descriptor(int service_id,
		int char_id, artik_bt_gatt_desc desc, int *id);
static artik_error artik_bluetooth_gatt_set_desc_on_read_request(int svc_id, int char_id,
		int desc_id, artik_bt_gatt_req_read callback, void *user_data);
static artik_error artik_bluetooth_gatt_set_desc_on_write_request(int svc_id, int char_id,
		int desc_id, artik_bt_gatt_req_write callback, void *user_data);
static artik_error artik_bluetooth_gatt_register_service(int id);
static artik_error artik_bluetooth_gatt_unregister_service(int id);
static artik_error artik_bluetooth_gatt_get_service_list(const char *addr,
		artik_bt_uuid **uuid_list, int *len);
static artik_error artik_bluetooth_gatt_get_chracteristic_list(const char *addr,
		const char *srv_uuid, artik_bt_uuid **uuid_list, int *len);
static artik_error artik_bluetooth_gatt_get_descriptor_list(const char *addr, const char *srv_uuid,
		const char *char_uuid, artik_bt_uuid **uuid_list, int *len);
static artik_error artik_bluetooth_gatt_char_read_value(const char *addr, const char *srv_uuid,
		const char *char_uuid, unsigned char **byte, int *byte_len);
static artik_error artik_bluetooth_gatt_char_write_value(const char *addr, const char *srv_uuid,
		const char *char_uuid, const unsigned char byte[], int byte_len);
static artik_error artik_bluetooth_gatt_desc_read_value(const char *addr, const char *srv_uuid,
		const char *char_uuid, const char *desc_uuid, unsigned char **byte, int *byte_len);
static artik_error artik_bluetooth_gatt_desc_write_value(const char *addr, const char *srv_uuid,
		const char *char_uuid, const char *desc_uuid, const unsigned char byte[], int byte_len);
static artik_error artik_bluetooth_gatt_start_notify(const char *addr, const char *srv_uuid,
		const char *char_uuid);
static artik_error artik_bluetooth_gatt_stop_notify(const char *addr, const char *srv_uuid,
		const char *char_uuid);
static artik_error artik_bluetooth_gatt_get_char_properties(const char *addr, const char *srv_uuid,
		const char *char_uuid, artik_bt_gatt_char_properties *properties);
static artik_error artik_bluetooth_register_advertisement(
		artik_bt_advertisement * user_adv, int *id);
static artik_error artik_bluetooth_unregister_advertisement(int id);
static artik_error artik_bluetooth_gatt_req_set_value(artik_bt_gatt_req req, int len,
		const unsigned char *value);
static artik_error artik_bluetooth_gatt_req_set_result(artik_bt_gatt_req req,
		artik_bt_gatt_req_state_type state, const char *err_msg);
static artik_error artik_bluetooth_gatt_notify(int svc_id, int char_id,
		unsigned char *byte, int len);
static artik_error artik_bluetooth_avrcp_controller_change_folder(
		const char *folder);
static artik_error artik_bluetooth_avrcp_controller_list_item(int start_item,
		int end_item, artik_bt_avrcp_item **item_list);
static artik_error artik_bluetooth_avrcp_controller_get_repeat(
		artik_bt_avrcp_repeat_mode * repeat_mode);
static artik_error artik_bluetooth_avrcp_controller_set_repeat(
		artik_bt_avrcp_repeat_mode repeat_mode);
static artik_error artik_bluetooth_avrcp_controller_is_connected(
		bool *is_connected);
static artik_error artik_bluetooth_avrcp_controller_resume_play(void);
static artik_error artik_bluetooth_avrcp_controller_pause(void);
static artik_error artik_bluetooth_avrcp_controller_stop(void);
static artik_error artik_bluetooth_avrcp_controller_next(void);
static artik_error artik_bluetooth_avrcp_controller_previous(void);
static artik_error artik_bluetooth_avrcp_controller_fast_forward(void);
static artik_error artik_bluetooth_avrcp_controller_rewind(void);
static artik_error artik_bluetooth_avrcp_controller_get_property(char *item,
		artik_bt_avrcp_item_property **properties);
static artik_error artik_bluetooth_avrcp_controller_play_item(char *item);
static artik_error artik_bluetooth_avrcp_controller_add_to_playing(char *item);
static artik_error artik_bluetooth_avrcp_controller_get_name(char **name);
static artik_error artik_bluetooth_avrcp_controller_get_status(char **status);
static artik_error artik_bluetooth_avrcp_controller_get_subtype(
		char **subtype);
static artik_error artik_bluetooth_avrcp_controller_get_type(char **type);
static artik_error artik_bluetooth_avrcp_controller_get_browsable(
		bool *is_browsable);
static artik_error artik_bluetooth_avrcp_controller_get_position(
		unsigned int *position);
static artik_error artik_bluetooth_pan_register(const char *uuid,
		const char *bridge);
static artik_error artik_bluetooth_pan_unregister(const char *uuid);
static artik_error artik_bluetooth_pan_connect(const char *mac_addr,
		const char *uuid, char **network_interface);
static artik_error artik_bluetooth_pan_disconnect(void);
static artik_error artik_bluetooth_pan_get_connected(bool *connected);
static artik_error artik_bluetooth_pan_get_interface(char **_interface);
static artik_error artik_bluetooth_pan_get_UUID(char **uuid);
static artik_error artik_bluetooth_spp_register_profile(
		artik_bt_spp_profile_option * opt);
static artik_error artik_bluetooth_spp_unregister_profile(void);
static artik_error artik_bluetooth_spp_set_callback(
		release_callback release_func, new_connection_callback connect_func,
		request_disconnect_callback disconnect_func,
		void *user_data);
static artik_error artik_bluetooth_ftp_create_session(char *dest_addr);
static artik_error artik_bluetooth_ftp_remove_session(void);
static artik_error artik_bluetooth_ftp_change_folder(char *folder);
static artik_error artik_bluetooth_ftp_create_folder(char *folder);
static artik_error artik_bluetooth_ftp_delete_file(char *file);
static artik_error artik_bluetooth_ftp_list_folder(
		artik_bt_ftp_file * *file_list);
static artik_error artik_bluetooth_ftp_get_file(char *target_file,
		char *source_file);
static artik_error artik_bluetooth_ftp_put_file(char *source_file,
		char *target_file);
static artik_error artik_bluetooth_ftp_resume_transfer(void);
static artik_error artik_bluetooth_ftp_suspend_transfer(void);
static artik_error artik_bluetooth_agent_register_capability(
	artik_bt_agent_capability e);
static artik_error artik_bluetooth_agent_set_default(void);
static artik_error artik_bluetooth_agent_unregister(void);
static artik_error artik_bluetooth_agent_set_callback(
	artik_bt_agent_callbacks * agent_callback);
static artik_error artik_bluetooth_agent_send_pincode(artik_bt_agent_request_handle handle, char *pincode);
static artik_error artik_bluetooth_agent_send_passkey(artik_bt_agent_request_handle handle, unsigned int passkey);
static artik_error artik_bluetooth_agent_send_error(artik_bt_agent_request_handle handle,
		artik_bt_agent_request_error e, const char *err_msg);
static artik_error artik_bluetooth_agent_send_empty_response(artik_bt_agent_request_handle handle);
const artik_bluetooth_module bluetooth_module = {
	artik_bluetooth_start_scan,
	artik_bluetooth_stop_scan,
	artik_bluetooth_get_devices,
	artik_bluetooth_get_paired_devices,
	artik_bluetooth_get_connected_devices,
	artik_bluetooth_start_bond,
	artik_bluetooth_stop_bond,
	artik_bluetooth_connect,
	artik_bluetooth_disconnect,
	artik_bluetooth_free_devices,
	artik_bluetooth_set_callback,
	artik_bluetooth_unset_callback,
	artik_bluetooth_remove_unpaired_devices,
	artik_bluetooth_remove_device,
	artik_bluetooth_set_scan_filter,
	artik_bluetooth_set_alias,
	artik_bluetooth_set_discoverable,
	artik_bluetooth_set_pairable,
	artik_bluetooth_set_pairableTimeout,
	artik_bluetooth_set_discoverableTimeout,
	artik_bluetooth_is_scanning,
	artik_bluetooth_get_device_property,
	artik_bluetooth_get_adapter_info,
	artik_bluetooth_remove_devices,
	artik_bluetooth_connect_profile,
	artik_bluetooth_set_trust,
	artik_bluetooth_unset_trust,
	artik_bluetooth_set_block,
	artik_bluetooth_unset_block,
	artik_bluetooth_free_device,
	artik_bluetooth_is_paired,
	artik_bluetooth_is_connected,
	artik_bluetooth_is_trusted,
	artik_bluetooth_is_blocked,
	artik_bluetooth_gatt_add_service,
	artik_bluetooth_gatt_add_characteristic,
	artik_bluetooth_gatt_set_chr_on_read_request,
	artik_bluetooth_gatt_set_chr_on_write_request,
	artik_bluetooth_gatt_set_chr_on_notify_request,
	artik_bluetooth_gatt_add_descriptor,
	artik_bluetooth_gatt_set_desc_on_read_request,
	artik_bluetooth_gatt_set_desc_on_write_request,
	artik_bluetooth_gatt_register_service,
	artik_bluetooth_gatt_unregister_service,
	artik_bluetooth_gatt_get_service_list,
	artik_bluetooth_gatt_get_chracteristic_list,
	artik_bluetooth_gatt_get_descriptor_list,
	artik_bluetooth_gatt_char_read_value,
	artik_bluetooth_gatt_char_write_value,
	artik_bluetooth_gatt_desc_read_value,
	artik_bluetooth_gatt_desc_write_value,
	artik_bluetooth_gatt_start_notify,
	artik_bluetooth_gatt_stop_notify,
	artik_bluetooth_gatt_get_char_properties,
	artik_bluetooth_register_advertisement,
	artik_bluetooth_unregister_advertisement,
	artik_bluetooth_gatt_req_set_value,
	artik_bluetooth_gatt_req_set_result,
	artik_bluetooth_gatt_notify,
	artik_bluetooth_avrcp_controller_change_folder,
	artik_bluetooth_avrcp_controller_list_item,
	artik_bluetooth_avrcp_controller_get_repeat,
	artik_bluetooth_avrcp_controller_set_repeat,
	artik_bluetooth_avrcp_controller_is_connected,
	artik_bluetooth_avrcp_controller_resume_play,
	artik_bluetooth_avrcp_controller_pause,
	artik_bluetooth_avrcp_controller_stop,
	artik_bluetooth_avrcp_controller_next,
	artik_bluetooth_avrcp_controller_previous,
	artik_bluetooth_avrcp_controller_fast_forward,
	artik_bluetooth_avrcp_controller_rewind,
	artik_bluetooth_avrcp_controller_get_property,
	artik_bluetooth_avrcp_controller_play_item,
	artik_bluetooth_avrcp_controller_add_to_playing,
	artik_bluetooth_avrcp_controller_get_name,
	artik_bluetooth_avrcp_controller_get_status,
	artik_bluetooth_avrcp_controller_get_subtype,
	artik_bluetooth_avrcp_controller_get_type,
	artik_bluetooth_avrcp_controller_get_browsable,
	artik_bluetooth_avrcp_controller_get_position,
	artik_bluetooth_pan_register,
	artik_bluetooth_pan_unregister,
	artik_bluetooth_pan_connect,
	artik_bluetooth_pan_disconnect,
	artik_bluetooth_pan_get_connected,
	artik_bluetooth_pan_get_interface,
	artik_bluetooth_pan_get_UUID,
	artik_bluetooth_spp_register_profile,
	artik_bluetooth_spp_unregister_profile,
	artik_bluetooth_spp_set_callback,
	artik_bluetooth_ftp_create_session,
	artik_bluetooth_ftp_remove_session,
	artik_bluetooth_ftp_change_folder,
	artik_bluetooth_ftp_create_folder,
	artik_bluetooth_ftp_delete_file,
	artik_bluetooth_ftp_list_folder,
	artik_bluetooth_ftp_get_file,
	artik_bluetooth_ftp_put_file,
	artik_bluetooth_ftp_resume_transfer,
	artik_bluetooth_ftp_suspend_transfer,
	artik_bluetooth_agent_register_capability,
	artik_bluetooth_agent_set_default,
	artik_bluetooth_agent_unregister,
	artik_bluetooth_agent_set_callback,
	artik_bluetooth_agent_send_pincode,
	artik_bluetooth_agent_send_passkey,
	artik_bluetooth_agent_send_error,
	artik_bluetooth_agent_send_empty_response
};

artik_error artik_bluetooth_set_scan_filter(artik_bt_scan_filter *filter)
{
	if (!filter)
		return E_BAD_ARGS;

	return os_bt_set_scan_filter(filter);
}

artik_error artik_bluetooth_set_alias(const char *alias)
{
	if (!alias)
		return E_BAD_ARGS;

	return os_bt_set_alias(alias);
}

artik_error artik_bluetooth_set_discoverable(bool discoverable)
{
	return os_bt_set_discoverable(discoverable);
}

artik_error artik_bluetooth_set_pairable(bool pairable)
{
	return os_bt_set_pairable(pairable);
}

artik_error artik_bluetooth_set_pairableTimeout(unsigned int timeout)
{
	return os_bt_set_pairableTimeout(timeout);
}

artik_error artik_bluetooth_set_discoverableTimeout(unsigned int timeout)
{
	return os_bt_set_discoverableTimeout(timeout);
}

bool artik_bluetooth_is_scanning(void)
{
	return os_bt_is_scanning();
}

artik_error artik_bluetooth_get_device_property(const char *addr,
		const char *property, char **value)
{
	if (!addr || !property || !value)
		return E_BAD_ARGS;

	return os_bt_get_device_property(addr, property, value);
}

artik_error artik_bluetooth_get_adapter_info(artik_bt_adapter *adapter)
{
	if (!adapter)
		return E_BAD_ARGS;

	return os_bt_get_adapter_info(adapter);
}

artik_error artik_bluetooth_start_scan(void)
{
	return os_bt_start_scan();
}

artik_error artik_bluetooth_stop_scan(void)
{
	return os_bt_stop_scan();
}

artik_error artik_bluetooth_get_devices(artik_bt_device **devices,
					int *num_devices)
{
	if (!devices || !num_devices)
		return E_BAD_ARGS;

	return os_bt_get_devices(devices, num_devices);
}

artik_error artik_bluetooth_get_paired_devices(artik_bt_device **devices,
					       int *num_devices)
{
	if (!devices || !num_devices)
		return E_BAD_ARGS;

	return os_bt_get_paired_devices(devices, num_devices);
}

artik_error artik_bluetooth_get_connected_devices(artik_bt_device **devices,
						  int *num_devices)
{
	if (!devices || !num_devices)
		return E_BAD_ARGS;

	return os_bt_get_connected_devices(devices, num_devices);
}

artik_error artik_bluetooth_start_bond(const char *addr)
{
	if (!addr)
		return E_BAD_ARGS;

	return os_bt_start_bond(addr);
}

artik_error artik_bluetooth_stop_bond(const char *addr)
{
	if (!addr)
		return E_BAD_ARGS;

	return os_bt_stop_bond(addr);
}

artik_error artik_bluetooth_connect(const char *addr)
{
	if (!addr)
		return E_BAD_ARGS;

	return os_bt_connect(addr);
}

artik_error artik_bluetooth_disconnect(const char *addr)
{
	if (!addr)
		return E_BAD_ARGS;

	return os_bt_disconnect(addr);
}

artik_error artik_bluetooth_free_devices(artik_bt_device *device_list,
					 int count)
{
	if (!device_list || (count < 0))
		return E_BAD_ARGS;

	return os_bt_free_devices(device_list, count);
}

artik_error artik_bluetooth_set_callback(artik_bt_event event,
					 artik_bt_callback user_callback,
					 void *user_data)
{
	if ((event >= BT_EVENT_END) || !user_callback)
		return E_BAD_ARGS;

	return os_bt_set_callback(event, user_callback, user_data);
}

artik_error artik_bluetooth_unset_callback(artik_bt_event event)
{
	if (event >= BT_EVENT_END)
		return E_BAD_ARGS;

	return os_bt_unset_callback(event);
}

artik_error artik_bluetooth_remove_unpaired_devices(void)
{
	return os_bt_remove_unpaired_devices();
}

artik_error artik_bluetooth_remove_device(const char *remote_address)
{
	if (!remote_address)
		return E_BAD_ARGS;

	return os_bt_remove_device(remote_address);
}

artik_error artik_bluetooth_remove_devices(void)
{
	return os_bt_remove_devices();
}

artik_error artik_bluetooth_connect_profile(const char *addr, const char *uuid)
{
	if (!addr || !uuid)
		return E_BAD_ARGS;

	return os_bt_connect_profile(addr, uuid);
}

artik_error artik_bluetooth_set_trust(const char *addr)
{
	if (!addr)
		return E_BAD_ARGS;

	return os_bt_set_trust(addr);
}

artik_error artik_bluetooth_unset_trust(const char *addr)
{
	if (!addr)
		return E_BAD_ARGS;

	return os_bt_unset_trust(addr);
}

artik_error artik_bluetooth_set_block(const char *addr)
{
	if (!addr)
		return E_BAD_ARGS;

	return os_bt_set_block(addr);
}

artik_error artik_bluetooth_unset_block(const char *addr)
{
	if (!addr)
		return E_BAD_ARGS;

	return os_bt_unset_block(addr);
}

artik_error artik_bluetooth_free_device(artik_bt_device *device)
{
	if (!device)
		return E_BAD_ARGS;

	return os_bt_free_device(device);
}

bool artik_bluetooth_is_paired(const char *addr)
{
	if (!addr)
		return E_BAD_ARGS;

	return os_bt_is_paired(addr);
}

bool artik_bluetooth_is_connected(const char *addr)
{
	if (!addr)
		return E_BAD_ARGS;

	return os_bt_is_connected(addr);
}

bool artik_bluetooth_is_trusted(const char *addr)
{
	if (!addr)
		return E_BAD_ARGS;

	return os_bt_is_trusted(addr);
}

bool artik_bluetooth_is_blocked(const char *addr)
{
	if (!addr)
		return E_BAD_ARGS;

	return os_bt_is_blocked(addr);
}

artik_error artik_bluetooth_gatt_add_service(artik_bt_gatt_service svc, int *id)
{
	if (!id)
		return E_BAD_ARGS;

	return os_bt_gatt_add_service(svc, id);
}

artik_error artik_bluetooth_gatt_add_characteristic(int service_id,
		artik_bt_gatt_chr chr, int *id)
{
	if (!id)
		return E_BAD_ARGS;

	return os_bt_gatt_add_characteristic(service_id, chr, id);
}

artik_error artik_bluetooth_gatt_set_chr_on_read_request(int svc_id, int char_id,
		artik_bt_gatt_req_read callback, void *user_data)
{
	return os_bt_gatt_set_chr_on_read_request(svc_id, char_id, callback, user_data);
}

artik_error artik_bluetooth_gatt_set_chr_on_write_request(int svc_id, int char_id,
		artik_bt_gatt_req_write callback, void *user_data)
{
	return os_bt_gatt_set_chr_on_write_request(svc_id, char_id, callback, user_data);
}

artik_error artik_bluetooth_gatt_set_chr_on_notify_request(int svc_id, int char_id,
		artik_bt_gatt_req_notify callback, void *user_data)
{
	return os_bt_gatt_set_chr_on_notify_request(svc_id, char_id, callback, user_data);
}

artik_error artik_bluetooth_gatt_add_descriptor(int service_id, int char_id,
		artik_bt_gatt_desc desc, int *id)
{
	if (!id)
		return E_BAD_ARGS;

	return os_bt_gatt_add_descriptor(service_id, char_id, desc, id);
}

artik_error artik_bluetooth_gatt_set_desc_on_read_request(int svc_id, int char_id,
		int desc_id, artik_bt_gatt_req_read callback, void *user_data)
{
	return os_bt_gatt_set_desc_on_read_request(svc_id, char_id, desc_id, callback, user_data);
}

artik_error artik_bluetooth_gatt_set_desc_on_write_request(int svc_id, int char_id,
		int desc_id, artik_bt_gatt_req_write callback, void *user_data)
{
	return os_bt_gatt_set_desc_on_write_request(svc_id, char_id, desc_id, callback, user_data);
}

artik_error artik_bluetooth_gatt_register_service(int id)
{
	return os_bt_gatt_register_service(id);
}

artik_error artik_bluetooth_gatt_unregister_service(int id)
{
	return os_bt_gatt_unregister_service(id);
}

artik_error artik_bluetooth_gatt_get_service_list(const char *addr,
		artik_bt_uuid **uuid_list, int *len)
{
	if (!addr || !uuid_list || !len)
		return E_BAD_ARGS;

	return os_bt_gatt_get_service_list(addr, uuid_list, len);
}

artik_error artik_bluetooth_gatt_get_chracteristic_list(const char *addr,
		const char *srv_uuid, artik_bt_uuid **uuid_list, int *len)
{
	if (!addr || !srv_uuid || !uuid_list || !len)
		return E_BAD_ARGS;

	return os_bt_gatt_get_chracteristic_list(addr, srv_uuid, uuid_list, len);
}

artik_error artik_bluetooth_gatt_get_descriptor_list(const char *addr,
		const char *srv_uuid, const char *char_uuid, artik_bt_uuid **uuid_list, int *len)
{
	if (!addr || !srv_uuid || !char_uuid || !uuid_list || !len)
		return E_BAD_ARGS;

	return os_bt_gatt_get_descriptor_list(addr, srv_uuid, char_uuid, uuid_list, len);
}

artik_error artik_bluetooth_gatt_char_read_value(const char *addr,
		const char *srv_uuid, const char *char_uuid, unsigned char **byte, int *byte_len)
{
	if (!addr || !srv_uuid || !char_uuid || !byte || !byte_len)
		return E_BAD_ARGS;

	return os_bt_gatt_char_read_value(addr, srv_uuid, char_uuid, byte, byte_len);
}

artik_error artik_bluetooth_gatt_char_write_value(const char *addr, const char *srv_uuid,
		const char *char_uuid, const unsigned char byte[], int byte_len)
{
	if (!addr || !srv_uuid || !char_uuid)
		return E_BAD_ARGS;

	return os_bt_gatt_char_write_value(addr, srv_uuid, char_uuid, byte, byte_len);
}

artik_error artik_bluetooth_gatt_desc_read_value(const char *addr, const char *srv_uuid,
		const char *char_uuid, const char *desc_uuid, unsigned char **byte, int *byte_len)
{
	if (!addr || !srv_uuid || !char_uuid || !desc_uuid || !byte || !byte_len)
		return E_BAD_ARGS;

	return os_bt_gatt_desc_read_value(addr, srv_uuid, char_uuid, desc_uuid, byte, byte_len);
}

artik_error artik_bluetooth_gatt_desc_write_value(const char *addr, const char *srv_uuid,
		const char *char_uuid, const char *desc_uuid, const unsigned char byte[], int byte_len)
{
	if (!addr || !srv_uuid || !char_uuid || !desc_uuid)
		return E_BAD_ARGS;

	return os_bt_gatt_desc_write_value(addr, srv_uuid, char_uuid, desc_uuid, byte, byte_len);
}

artik_error artik_bluetooth_gatt_start_notify(const char *addr,
		const char *srv_uuid, const char *char_uuid)
{
	if (!addr || !srv_uuid || !char_uuid)
		return E_BAD_ARGS;

	return os_bt_gatt_start_notify(addr, srv_uuid, char_uuid);
}

artik_error artik_bluetooth_gatt_stop_notify(const char *addr,
		const char *srv_uuid, const char *char_uuid)
{
	if (!addr || !srv_uuid || !char_uuid)
		return E_BAD_ARGS;

	return os_bt_gatt_stop_notify(addr, srv_uuid, char_uuid);
}

artik_error artik_bluetooth_gatt_get_char_properties(const char *addr,
		const char *srv_uuid, const char *char_uuid, artik_bt_gatt_char_properties *properties)
{
	if (!addr || !srv_uuid || !char_uuid)
		return E_BAD_ARGS;

	return os_bt_gatt_get_char_properties(addr, srv_uuid, char_uuid, properties);
}

artik_error artik_bluetooth_register_advertisement(
		artik_bt_advertisement * user_adv, int *id)
{
	if (!user_adv || !id)
		return E_BAD_ARGS;

	return os_bt_register_advertisement(user_adv, id);
}

artik_error artik_bluetooth_unregister_advertisement(int id)
{
	return os_bt_unregister_advertisement(id);
}

artik_error artik_bluetooth_gatt_req_set_value(artik_bt_gatt_req req, int len,
		const unsigned char *value)
{
	if (!req || !value || len <= 0)
		return E_BAD_ARGS;

	return os_bt_gatt_req_set_value(req, len, value);
}
artik_error artik_bluetooth_gatt_req_set_result(artik_bt_gatt_req req,
		artik_bt_gatt_req_state_type state, const char *err_msg)
{
	if (!req)
		return E_BAD_ARGS;

	return os_bt_gatt_req_set_result(req, state, err_msg);
}

artik_error artik_bluetooth_gatt_notify(int svc_id, int char_id,
		unsigned char *byte, int len)
{
	return os_bt_gatt_notify(svc_id, char_id, byte, len);
}

artik_error artik_bluetooth_avrcp_controller_change_folder(const char *folder)
{
	return os_bt_avrcp_controller_change_folder(folder);
}

artik_error artik_bluetooth_avrcp_controller_list_item(int start_item,
		int end_item, artik_bt_avrcp_item **item_list)
{
	return os_bt_avrcp_controller_list_item(start_item, end_item, item_list);
}

artik_error artik_bluetooth_avrcp_controller_get_repeat(
		artik_bt_avrcp_repeat_mode * repeat_mode)
{
	int max_repeat_mode_len = 12;
	char *repeat_mode_str = (char *) malloc(max_repeat_mode_len *
			sizeof(char));
	artik_error ret = S_OK;

	ret = os_bt_avrcp_controller_get_repeat(&repeat_mode_str);
	int str_len = strlen(repeat_mode_str);

	if (strncmp(repeat_mode_str, "singletrack", str_len) == 0)
		*repeat_mode = BT_AVRCP_REPEAT_SINGLETRACK;
	else if (strncmp(repeat_mode_str, "alltracks", str_len) == 0)
		*repeat_mode = BT_AVRCP_REPEAT_ALLTRACKS;
	else if (strncmp(repeat_mode_str, "group", str_len) == 0)
		*repeat_mode = BT_AVRCP_REPEAT_GROUP;
	else if (strncmp(repeat_mode_str, "off", str_len) == 0)
		*repeat_mode = BT_AVRCP_REPEAT_OFF;
	else
		return E_INVALID_VALUE;
	return ret;
}

artik_error artik_bluetooth_avrcp_controller_set_repeat(
		artik_bt_avrcp_repeat_mode repeat_mode)
{
	char *repeat_mode_str = NULL;

	if (repeat_mode == BT_AVRCP_REPEAT_SINGLETRACK)
		repeat_mode_str = "singletrack";
	else if (repeat_mode == BT_AVRCP_REPEAT_ALLTRACKS)
		repeat_mode_str = "alltracks";
	else if (repeat_mode == BT_AVRCP_REPEAT_GROUP)
		repeat_mode_str = "group";
	else if (repeat_mode == BT_AVRCP_REPEAT_OFF)
		repeat_mode_str = "off";
	else
		return E_INVALID_VALUE;

	return os_bt_avrcp_controller_set_repeat(repeat_mode_str);
}

artik_error artik_bluetooth_avrcp_controller_is_connected(bool *is_connected)
{
	return os_bt_avrcp_controller_is_connected(is_connected);
}

artik_error artik_bluetooth_avrcp_controller_resume_play(void)
{
	return os_bt_avrcp_controller_resume_play();
}

artik_error artik_bluetooth_avrcp_controller_pause(void)
{
	return os_bt_avrcp_controller_pause();
}

artik_error artik_bluetooth_avrcp_controller_stop(void)
{
	return os_bt_avrcp_controller_stop();
}

artik_error artik_bluetooth_avrcp_controller_next(void)
{
	return os_bt_avrcp_controller_next();
}

artik_error artik_bluetooth_avrcp_controller_previous(void)
{
	return os_bt_avrcp_controller_previous();
}

artik_error artik_bluetooth_avrcp_controller_fast_forward(void)
{
	return os_bt_avrcp_controller_fast_forward();
}

artik_error artik_bluetooth_avrcp_controller_rewind(void)
{
	return os_bt_avrcp_controller_rewind();
}

artik_error artik_bluetooth_avrcp_controller_get_property(char *item,
		artik_bt_avrcp_item_property **properties)
{
	return os_bt_avrcp_controller_get_property(item, properties);
}

artik_error artik_bluetooth_avrcp_controller_play_item(char *item)
{
	return os_bt_avrcp_controller_play_item(item);
}

artik_error artik_bluetooth_avrcp_controller_add_to_playing(char *item)
{
	return os_bt_avrcp_controller_add_to_playing(item);
}

artik_error artik_bluetooth_avrcp_controller_get_name(char **name)
{
	return os_bt_avrcp_controller_get_name(name);
}

artik_error artik_bluetooth_avrcp_controller_get_status(char **status)
{
	return os_bt_avrcp_controller_get_status(status);
}

artik_error artik_bluetooth_avrcp_controller_get_subtype(char **subtype)
{
	return os_bt_avrcp_controller_get_subtype(subtype);
}

artik_error artik_bluetooth_avrcp_controller_get_type(char **type)
{
	return os_bt_avrcp_controller_get_type(type);
}

artik_error artik_bluetooth_avrcp_controller_get_browsable(bool *is_browsable)
{
	return os_bt_avrcp_controller_get_browsable(is_browsable);
}

artik_error artik_bluetooth_avrcp_controller_get_position(
		unsigned int *position)
{
	return os_bt_avrcp_controller_get_position(position);
}

artik_error artik_bluetooth_pan_register(const char *uuid, const char *bridge)
{
	return os_bt_pan_register(uuid, bridge);
}

artik_error artik_bluetooth_pan_unregister(const char *uuid)
{
	return os_bt_pan_unregister(uuid);
}

artik_error artik_bluetooth_pan_connect(const char *mac_addr,
		const char *uuid, char **network_interface)
{
	return os_bt_pan_connect(mac_addr, uuid, network_interface);
}

artik_error artik_bluetooth_pan_disconnect(void)
{
	return os_bt_pan_disconnect();
}

artik_error artik_bluetooth_pan_get_connected(bool *connected)
{
	return os_bt_pan_get_connected(connected);
}

artik_error artik_bluetooth_pan_get_interface(char **_interface)
{
	return os_bt_pan_get_interface(_interface);
}

artik_error artik_bluetooth_pan_get_UUID(char **uuid)
{
	return os_bt_pan_get_UUID(uuid);
}

artik_error artik_bluetooth_spp_register_profile(
		artik_bt_spp_profile_option * opt)
{
	return os_bt_spp_register_profile(opt);
}

artik_error artik_bluetooth_spp_unregister_profile(void)
{
	return os_bt_spp_unregister_profile();
}

artik_error artik_bluetooth_spp_set_callback(release_callback release_func,
		new_connection_callback connect_func,
		request_disconnect_callback disconnect_func,
		void *user_data) {
	return os_bt_spp_set_callback(release_func, connect_func, disconnect_func, user_data);
}

artik_error artik_bluetooth_ftp_create_session(char *dest_addr)
{
	return os_bt_ftp_create_session(dest_addr);
}

artik_error artik_bluetooth_ftp_remove_session(void)
{
	return os_bt_ftp_remove_session();
}

artik_error artik_bluetooth_ftp_change_folder(char *folder)
{
	return os_bt_ftp_change_folder(folder);
}

artik_error artik_bluetooth_ftp_create_folder(char *folder)
{
	return os_bt_ftp_create_folder(folder);
}

artik_error artik_bluetooth_ftp_delete_file(char *file)
{
	return os_bt_ftp_delete_file(file);
}

artik_error artik_bluetooth_ftp_list_folder(artik_bt_ftp_file **file_list)
{
	return os_bt_ftp_list_folder(file_list);
}

artik_error artik_bluetooth_ftp_get_file(char *target_file, char *source_file)
{
	return os_bt_ftp_get_file(target_file, source_file);
}
artik_error artik_bluetooth_ftp_put_file(char *source_file, char *target_file)
{
	return os_bt_ftp_put_file(source_file, target_file);
}

artik_error artik_bluetooth_ftp_resume_transfer(void)
{
	return os_bt_ftp_resume_transfer();
}

artik_error artik_bluetooth_ftp_suspend_transfer(void)
{
	return os_bt_ftp_suspend_transfer();
}

artik_error artik_bluetooth_agent_register_capability(
	artik_bt_agent_capability e)
{
	return os_bt_agent_register_capability(e);
}

artik_error artik_bluetooth_agent_set_default(void)
{
	return os_bt_agent_set_default();
}

artik_error artik_bluetooth_agent_unregister(void)
{
	return os_bt_agent_unregister();
}

artik_error artik_bluetooth_agent_set_callback(
	artik_bt_agent_callbacks * agent_callback)
{
	return os_bt_agent_set_callback(agent_callback);
}

artik_error artik_bluetooth_agent_send_pincode(artik_bt_agent_request_handle handle, char *pincode)
{
	if (!handle || !pincode)
		return E_BAD_ARGS;

	return os_bt_agent_send_pincode(handle, pincode);
}

artik_error artik_bluetooth_agent_send_passkey(artik_bt_agent_request_handle handle, unsigned int passkey)
{
	if (!handle)
		return E_BAD_ARGS;

	return os_bt_agent_send_passkey(handle, passkey);
}

artik_error artik_bluetooth_agent_send_error(artik_bt_agent_request_handle handle,
		artik_bt_agent_request_error e, const char *err_msg)
{
	if (!handle || !err_msg)
		return E_BAD_ARGS;

	return os_bt_agent_send_error(handle, e, err_msg);
}

artik_error artik_bluetooth_agent_send_empty_response(artik_bt_agent_request_handle handle)
{
	if (!handle)
		return E_BAD_ARGS;

	return os_bt_agent_send_empty_response(handle);
}
