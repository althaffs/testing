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

#include "../os_zigbee.h"
#include <zigbee/zigbee.h>
#include "artik_log.h"

static artik_error _convert_error(int err)
{
	artik_error result;

	switch (err) {
	case EZ_OK:
		result = S_OK;
		break;
	case EZ_BAD_ARGS:
		result = E_BAD_ARGS;
		break;
	case EZ_NOT_INITIALIZED:
		result = E_NOT_INITIALIZED;
		break;
	case EZ_NO_MEM:
		result = E_NO_MEM;
		break;
	case EZ_NOT_SUPPORTED:
		result = E_NOT_SUPPORTED;
		break;
	case EZ_INVALID_VALUE:
		result = E_INVALID_VALUE;
		break;
	case EZ_INVALID_DAEMON:
		result = E_ZIGBEE_INVALID_DAEMON;
		break;
	case EZ_NO_DAEMON:
		result = E_ZIGBEE_NO_DAEMON;
		break;
	case EZ_NO_MESSAGE:
		result = E_ZIGBEE_NO_MESSAGE;
		break;
	case EZ_NO_DEVICE:
		result = E_ZIGBEE_NO_DEVICE;
		break;
	case EZ_ERR_SOCK:
		result = E_ZIGBEE_ERR_SOCK;
		break;
	case EZ_MSG_SEND_ERROR:
		result = E_ZIGBEE_MSG_SEND_ERROR;
		break;
	case EZ_NETWORK_EXIST:
		result = E_ZIGBEE_NETWORK_EXIST;
		break;

	default:
		result = E_ZIGBEE_ERROR;
		break;
	}

	return result;
}

artik_error os_set_local_endpoint(artik_zigbee_local_endpoint_info
								*endpoint_info)
{
	int ret = EZ_OK;

	if (endpoint_info == NULL)
		return E_BAD_ARGS;

	ret = zigbee_set_local_endpoint((zigbee_local_endpoint_info *)
								endpoint_info);

	return _convert_error(ret);
}

artik_error os_get_local_endpoint(artik_zigbee_local_endpoint_info
								*endpoint_info)
{
	int ret = EZ_OK;

	if (endpoint_info == NULL)
		return E_BAD_ARGS;

	ret = zigbee_get_local_endpoint((zigbee_local_endpoint_info *)
								endpoint_info);

	return _convert_error(ret);
}

void os_deinitialize(void)
{
	zigbee_deinitialize();
}

artik_error os_zigbee_initialize(artik_zigbee_client_callback callback,
							 void *user_data)
{
	int ret;

	ret = zigbee_initialize((zigbee_client_callback)callback, user_data);

	return _convert_error(ret);
}

artik_error os_zigbee_network_start(artik_zigbee_network_state *state)
{
	int ret;
	zigbee_network_state nw_state;

	ret = zigbee_network_start(&nw_state);
	if (ret == S_OK && nw_state == ZIGBEE_JOINED_NETWORK)
		zigbee_device_discover();
	if (state != NULL)
		*state = (artik_zigbee_network_state)nw_state;

	return ret;
}

artik_error os_zigbee_network_form_manually(const artik_zigbee_network_info
								*network_info)
{
	int ret;

	ret = zigbee_network_form_manually((zigbee_network_info *)network_info);

	return _convert_error(ret);
}

artik_error os_zigbee_network_form(void)
{
	return _convert_error(zigbee_network_form());
}

artik_error os_zigbee_network_permitjoin(int duration_sec)
{
	int ret;

	ret = zigbee_network_permitjoin(duration_sec);

	return _convert_error(ret);
}

artik_error os_zigbee_network_leave(void)
{
	int ret;

	ret = zigbee_network_leave();

	return _convert_error(ret);
}

artik_error os_zigbee_network_join(void)
{
	return _convert_error(zigbee_network_join());
}

artik_error os_zigbee_network_stop_scan(void)
{
	return _convert_error(zigbee_network_stop_scan());
}

artik_error os_zigbee_network_join_manually(const artik_zigbee_network_info
								*network_info)
{
	int ret;

	ret = zigbee_network_join_manually((zigbee_network_info *)network_info);

	return _convert_error(ret);
}

artik_error os_zigbee_network_find(void)
{
	return _convert_error(zigbee_network_find());
}

artik_error os_zigbee_network_request_my_network_status(
					artik_zigbee_network_state *state)
{
	int ret;

	if (state == NULL)
		return E_BAD_ARGS;

	ret = zigbee_network_request_my_network_status((zigbee_network_state *)
									state);

	return _convert_error(ret);
}

artik_error os_zigbee_device_discover(void)
{
	return _convert_error(zigbee_device_discover());
}

artik_error os_zigbee_set_discover_cycle_time(unsigned int cycle_duration)
{
	return _convert_error(zigbee_set_discover_cycle_time(cycle_duration));
}

artik_error os_zigbee_get_discovered_device_list(artik_zigbee_device_info
								*device_info)
{
	int ret;

	if (device_info == NULL)
		return E_BAD_ARGS;

	ret = zigbee_get_discovered_device_list((zigbee_device_info *)
								device_info);

	return _convert_error(ret);
}

artik_error os_zigbee_device_request_my_node_type(artik_zigbee_node_type *type)
{
	int ret;

	if (type == NULL)
		return E_BAD_ARGS;

	ret = zigbee_device_request_my_node_type((zigbee_node_type *)type);

	return _convert_error(ret);
}

