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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "os_zigbee.h"
#include "artik_zigbee.h"

static artik_error artik_zigbee_set_local_endpoint(
			artik_zigbee_local_endpoint_info * endpoint_info);
static artik_error artik_zigbee_get_local_endpoint(
			artik_zigbee_local_endpoint_info * endpoint_info);
static void artik_zigbee_deinitialize(void);
static artik_error artik_zigbee_initialize(
			artik_zigbee_client_callback callback,
			void *user_data);
static artik_error artik_zigbee_network_start(
			artik_zigbee_network_state * state);
static artik_error artik_zigbee_network_form(void);
static artik_error artik_zigbee_network_form_manually(
			const artik_zigbee_network_info * network_info);
static artik_error artik_zigbee_network_permitjoin(int duration_sec);
static artik_error artik_zigbee_network_leave(void);
static artik_error artik_zigbee_network_join(void);
static artik_error artik_zigbee_network_stop_scan(void);
static artik_error artik_zigbee_network_join_manually(
			const artik_zigbee_network_info * network_info);
static artik_error artik_zigbee_network_find(void);
static artik_error artik_zigbee_network_request_my_network_status(
			artik_zigbee_network_state * state);
static artik_error artik_zigbee_device_discover(void);
static artik_error artik_zigbee_set_discover_cycle_time(
			unsigned int cycle_duration);
static artik_error artik_zigbee_get_discovered_device_list(
			artik_zigbee_device_info * device_info);
static artik_error artik_zigbee_device_request_my_node_type(
			artik_zigbee_node_type * type);
static void artik_zigbee_device_find_by_cluster(
			artik_zigbee_endpoint_list * endpoints,
			int cluster_id,
			int is_server);
static void artik_zigbee_raw_request(const char *command);
static artik_error artik_identify_request(artik_zigbee_endpoint_handle handle,
			const artik_zigbee_endpoint *endpoint,
			int duration);
static artik_error artik_identify_get_remaining_time(
			artik_zigbee_endpoint_handle handle,
			const artik_zigbee_endpoint *endpoint,
			int *time);
static artik_error artik_onoff_command(artik_zigbee_endpoint_handle handle,
			const artik_zigbee_endpoint *endpoint,
			artik_zigbee_onoff_status target_status);
static artik_error artik_groups_get_local_name_support(
			artik_zigbee_endpoint_handle handle,
			int endpoint_id);
static artik_error artik_groups_set_local_name_support(
			artik_zigbee_endpoint_handle handle,
			bool support);
static artik_error artik_onoff_get_value(artik_zigbee_endpoint_handle handle,
			artik_zigbee_onoff_status *status);
static artik_error artik_level_control_request(
			artik_zigbee_endpoint_handle handle,
			const artik_zigbee_endpoint *endpoint,
			const artik_zigbee_level_control_command *command);
static artik_error artik_level_control_get_value(
			artik_zigbee_endpoint_handle handle,
			int *value);
static artik_error artik_illum_set_measured_value_range(
			artik_zigbee_endpoint_handle handle,
			int min, int max);
static artik_error artik_illum_set_measured_value(
			artik_zigbee_endpoint_handle handle,
			int value);
static artik_error artik_illum_get_measured_value(
			artik_zigbee_endpoint_handle handle,
			int *value);
static artik_error artik_ezmode_commissioning_initiator_start(
			artik_zigbee_endpoint_handle handle);
static artik_error artik_ezmode_commissioning_initiator_stop(
			artik_zigbee_endpoint_handle handle);
static artik_error artik_ezmode_commissioning_target_start(
			artik_zigbee_endpoint_handle handle);
static artik_error artik_ezmode_commissioning_target_stop(
			artik_zigbee_endpoint_handle handle);
static artik_error artik_zigbee_reset_local(void);
static artik_error artik_reset_to_factory_default(
			artik_zigbee_endpoint_handle handle,
			const artik_zigbee_endpoint *endpoint);
static artik_error artik_request_reporting(artik_zigbee_endpoint_handle handle,
			const artik_zigbee_endpoint *endpoint,
			artik_zigbee_reporting_type report_type,
			int min_interval, int max_interval,
			int change_threshold);
static artik_error artik_stop_reporting(artik_zigbee_endpoint_handle handle,
			const artik_zigbee_endpoint *endpoint,
			artik_zigbee_reporting_type report_type);