void os_zigbee_device_find_by_cluster(artik_zigbee_endpoint_list *endpoints,
			int cluster_id, int is_server)
{
	zigbee_device_find_by_cluster((zigbee_endpoint_list *)endpoints,
							cluster_id, is_server);
}

void os_zigbee_raw_request(const char *command)
{
	zigbee_raw_request(command);
}

artik_error os_identify_request(artik_zigbee_endpoint_handle handle,
					const artik_zigbee_endpoint *endpoint,
					int duration)
{
	struct inner_endpoint_info *info = get_device_info_by_handle((
						zigbee_endpoint_handle)handle);
	int ret;

	if (info == NULL) {
		log_err("wrong handle %d", handle);
		return E_ZIGBEE_ERROR;
	}

	if (zigbee_check_cluster(info, ZCL_IDENTIFY_CLUSTER_ID, 0) != EZ_OK) {
		log_err("not supported operation");
		return E_ZIGBEE_ERROR;
	}

	ret = zigbee_identify_request((zigbee_endpoint *)endpoint, duration,
							info->endpoint_id);

	return _convert_error(ret);
}

artik_error os_identify_get_remaining_time(artik_zigbee_endpoint_handle handle,
					const artik_zigbee_endpoint *endpoint,
					int *time)
{
	struct inner_endpoint_info *info;
	int ret;

	if (time == NULL)
		return E_BAD_ARGS;

	info = get_device_info_by_handle(handle);
	if (info == NULL) {
		log_err("wrong handle %d", handle);
		return E_ZIGBEE_ERROR;
	}

	if (zigbee_check_cluster(info, ZCL_IDENTIFY_CLUSTER_ID, 0) != EZ_OK) {
		log_err("not supported operation");
		return E_ZIGBEE_ERROR;
	}

	ret = zigbee_identify_get_remaining_time((zigbee_endpoint *)endpoint,
						time, info->endpoint_id);

	return _convert_error(ret);
}

artik_error os_onoff_command(artik_zigbee_endpoint_handle handle,
				const artik_zigbee_endpoint *endpoint,
				artik_zigbee_onoff_status target_status)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret;

	if (info == NULL) {
		log_err("wrong handle %d", handle);
		return E_ZIGBEE_ERROR;
	}

	if (zigbee_check_cluster(info, ZCL_ON_OFF_CLUSTER_ID, 0) != EZ_OK) {
		log_err("not supported operation");
		return E_ZIGBEE_ERROR;
	}

	ret = zigbee_onoff_command((zigbee_endpoint *)endpoint, target_status,
							info->endpoint_id);

	return _convert_error(ret);
}

artik_error os_groups_get_local_name_support(
				artik_zigbee_endpoint_handle handle,
				int endpoint_id)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret;

	if (info == NULL) {
		log_err("wrong handle %d", handle);
		return E_ZIGBEE_ERROR;
	}

	if (zigbee_check_cluster(info, ZCL_GROUPS_CLUSTER_ID, 1) != EZ_OK) {
		log_err("not supported operation");
		return E_ZIGBEE_ERROR;
	}

	ret = zigbee_groups_get_local_name_support(endpoint_id);

	return _convert_error(ret);
}

artik_error os_groups_set_local_name_support(
				artik_zigbee_endpoint_handle handle,
				bool support)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret;

	if (info == NULL) {
		log_err("wrong handle %d", handle);
		return E_ZIGBEE_ERROR;
	}

	if (zigbee_check_cluster(info, ZCL_GROUPS_CLUSTER_ID, 1) != EZ_OK) {
		log_err("not supported operation");
		return E_ZIGBEE_ERROR;
	}

	ret = zigbee_groups_set_local_name_support(info->endpoint_id, support);

	return _convert_error(ret);
}

artik_error os_onoff_get_value(artik_zigbee_endpoint_handle handle,
				artik_zigbee_onoff_status *status)
{
	struct inner_endpoint_info *info;
	int ret;

	if (status == NULL)
		return E_BAD_ARGS;

	info = get_device_info_by_handle(handle);
	if (info == NULL) {
		log_err("wrong handle %d", handle);
		return E_ZIGBEE_ERROR;
	}

	if (zigbee_check_cluster(info, ZCL_ON_OFF_CLUSTER_ID, 1) != EZ_OK) {
		log_err("not supported operation");
		return E_ZIGBEE_ERROR;
	}

	ret = zigbee_onoff_get_value(info->endpoint_id, (zigbee_onoff_status *)
									status);

	return _convert_error(ret);
}

artik_error os_level_control_request(artik_zigbee_endpoint_handle handle,
			const artik_zigbee_endpoint *endpoint,
			const artik_zigbee_level_control_command *command)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret;

	if (info == NULL) {
		log_err("wrong handle %d", handle);
		return E_ZIGBEE_ERROR;
	}

	if (zigbee_check_cluster(info, ZCL_LEVEL_CONTROL_CLUSTER_ID, 0) !=
								EZ_OK) {
		log_err("not supported operation");
		return E_ZIGBEE_ERROR;
	}

	ret = zigbee_level_control_request((zigbee_endpoint *)endpoint,
			(zigbee_level_control_command *)command,
			info->endpoint_id);

	return _convert_error(ret);
}

artik_error os_level_control_get_value(artik_zigbee_endpoint_handle handle,
			int *value)
{
	struct inner_endpoint_info *info;
	int ret;

	if (value == NULL)
		return E_BAD_ARGS;

	info = get_device_info_by_handle(handle);
	if (info == NULL) {
		log_err("wrong handle %d", handle);
		return E_ZIGBEE_ERROR;
	}

	if (zigbee_check_cluster(info, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1) !=
								EZ_OK) {
		log_err("not supported operation");
		return E_ZIGBEE_ERROR;
	}

	ret = zigbee_level_control_get_value(info->endpoint_id, value);

	return _convert_error(ret);
}

artik_error os_color_control_request(artik_zigbee_endpoint_handle handle,
			const artik_zigbee_endpoint *endpoint,
			const artik_zigbee_color_control_command *command)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret;

	if (info == NULL) {
		log_err("wrong handle %d", handle);
		return E_ZIGBEE_ERROR;
	}

	if (zigbee_check_cluster(info, ZCL_LEVEL_CONTROL_CLUSTER_ID, 0) !=
								EZ_OK) {
		log_err("not supported operation");
		return E_ZIGBEE_ERROR;
	}


	ret = zigbee_color_control_request((zigbee_endpoint *)endpoint,
			(zigbee_color_control_command *)command,
			info->endpoint_id);

	return _convert_error(ret);
}

artik_error os_color_control_get_value(artik_zigbee_endpoint_handle handle,
			artik_zigbee_color_control_value *value)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret;

	if (info == NULL) {
		log_err("wrong handle %d", handle);
		return E_ZIGBEE_ERROR;
	}

	if (zigbee_check_cluster(info, ZCL_COLOR_CONTROL_CLUSTER_ID, 1) !=
								EZ_OK) {
		log_err("not supported operation");
		return E_ZIGBEE_ERROR;
	}

	ret = zigbee_color_control_get_value(info->endpoint_id,
			(zigbee_color_control_value *)value);

	return _convert_error(ret);
}

artik_error os_illum_set_measured_value_range(
			artik_zigbee_endpoint_handle handle,
			int min, int max)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret;

	if (info == NULL) {
		log_err("wrong handle %d", handle);
		return E_ZIGBEE_ERROR;
	}

	if (zigbee_check_cluster(info, ZCL_ILLUM_MEASUREMENT_CLUSTER_ID, 1) !=
		EZ_OK) {
		log_err("not supported operation");
		return E_ZIGBEE_ERROR;
	}

	ret = zigbee_illum_set_measured_value_range(info->endpoint_id, min,
									max);

	return _convert_error(ret);
}

artik_error os_illum_set_measured_value(artik_zigbee_endpoint_handle handle,
			int value)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret;

	if (info == NULL) {
		log_err("wrong handle %d", handle);
		return E_ZIGBEE_ERROR;
	}

	if (zigbee_check_cluster(info, ZCL_ILLUM_MEASUREMENT_CLUSTER_ID, 1) !=
		EZ_OK) {
		log_err("not supported operation");
		return E_ZIGBEE_ERROR;
	}

	ret = zigbee_illum_set_measured_value(info->endpoint_id, value);

	return _convert_error(ret);
}

artik_error os_illum_get_measured_value(artik_zigbee_endpoint_handle handle,
			int *value)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret;

	if (info == NULL) {
		log_err("wrong handle %d", handle);
		return E_ZIGBEE_ERROR;
	}

	if (zigbee_check_cluster(info, ZCL_ILLUM_MEASUREMENT_CLUSTER_ID, 1) !=
		EZ_OK) {
		log_err("not supported operation");
		return E_ZIGBEE_ERROR;
	}

	ret = zigbee_illum_get_measured_value(info->endpoint_id, value);

	return _convert_error(ret);
}

artik_error os_occupancy_set_type(artik_zigbee_endpoint_handle handle,
			artik_zigbee_occupancy_type type)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret;

	if (info == NULL) {
		log_err("wrong handle %d", handle);
		return E_ZIGBEE_ERROR;
	}

	if (zigbee_check_cluster(info, ZCL_OCCUPANCY_SENSING_CLUSTER_ID, 1) !=
		EZ_OK) {
		log_err("not supported operation");
		return E_ZIGBEE_ERROR;
	}

	ret = zigbee_occupancy_set_type(info->endpoint_id, type);

	return _convert_error(ret);
}

artik_error os_occupancy_set_value(artik_zigbee_endpoint_handle handle,
			artik_zigbee_occupancy_status status)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret;

	if (info == NULL) {
		log_err("wrong handle %d", handle);
		return E_ZIGBEE_ERROR;
	}

	if (zigbee_check_cluster(info, ZCL_OCCUPANCY_SENSING_CLUSTER_ID, 1) !=
		EZ_OK) {
		log_err("not supported operation");
		return E_ZIGBEE_ERROR;
	}

	ret = zigbee_occupancy_set_value(info->endpoint_id, status);

	return _convert_error(ret);
}