const artik_zigbee_module zigbee_module = {
	artik_zigbee_reset_local,
	artik_zigbee_set_local_endpoint,
	artik_zigbee_get_local_endpoint,
	artik_zigbee_initialize,
	artik_zigbee_deinitialize,
	artik_zigbee_network_start,
	artik_zigbee_network_form,
	artik_zigbee_network_form_manually,
	artik_zigbee_network_permitjoin,
	artik_zigbee_network_leave,
	artik_zigbee_network_join,
	artik_zigbee_network_stop_scan,
	artik_zigbee_network_join_manually,
	artik_zigbee_network_find,
	artik_zigbee_network_request_my_network_status,
	artik_zigbee_device_discover,
	artik_zigbee_set_discover_cycle_time,
	artik_zigbee_get_discovered_device_list,
	artik_zigbee_device_request_my_node_type,
	artik_zigbee_device_find_by_cluster,
	artik_zigbee_raw_request
};

const artik_zigbee_device_on_off_switch device_on_off_switch_func = {
	artik_identify_request,
	artik_identify_get_remaining_time,
	artik_onoff_command,
	artik_ezmode_commissioning_initiator_start,
	artik_ezmode_commissioning_initiator_stop
};

const artik_zigbee_device_level_control_switch
				device_level_control_switch_func = {
	artik_identify_request,
	artik_identify_get_remaining_time,
	artik_onoff_command,
	artik_level_control_request,
	artik_ezmode_commissioning_initiator_start,
	artik_ezmode_commissioning_initiator_stop
};

const artik_zigbee_device_on_off_light device_on_off_light_func = {
	artik_groups_get_local_name_support,
	artik_groups_set_local_name_support,
	artik_onoff_get_value,
	artik_ezmode_commissioning_target_start,
	artik_ezmode_commissioning_target_stop
};

const artik_zigbee_device_dimmable_light device_dimmable_light_func = {
	artik_groups_get_local_name_support,
	artik_groups_set_local_name_support,
	artik_onoff_get_value,
	artik_level_control_get_value,
	artik_ezmode_commissioning_target_start,
	artik_ezmode_commissioning_target_stop
};

const artik_zigbee_device_light_sensor device_light_sensor_func = {
	artik_identify_request,
	artik_identify_get_remaining_time,
	artik_illum_set_measured_value_range,
	artik_illum_set_measured_value,
	artik_illum_get_measured_value,
	artik_ezmode_commissioning_initiator_start,
	artik_ezmode_commissioning_initiator_stop
};

const artik_zigbee_device_remote_control device_remote_control_func = {
	artik_reset_to_factory_default,
	artik_identify_request,
	artik_identify_get_remaining_time,
	artik_onoff_command,
	artik_level_control_request,
	artik_request_reporting,
	artik_stop_reporting,
	artik_ezmode_commissioning_target_start,
	artik_ezmode_commissioning_target_stop
};

artik_error artik_zigbee_set_local_endpoint(
			artik_zigbee_local_endpoint_info * endpoint_info)
{
	return os_set_local_endpoint(endpoint_info);
}

artik_error artik_zigbee_get_local_endpoint(
			artik_zigbee_local_endpoint_info * endpoint_info)
{
	return os_get_local_endpoint(endpoint_info);
}

void artik_zigbee_deinitialize(void)
{
	os_deinitialize();
}

artik_error artik_zigbee_initialize(artik_zigbee_client_callback callback,
			void *user_data)
{
	return os_zigbee_initialize(callback, user_data);
}

artik_error artik_zigbee_network_start(artik_zigbee_network_state *state)
{
	return os_zigbee_network_start(state);
}

artik_error artik_zigbee_network_form_manually(
		const artik_zigbee_network_info * network_info)
{
	return os_zigbee_network_form_manually(network_info);
}

artik_error artik_zigbee_network_form(void)
{
	return os_zigbee_network_form();
}

artik_error artik_zigbee_network_permitjoin(int duration_sec)
{
	return os_zigbee_network_permitjoin(duration_sec);
}

artik_error artik_zigbee_network_leave(void)
{
	return os_zigbee_network_leave();
}

artik_error artik_zigbee_network_join(void)
{
	return os_zigbee_network_join();
}

artik_error artik_zigbee_network_stop_scan(void)
{
	return os_zigbee_network_stop_scan();
}

artik_error artik_zigbee_network_join_manually(
		const artik_zigbee_network_info * network_info)
{
	return os_zigbee_network_join_manually(network_info);
}

artik_error artik_zigbee_network_find(void)
{
	return os_zigbee_network_find();
}

artik_error artik_zigbee_network_request_my_network_status(
		artik_zigbee_network_state *state)
{
	return os_zigbee_network_request_my_network_status(state);
}

artik_error artik_zigbee_device_discover(void)
{
	return os_zigbee_device_discover();
}