artik_error os_thermostat_get_occupied_cooling_setpoint(
			artik_zigbee_endpoint_handle handle,
			int *setpoint)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret;

	if (info == NULL) {
		log_err("wrong handle %d", handle);
		return E_ZIGBEE_ERROR;
	}

	if (zigbee_check_cluster(info, ZCL_THERMOSTAT_CLUSTER_ID, 1) != S_OK) {
		log_err("not supported operation");
		return E_ZIGBEE_ERROR;
	}

	ret = zigbee_thermostat_get_occupied_cooling_setpoint(info->endpoint_id,
								setpoint);

	return _convert_error(ret);
}

artik_error os_thermostat_get_occupied_heating_setpoint(
			artik_zigbee_endpoint_handle handle,
			int *setpoint)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret;

	if (info == NULL) {
		log_err("wrong handle %d", handle);
		return E_ZIGBEE_ERROR;
	}

	if (zigbee_check_cluster(info, ZCL_THERMOSTAT_CLUSTER_ID, 1) != S_OK) {
		log_err("not supported operation");
		return E_ZIGBEE_ERROR;
	}

	ret = zigbee_thermostat_get_occupied_heating_setpoint(info->endpoint_id,
								setpoint);

	return _convert_error(ret);
}

artik_error os_fan_get_mode(artik_zigbee_endpoint_handle handle,
			artik_zigbee_fan_mode *mode)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret;

	if (info == NULL) {
		log_err("wrong handle %d", handle);
		return E_ZIGBEE_ERROR;
	}

	if (zigbee_check_cluster(info, ZCL_FAN_CONTROL_CLUSTER_ID, 1) != S_OK) {
		log_err("not supported operation");
		return E_ZIGBEE_ERROR;
	}

	ret = zigbee_fan_get_mode(info->endpoint_id, (zigbee_fan_mode *)mode);

	return _convert_error(ret);
}

artik_error os_fan_get_mode_sequence(artik_zigbee_endpoint_handle handle,
			artik_zigbee_fan_mode_sequence *seq)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret;

	if (info == NULL) {
		log_err("wrong handle %d", handle);
		return E_ZIGBEE_ERROR;
	}

	if (zigbee_check_cluster(info, ZCL_FAN_CONTROL_CLUSTER_ID, 1) != S_OK) {
		log_err("not supported operation");
		return E_ZIGBEE_ERROR;
	}

	ret = zigbee_fan_get_mode_sequence(info->endpoint_id,
					(zigbee_fan_mode_sequence *)seq);

	return _convert_error(ret);
}

/*ezmode commissioning*/
artik_error os_ezmode_commissioning(artik_zigbee_endpoint_handle handle,
				bool initiator, bool start)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);

	if (info == NULL) {
		log_err("wrong handle %d", handle);
		return E_ZIGBEE_ERROR;
	}

	return _convert_error(zigbee_ezmode_commissioning(info->endpoint_id,
			initiator, start));
}

artik_error os_factory_reset(void)
{
	int ret;

	ret = zigbee_factory_reset();
	return _convert_error(ret);
}

artik_error os_reset_local(void)
{
	artik_zigbee_network_state state;
	int ret;
	GList *list = NULL;
	struct inner_endpoint_info *device = NULL;

	ret = zigbee_network_stop_scan();
	if (ret != S_OK) {
		log_err("Get current network state error(%d)", ret);
		return _convert_error(ret);
	}

	list = zigbee_get_handle_list();
	while (list != NULL) {
		device = (struct inner_endpoint_info *)list->data;
		ret = zigbee_reset_local_attr(device->endpoint_id);
		if (ret != S_OK) {
			log_err("Reset local attribute error(%d)", ret);
			return _convert_error(ret);
		}
		list = list->next;
	}

	ret = os_zigbee_network_request_my_network_status(&state);
	if (ret != S_OK) {
		log_err("Get current network state error(%d)", ret);
		return _convert_error(ret);
	}

	if (state == ARTIK_ZIGBEE_JOINED_NETWORK
			|| state ==
			ARTIK_ZIGBEE_JOINED_NETWORK_NO_PARENT) {
		ret = os_zigbee_network_leave();
		if (ret != S_OK) {
			log_err("Network leave operation error(%d)",
				ret);
			return _convert_error(ret);
		}
	}

	/*clear binding table*/
	ret = os_factory_reset();
	if (ret != S_OK)
		log_err("Factory reset operation(clear binding table) error(%d)"
									, ret);

	return _convert_error(ret);
}

artik_error os_network_steering(void)
{
	return _convert_error(zigbee_network_steering());
}

artik_error os_find_bind(artik_zigbee_endpoint_handle handle, bool initiator,
						 bool start)
{
	int ret;
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);

	if (info == NULL) {
		log_err("wrong handle %d", handle);
		return E_ZIGBEE_ERROR;
	}

	ret = zigbee_find_bind(info->endpoint_id, initiator, start);
	return _convert_error(ret);
}

artik_error os_zigbee_broadcast_permitjoin(int duration)
{
	int ret;

	ret = zigbee_broadcast_permitjoin(duration);
	return _convert_error(ret);
}

artik_error os_zigbee_broadcast_identify_query(
					artik_zigbee_endpoint_handle handle)
{
	int ret;
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);

	if (info == NULL) {
		log_err("wrong handle %d", handle);
		return E_ZIGBEE_ERROR;
	}

	ret = zigbee_broadcast_identify_query(info->endpoint_id);
	return _convert_error(ret);
}

artik_error os_zigbee_ieee_addr_request(int node_id)
{
	int ret;

	ret = zigbee_ieee_addr_request(node_id);
	return _convert_error(ret);
}

artik_error os_zigbee_simple_descriptor_request(int node_id,
					int target_endpoint)
{
	int ret;

	ret = zigbee_simple_descriptor_request(node_id, target_endpoint);
	return _convert_error(ret);
}

artik_error os_zigbee_broadcast_match_descriptor_request(
				ZIGBEE_PROFILE profile_id,
				int cluster_id,
				bool server_cluster)
{
	int ret;

	ret = zigbee_broadcast_match_descriptor_request(profile_id, cluster_id,
							server_cluster);
	return _convert_error(ret);
}

artik_error os_zigbee_get_binding(int index,
				artik_zigbee_binding_table_entry *value)
{
	int ret;

	ret = zigbee_get_binding(index, (zigbee_binding_table_entry *)value);
	return _convert_error(ret);
}

artik_error os_zigbee_set_binding(int index,
				artik_zigbee_binding_table_entry *value)
{
	int ret;

	ret = zigbee_set_binding(index, (zigbee_binding_table_entry *)value);
	return _convert_error(ret);
}

artik_error os_zigbee_delete_binding(int index)
{
	int ret;

	ret = zigbee_delete_binding(index);
	return _convert_error(ret);
}

artik_error os_zigbee_clear_binding(void)
{
	int ret;

	ret = zigbee_clear_binding();
	return _convert_error(ret);
}

artik_error os_zigbee_end_device_bind_request(
					artik_zigbee_endpoint_handle handle)
{
	int ret;
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);

	if (info == NULL) {
		log_err("wrong handle %d", handle);
		return E_ZIGBEE_ERROR;
	}

	ret = zigbee_end_device_bind_request(info->endpoint_id);
	return _convert_error(ret);
}

int os_fan_write_mode(artik_zigbee_endpoint_handle handle,
					  const artik_zigbee_endpoint *endpoint,
					  artik_zigbee_fan_mode mode)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret = E_ZIGBEE_ERROR;

	if (info == NULL)
		return E_ZIGBEE_ERROR;

	if (zigbee_check_cluster(info, ZCL_FAN_CONTROL_CLUSTER_ID, 0) != S_OK)
		return E_INVALID_VALUE;

	ret = zigbee_fan_write_mode((zigbee_endpoint *)endpoint,
			(zigbee_fan_mode)mode, info->endpoint_id);

	return _convert_error(ret);
}

int os_fan_write_mode_sequence(artik_zigbee_endpoint_handle handle,
					const artik_zigbee_endpoint *endpoint,
					artik_zigbee_fan_mode_sequence seq)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret = E_ZIGBEE_ERROR;

	if (info == NULL)
		return E_ZIGBEE_ERROR;

	if (zigbee_check_cluster(info, ZCL_FAN_CONTROL_CLUSTER_ID, 0) != S_OK)
		return E_INVALID_VALUE;

	ret = zigbee_fan_write_mode_sequence((zigbee_endpoint *)endpoint,
			(zigbee_fan_mode_sequence)seq, info->endpoint_id);

	return _convert_error(ret);
}

int os_fan_read_mode(artik_zigbee_endpoint_handle handle,
					 const artik_zigbee_endpoint *endpoint,
					 artik_zigbee_fan_mode *fanmode)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret = E_ZIGBEE_ERROR;

	if (fanmode == NULL)
		return E_BAD_ARGS;

	if (info == NULL)
		return E_ZIGBEE_ERROR;

	if (zigbee_check_cluster(info, ZCL_FAN_CONTROL_CLUSTER_ID, 0) != S_OK)
		return E_INVALID_VALUE;

	ret = zigbee_fan_read_mode((zigbee_endpoint *)endpoint,
			(zigbee_fan_mode *)fanmode, info->endpoint_id);

	return _convert_error(ret);
}

int os_fan_read_mode_sequence(artik_zigbee_endpoint_handle handle,
			const artik_zigbee_endpoint *endpoint,
			artik_zigbee_fan_mode_sequence *fanmode_sequence)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret = E_ZIGBEE_ERROR;

	if (fanmode_sequence == NULL)
		return E_BAD_ARGS;

	if (info == NULL)
		return E_ZIGBEE_ERROR;

	if (zigbee_check_cluster(info, ZCL_FAN_CONTROL_CLUSTER_ID, 0) != S_OK)
		return E_INVALID_VALUE;

	ret = zigbee_fan_read_mode_sequence((zigbee_endpoint *)endpoint,
			(zigbee_fan_mode_sequence *)fanmode_sequence,
							info->endpoint_id);

	return _convert_error(ret);
}

artik_error os_thermostat_set_local_temperature(
				artik_zigbee_endpoint_handle handle,
				int temperature)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret = E_ZIGBEE_ERROR;

	if (info == NULL)
		return E_ZIGBEE_ERROR;

	if (zigbee_check_cluster(info, ZCL_THERMOSTAT_CLUSTER_ID, 1) != EZ_OK)
		return E_INVALID_VALUE;

	ret = zigbee_thermostat_set_local_temperature(info->endpoint_id,
								temperature);

	return _convert_error(ret);
}