artik_error artik_zigbee_set_discover_cycle_time(unsigned int cycle_duration)
{
	return os_zigbee_set_discover_cycle_time(cycle_duration);
}

artik_error artik_zigbee_get_discovered_device_list(
		artik_zigbee_device_info * device_info)
{
	return os_zigbee_get_discovered_device_list(device_info);
}

artik_error artik_zigbee_device_request_my_node_type(
		artik_zigbee_node_type * type)
{
	return os_zigbee_device_request_my_node_type(type);
}

void artik_zigbee_device_find_by_cluster(artik_zigbee_endpoint_list *endpoints,
		int cluster_id, int is_server)
{
	os_zigbee_device_find_by_cluster(endpoints, cluster_id, is_server);
}

void artik_zigbee_raw_request(const char *command)
{
	os_zigbee_raw_request(command);
}

artik_error artik_identify_request(artik_zigbee_endpoint_handle handle,
		const artik_zigbee_endpoint *endpoint,
		int duration)
{
	return os_identify_request(handle, endpoint, duration);
}

artik_error artik_identify_get_remaining_time(
		artik_zigbee_endpoint_handle handle,
		const artik_zigbee_endpoint *endpoint,
		int *time)
{
	return os_identify_get_remaining_time(handle, endpoint, time);
}

artik_error artik_onoff_command(artik_zigbee_endpoint_handle handle,
		const artik_zigbee_endpoint *endpoint,
		artik_zigbee_onoff_status target_status)
{
	return os_onoff_command(handle, endpoint, target_status);
}

artik_error artik_groups_get_local_name_support(
		artik_zigbee_endpoint_handle handle, int endpoint_id)
{
	return os_groups_get_local_name_support(handle, endpoint_id);
}

artik_error artik_groups_set_local_name_support(
		artik_zigbee_endpoint_handle handle, bool support)
{
	return os_groups_set_local_name_support(handle, support);
}

artik_error artik_onoff_get_value(artik_zigbee_endpoint_handle handle,
		artik_zigbee_onoff_status *status)
{
	return os_onoff_get_value(handle, status);
}

artik_error artik_level_control_request(artik_zigbee_endpoint_handle handle,
		const artik_zigbee_endpoint *endpoint,
		const artik_zigbee_level_control_command *command)
{
	return os_level_control_request(handle, endpoint, command);
}

artik_error artik_level_control_get_value(artik_zigbee_endpoint_handle handle,
		int *value)
{
	return os_level_control_get_value(handle, value);
}

artik_error artik_illum_set_measured_value_range(
		artik_zigbee_endpoint_handle handle,
		int min, int max)
{
	return os_illum_set_measured_value_range(handle, min, max);
}

artik_error artik_illum_set_measured_value(artik_zigbee_endpoint_handle handle,
		int value)
{
	return os_illum_set_measured_value(handle, value);
}

artik_error artik_illum_get_measured_value(artik_zigbee_endpoint_handle handle,
		int *value)
{
	return os_illum_get_measured_value(handle, value);
}

/* ezmode commissioning */
artik_error artik_ezmode_commissioning_initiator_start(
		artik_zigbee_endpoint_handle handle)
{
	return os_ezmode_commissioning(handle, true, true);
}

artik_error artik_ezmode_commissioning_initiator_stop(
		artik_zigbee_endpoint_handle handle)
{
	return os_ezmode_commissioning(handle, true, false);
}

artik_error artik_ezmode_commissioning_target_start(
		artik_zigbee_endpoint_handle handle)
{
	return os_ezmode_commissioning(handle, false, true);
}

artik_error artik_ezmode_commissioning_target_stop(
		artik_zigbee_endpoint_handle handle)
{
	return os_ezmode_commissioning(handle, false, false);
}

artik_error artik_zigbee_reset_local(void)
{
	return os_reset_local();
}

artik_error artik_reset_to_factory_default(artik_zigbee_endpoint_handle handle,
		const artik_zigbee_endpoint *endpoint)
{
	return os_reset_to_factory_default(handle, endpoint);
}

artik_error artik_request_reporting(artik_zigbee_endpoint_handle handle,
		const artik_zigbee_endpoint *endpoint,
		artik_zigbee_reporting_type report_type,
		int min_interval, int max_interval,
		int change_threshold)
{
	return os_request_reporting(handle, endpoint, report_type, min_interval,
			max_interval, change_threshold);
}

artik_error artik_stop_reporting(artik_zigbee_endpoint_handle handle,
		const artik_zigbee_endpoint *endpoint,
		artik_zigbee_reporting_type report_type)
{
	return os_stop_reporting(handle, endpoint, report_type);
}