artik_error os_thermostat_get_local_temperature(
				artik_zigbee_endpoint_handle handle,
				int *temperature)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret = E_ZIGBEE_ERROR;

	if (temperature == NULL)
		return E_BAD_ARGS;

	if (info == NULL)
		return E_ZIGBEE_ERROR;

	if (zigbee_check_cluster(info, ZCL_THERMOSTAT_CLUSTER_ID, 1) != EZ_OK)
		return E_INVALID_VALUE;

	ret = zigbee_thermostat_get_local_temperature(info->endpoint_id,
								temperature);

	return _convert_error(ret);
}

artik_error os_thermostat_set_occupancy_status(
				artik_zigbee_endpoint_handle handle,
				artik_zigbee_occupancy_status status)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret = E_ZIGBEE_ERROR;

	if (info == NULL) {
		log_err("wrong handle %d", handle);
		return E_ZIGBEE_ERROR;
	}

	if (zigbee_check_cluster(info, ZCL_THERMOSTAT_CLUSTER_ID, 1) != EZ_OK) {
		log_err("not supported operation");
		return E_INVALID_VALUE;
	}

	ret = zigbee_thermostat_set_occupancy_status(info->endpoint_id, status);

	return _convert_error(ret);
}

artik_error os_thermostat_get_occupancy_status(
				artik_zigbee_endpoint_handle handle,
				artik_zigbee_occupancy_status *status)
{
	int ret = E_ZIGBEE_ERROR;
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);

	if (status == NULL)
		return E_BAD_ARGS;

	if (info == NULL)
		return E_ZIGBEE_ERROR;

	if (zigbee_check_cluster(info, ZCL_THERMOSTAT_CLUSTER_ID, 1) != EZ_OK)
		return E_INVALID_VALUE;

	ret = zigbee_thermostat_get_occupancy_status(info->endpoint_id,
			(zigbee_occupancy_status *)status);

	return _convert_error(ret);
}

artik_error os_thermostat_set_occupied_cooling_setpoint(
				artik_zigbee_endpoint_handle handle,
				int temperature)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret = E_ZIGBEE_ERROR;

	if (info == NULL)
		return E_ZIGBEE_ERROR;

	if (zigbee_check_cluster(info, ZCL_THERMOSTAT_CLUSTER_ID, 1) != EZ_OK)
		return E_INVALID_VALUE;

	ret = zigbee_thermostat_set_occupied_cooling_setpoint(info->endpoint_id,
								temperature);

	return _convert_error(ret);
}

artik_error os_thermostat_set_occupied_heating_setpoint(
				artik_zigbee_endpoint_handle handle,
				int temperature)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret = E_ZIGBEE_ERROR;

	if (info == NULL)
		return E_ZIGBEE_ERROR;

	if (zigbee_check_cluster(info, ZCL_THERMOSTAT_CLUSTER_ID, 1) != EZ_OK)
		return E_INVALID_VALUE;

	ret = zigbee_thermostat_set_occupied_heating_setpoint(info->endpoint_id,
								temperature);

	return _convert_error(ret);
}

artik_error os_thermostat_set_system_mode(artik_zigbee_endpoint_handle handle,
				artik_zigbee_thermostat_system_mode mode)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret = E_ZIGBEE_ERROR;

	if (info == NULL)
		return E_ZIGBEE_ERROR;

	if (zigbee_check_cluster(info, ZCL_THERMOSTAT_CLUSTER_ID, 1) != EZ_OK)
		return E_INVALID_VALUE;

	ret = zigbee_thermostat_set_system_mode(info->endpoint_id, mode);

	return _convert_error(ret);
}

artik_error os_thermostat_get_system_mode(artik_zigbee_endpoint_handle handle,
				artik_zigbee_thermostat_system_mode *mode)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret = E_ZIGBEE_ERROR;

	if (mode == NULL)
		return E_BAD_ARGS;

	if (info == NULL)
		return E_ZIGBEE_ERROR;

	if (zigbee_check_cluster(info, ZCL_THERMOSTAT_CLUSTER_ID, 1) != EZ_OK)
		return E_INVALID_VALUE;

	ret = zigbee_thermostat_get_system_mode(info->endpoint_id,
			(zigbee_thermostat_system_mode *)mode);

	return _convert_error(ret);
}

artik_error os_thermostat_set_control_sequence(
				artik_zigbee_endpoint_handle handle,
				artik_zigbee_thermostat_control_sequence seq)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret = E_ZIGBEE_ERROR;

	if (info == NULL)
		return E_ZIGBEE_ERROR;

	if (zigbee_check_cluster(info, ZCL_THERMOSTAT_CLUSTER_ID, 1) != EZ_OK)
		return E_INVALID_VALUE;

	ret = zigbee_thermostat_set_control_sequence(info->endpoint_id,
			(zigbee_thermostat_control_sequence)seq);

	return _convert_error(ret);
}

artik_error os_thermostat_get_control_sequence(
				artik_zigbee_endpoint_handle handle,
				artik_zigbee_thermostat_control_sequence *seq)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret = E_ZIGBEE_ERROR;

	if (seq == NULL)
		return E_BAD_ARGS;

	if (info == NULL)
		return E_ZIGBEE_ERROR;

	if (zigbee_check_cluster(info, ZCL_THERMOSTAT_CLUSTER_ID, 1) != EZ_OK)
		return E_INVALID_VALUE;

	ret = zigbee_thermostat_get_control_sequence(info->endpoint_id,
			(zigbee_thermostat_control_sequence *)seq);

	return _convert_error(ret);
}

int os_temperature_set_measured_value(artik_zigbee_endpoint_handle handle,
				int value)
{
	struct inner_endpoint_info *info;
	int ret;

	info = get_device_info_by_handle(handle);
	if (info == NULL) {
		log_err("wrong handle %d", handle);
		return E_ZIGBEE_ERROR;
	}

	if (zigbee_check_cluster(info, ZCL_TEMP_MEASUREMENT_CLUSTER_ID, 1) !=
		EZ_OK) {
		log_err("not supported operation");
		return E_ZIGBEE_ERROR;
	}

	ret = zigbee_temperature_set_measured_value(info->endpoint_id, value);

	return _convert_error(ret);
}

int os_temperature_set_measured_value_range(artik_zigbee_endpoint_handle handle,
				int min,
				int max)
{
	struct inner_endpoint_info *info;
	int ret;

	info = get_device_info_by_handle(handle);
	if (info == NULL) {
		log_err("wrong handle %d", handle);
		return E_ZIGBEE_ERROR;
	}

	if (zigbee_check_cluster(info, ZCL_TEMP_MEASUREMENT_CLUSTER_ID, 1) !=
		EZ_OK) {
		log_err("not supported operation");
		return E_ZIGBEE_ERROR;
	}

	ret = zigbee_temperature_set_measured_value_range(info->endpoint_id,
								min, max);

	return _convert_error(ret);
}

artik_error os_reset_to_factory_default(artik_zigbee_endpoint_handle handle,
				const artik_zigbee_endpoint *endpoint)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret = E_ZIGBEE_ERROR;

	if (info == NULL)
		return E_ZIGBEE_ERROR;

	if (zigbee_check_cluster(info, ZCL_BASIC_CLUSTER_ID, 0) != EZ_OK)
		return E_INVALID_VALUE;

	ret = zigbee_reset_to_factory_default((zigbee_endpoint *)endpoint,
		info->endpoint_id);

	return _convert_error(ret);
}

artik_error os_thermostat_request_setpoint_raise_lower(
				artik_zigbee_endpoint_handle handle,
				const artik_zigbee_endpoint *endpoint,
				artik_zigbee_thermostat_setpoint_mode mode,
				int temperature)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret = E_ZIGBEE_ERROR;

	if (info == NULL)
		return E_ZIGBEE_ERROR;

	if (zigbee_check_cluster(info, ZCL_THERMOSTAT_CLUSTER_ID, 0) != EZ_OK)
		return E_INVALID_VALUE;

	ret = zigbee_thermostat_request_setpoint_raise_lower((zigbee_endpoint *)
					endpoint,
					(zigbee_thermostat_setpoint_mode)mode,
					temperature, info->endpoint_id);

	return _convert_error(ret);
}

artik_error os_thermostat_read_occupied_cooling_setpoint(
				artik_zigbee_endpoint_handle handle,
				artik_zigbee_endpoint *endpoint,
				int *temperature)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret = E_ZIGBEE_ERROR;

	if (temperature == NULL)
		return E_BAD_ARGS;

	if (info == NULL)
		return E_ZIGBEE_ERROR;

	if (zigbee_check_cluster(info, ZCL_THERMOSTAT_CLUSTER_ID, 0) != EZ_OK)
		return E_INVALID_VALUE;

	ret = zigbee_thermostat_read_occupied_cooling_setpoint(
						(zigbee_endpoint *)endpoint,
						temperature, info->endpoint_id);

	return _convert_error(ret);
}

artik_error os_thermostat_read_occupied_heating_setpoint(
				artik_zigbee_endpoint_handle handle,
				artik_zigbee_endpoint *endpoint,
				int *temperature)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret = E_ZIGBEE_ERROR;

	if (temperature == NULL)
		return E_BAD_ARGS;

	if (info == NULL)
		return E_ZIGBEE_ERROR;

	if (zigbee_check_cluster(info, ZCL_THERMOSTAT_CLUSTER_ID, 0) != EZ_OK)
		return E_INVALID_VALUE;

	ret = zigbee_thermostat_read_occupied_heating_setpoint(
				(zigbee_endpoint *)endpoint,
				temperature, info->endpoint_id);

	return _convert_error(ret);
}

artik_error os_thermostat_read_system_mode(artik_zigbee_endpoint_handle handle,
				artik_zigbee_endpoint *endpoint,
				artik_zigbee_thermostat_system_mode *mode)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret = E_ZIGBEE_ERROR;

	if (mode == NULL)
		return E_BAD_ARGS;

	if (info == NULL)
		return E_ZIGBEE_ERROR;

	if (zigbee_check_cluster(info, ZCL_THERMOSTAT_CLUSTER_ID, 0) != EZ_OK)
		return E_INVALID_VALUE;

	ret = zigbee_thermostat_read_system_mode((zigbee_endpoint *)endpoint,
			(zigbee_thermostat_system_mode *)mode,
			info->endpoint_id);

	return _convert_error(ret);
}

artik_error os_thermostat_write_system_mode(artik_zigbee_endpoint_handle handle,
				artik_zigbee_endpoint *endpoint,
				artik_zigbee_thermostat_system_mode mode)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret = E_ZIGBEE_ERROR;

	if (info == NULL)
		return E_ZIGBEE_ERROR;

	if (zigbee_check_cluster(info, ZCL_THERMOSTAT_CLUSTER_ID, 0) != EZ_OK)
		return E_INVALID_VALUE;

	ret = zigbee_thermostat_write_system_mode((zigbee_endpoint *)endpoint,
			(zigbee_thermostat_system_mode)mode, info->endpoint_id);

	return _convert_error(ret);
}

artik_error os_thermostat_read_control_sequence(
				artik_zigbee_endpoint_handle handle,
				artik_zigbee_endpoint *endpoint,
				artik_zigbee_thermostat_control_sequence *seq)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret = E_ZIGBEE_ERROR;

	if (seq == NULL)
		return E_BAD_ARGS;

	if (info == NULL)
		return E_ZIGBEE_ERROR;

	if (zigbee_check_cluster(info, ZCL_THERMOSTAT_CLUSTER_ID, 0) != EZ_OK)
		return E_INVALID_VALUE;

	ret = zigbee_thermostat_read_control_sequence((zigbee_endpoint *)
								endpoint,
			(zigbee_thermostat_control_sequence *)seq,
							info->endpoint_id);

	return _convert_error(ret);
}

artik_error os_thermostat_write_control_sequence(
				artik_zigbee_endpoint_handle handle,
				artik_zigbee_endpoint *endpoint,
				artik_zigbee_thermostat_control_sequence seq)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret = E_ZIGBEE_ERROR;

	if (info == NULL)
		return E_ZIGBEE_ERROR;

	if (zigbee_check_cluster(info, ZCL_THERMOSTAT_CLUSTER_ID, 0) != EZ_OK)
		return E_INVALID_VALUE;

	ret = zigbee_thermostat_write_control_sequence((zigbee_endpoint *)
								endpoint,
			(zigbee_thermostat_control_sequence)seq,
							info->endpoint_id);

	return _convert_error(ret);
}

artik_error os_request_reporting(artik_zigbee_endpoint_handle handle,
				const artik_zigbee_endpoint *endpoint,
				artik_zigbee_reporting_type report_type,
				int min_interval, int max_interval,
				int change_threshold)
{
	struct inner_endpoint_info *info = get_device_info_by_handle(handle);
	int ret = E_ZIGBEE_ERROR;
	int cluster_id = -1;
	zigbee_endpoint *zb_endpoint = (zigbee_endpoint *)endpoint;

	if (info == NULL)
		return E_ZIGBEE_ERROR;

	if (max_interval < min_interval) {
		log_err("Min interval is large than max interval");
		return E_INVALID_VALUE;
	}

	if (zigbee_check_reporting(info->device_id, (zigbee_reporting_type)
						report_type) != EZ_OK) {
		log_err("device and reporting type can not match");
		return E_INVALID_VALUE;
	}

	switch (report_type) {
	case ARTIK_ZIGBEE_REPORTING_THERMOSTAT_TEMPERATURE:
		/* change_threshold = 0 is for stop_reporting*/
		if (change_threshold < 0 || change_threshold >
						TEMPERATURE_MAX_THRESHOLD)
			return E_INVALID_VALUE;
		cluster_id = ARTIK_ZCL_THERMOSTAT_CLUSTER_ID;
		break;

	case ARTIK_ZIGBEE_REPORTING_OCCUPANCY_SENSING:
		cluster_id = ARTIK_ZCL_OCCUPANCY_SENSING_CLUSTER_ID;
		break;

	case ARTIK_ZIGBEE_REPORTING_MEASURED_ILLUMINANCE:
		/* change_threshold = 0 is for stop_reporting*/
		if (change_threshold < 0 || change_threshold >
						ILLUMINANCE_MAX_THRESHOLD)
			return E_INVALID_VALUE;
		cluster_id = ARTIK_ZCL_ILLUM_MEASUREMENT_CLUSTER_ID;
		break;

	case ARTIK_ZIGBEE_REPORTING_MEASURED_TEMPERATURE:
		/* change_threshold = 0 is for stop_reporting*/
		if (change_threshold < 0 || change_threshold >
						TEMPERATURE_MAX_THRESHOLD)
			return E_INVALID_VALUE;
		cluster_id = ARTIK_ZCL_TEMP_MEASUREMENT_CLUSTER_ID;
		break;

	default:
		log_err("report_type(%d) is not supported, please check",
						report_type);
		return E_INVALID_VALUE;
	}

	if (zigbee_check_cluster(info, cluster_id, 0) != EZ_OK) {
		log_err("not supported operation");
		return E_INVALID_VALUE;
	}

	ret = zigbee_request_reporting(cluster_id, zb_endpoint,
			(zigbee_reporting_type)report_type,
			min_interval, max_interval, change_threshold,
							info->endpoint_id);

	return _convert_error(ret);
}

artik_error os_stop_reporting(artik_zigbee_endpoint_handle handle,
				const artik_zigbee_endpoint *endpoint,
				artik_zigbee_reporting_type report_type)
{
	return os_request_reporting(handle, endpoint, report_type, 0, 0xFFFF,
									0);
}
