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
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <assert.h>
#include <artik_loop.h>
#include <artik_zigbee.h>
#include <artik_module.h>
#include <artik_platform.h>
#include <artik_log.h>
#include "artik_zigbee_test_common.h"

#define EASY_PJOIN_DURATION		0x3C
#define EASY_IDENTIFY_DURATION		10 /* seconds */
#define ILLUMINANCE_MIN			100
#define ILLUMINANCE_MAX			10000
#define ILLUMINANCE_STEP		50
#define TEMPERATURE_MIN			(-273.15f) /* celsius */
#define TEMPERATURE_MAX			(327.67f)
#define TEMPERATURE_STEP		(10.0f)
#define REPORTING_MAX_COUNT		3
#define CS_TARGET_TIMEOUT_SEC		60 /* steering will cost much time */
#define CS_INITIATOR_RETRY_COUNT	5
#define NWK_FIND_MAX_SIZE		8

static struct device_type device_types[] = {
	{ ARTIK_ZIGBEE_PROFILE_HA, ARTIK_ZIGBEE_DEVICE_ON_OFF_SWITCH,
	  1, "On/Off Switch", "ON_OFF_SWITCH" },
	{ ARTIK_ZIGBEE_PROFILE_HA, ARTIK_ZIGBEE_DEVICE_LEVEL_CONTROL_SWITCH,
	  2, "Level Control Switch", "LEVEL_CONTROL_SWITCH" },
	{ ARTIK_ZIGBEE_PROFILE_HA, ARTIK_ZIGBEE_DEVICE_ON_OFF_LIGHT,
	  19, "On/Off Light", "ON_OFF_LIGHT" },
	{ ARTIK_ZIGBEE_PROFILE_HA, ARTIK_ZIGBEE_DEVICE_DIMMABLE_LIGHT,
	  20, "Dimmable Light", "DIMMABLE_LIGHT" },
	{ ARTIK_ZIGBEE_PROFILE_HA, ARTIK_ZIGBEE_DEVICE_LIGHT_SENSOR,
	  25, "Light Sensor", "LIGHT_SENSOR" },
	{ ARTIK_ZIGBEE_PROFILE_HA, ARTIK_ZIGBEE_DEVICE_REMOTE_CONTROL,
	  34, "Remote Control", "REMOTE_CONTROL" },
	{ -1, -1, -1, NULL, NULL}
};

static func current_func;
static bool network_started;
static int illuminance_value = ILLUMINANCE_MIN;
static int illuminance_min = ILLUMINANCE_MIN;
static int illuminance_max = ILLUMINANCE_MAX;

static int reporting_measured_illum_count = 0;
static bool cs_initiator_testing;	/* initial value is false */
static int cs_initiator_testing_index = -1;
static int cs_initiator_retry;
static bool cs_target_testing;	/* initial value is false */
static int cs_target_testing_index = -1;
static int cs_target_timeout_id = -1;
artik_zigbee_ieee_addr_response addr_rsp;
artik_zigbee_simple_descriptor_response simple_descriptor;
artik_zigbee_network_find_result nwk_found_list[NWK_FIND_MAX_SIZE];
static int nwk_find_size = 0;
static bool initiator_reminder = true;
static bool conducted_by_commissioning = false;

static void _func_network(char *input, int max_size);
static void _func_network_find_join(char *input, int max_size);
static void _on_timer_callback(enum timer_command cmd, int id, void *user_data);
static void _on_callback(void *user_data,
			artik_zigbee_response_type response_type,
			void *payload);

static int _get_device_type_count(void)
{
	int count = 0;

	for (count = 0;; count++) {
		if (device_types[count].name == NULL)
			break;
	}

	return count;
}

static struct device_type *_get_device_type(int index)
{
	if (index >= _get_device_type_count())
		return NULL;

	return &device_types[index];
}

char *get_device_name(int device_id)
{
	int i = 0;

	while (device_types[i].name != NULL) {
		if (device_types[i].device_id == device_id)
			return device_types[i].name;
		i++;
	}

	return NULL;
}

static int _add_timer(enum timer_command cmd, int second)
{
	return add_timer(_on_timer_callback, cmd, second, NULL);
}

static artik_error _identify_test(artik_zigbee_module *zb)
{
	artik_zigbee_endpoint *endpoint;
	artik_zigbee_endpoint_list endpoint_list;
	struct test_device *test_device;
	int i, j, count;
	int remained_time, duration;
	artik_error ret = S_OK;
	bool is_supported;

	is_supported = false;
	count = get_test_device_count();
	for (i = 0; i < count; i++) {
		test_device = get_test_device(i);
		if (test_device->identify_request != NULL &&
			test_device->identify_get_remaining_time != NULL) {
			is_supported = true;
			break;
		}
	}

	if (is_supported == false)
		return E_NOT_SUPPORTED;

	zb->device_find_by_cluster(&endpoint_list,
					ARTIK_ZCL_IDENTIFY_CLUSTER_ID, 1);

	if (endpoint_list.num == 0) {
		showln("There is no endpoint to send identify command");
		return E_ZIGBEE_ERROR;
	}

	count = get_test_device_count();

	for (i = 0; i < count; i++) {
		test_device = get_test_device(i);

		if (test_device->identify_request == NULL ||
			test_device->identify_get_remaining_time == NULL) {
			continue;
		}

		showln("Identify: testing started by ep(%d)",
						test_device->endpoint_id);

		for (j = 0; j < endpoint_list.num; j++) {
			endpoint = &endpoint_list.endpoint[j];
			ret = test_device->identify_request(test_device->handle,
						endpoint,
						EASY_IDENTIFY_DURATION + i);
			if (ret == S_OK)
				showln("Identify: send request to node(0x%04X)",
						endpoint->node_id);
			else {
				showln("Identify: send request failed: %s",
						error_msg(ret));
				return ret;
			}
		}

		sleep((int)((EASY_IDENTIFY_DURATION + i) / 2));

		for (duration = 0; duration < (EASY_IDENTIFY_DURATION + i) / 2;
			 duration++) {
			for (j = 0; j < endpoint_list.num; j++) {
				endpoint = &endpoint_list.endpoint[j];
				ret = test_device->identify_get_remaining_time(
					test_device->handle, endpoint,
					&remained_time);

				if (ret != S_OK) {
					log_err("Identify: get remaining"\
						" time failed: %s",
							error_msg(ret));
					return ret;
				}

				showln("Identify: remained time: %d",
					remained_time);

				if (remained_time == 0)
					break;
			}

			sleep(1);
		}

		showln("Identify: testing succeeded by ep(%d)",
			   test_device->endpoint_id);
	}

	return S_OK;
}

static artik_error _onoff_test(artik_zigbee_module *zb)
{
	artik_zigbee_endpoint *endpoint;
	artik_zigbee_endpoint_list endpoint_list;
	struct test_device *test_device;
	artik_error ret = S_OK;
	int i, j, count;
	bool is_supported;

	is_supported = false;
	count = get_test_device_count();
	for (i = 0; i < count; i++) {
		test_device = get_test_device(i);
		if (test_device->onoff_command != NULL) {
			is_supported = true;
			break;
		}
	}

	if (is_supported == false)
		return E_NOT_SUPPORTED;

	zb->device_find_by_cluster(&endpoint_list, ARTIK_ZCL_ON_OFF_CLUSTER_ID,
									1);
	if (endpoint_list.num == 0) {
		showln("There is no endpoint to send on/off command");
		return E_ZIGBEE_ERROR;
	}

	count = get_test_device_count();
	for (i = 0; i < count; i++) {
		test_device = get_test_device(i);
		if (test_device->onoff_command == NULL)
			continue;

		showln("On/off: testing started by ep(%d)",
						test_device->endpoint_id);

		for (j = 0; j < endpoint_list.num; j++) {
			endpoint = &endpoint_list.endpoint[j];
			ret = test_device->onoff_command(test_device->handle,
						endpoint,
						ARTIK_ZIGBEE_ONOFF_TOGGLE);
			if (ret == S_OK)
				showln("On/off: send TOGGLE to node(0x%04X)",
					   endpoint->node_id);
			else {
				showln("On/off: send TOGGLE failed: %s",
					error_msg(ret));
				return ret;
			}

			sleep(1);

			ret = test_device->onoff_command(test_device->handle,
						endpoint,
						ARTIK_ZIGBEE_ONOFF_ON);
			if (ret == S_OK)
				showln("On/off: send ON to node(0x%04X)",
					endpoint->node_id);
			else {
				showln("On/off: send ON failed: %s",
					error_msg(ret));
				return ret;
			}

			sleep(1);

			ret = test_device->onoff_command(test_device->handle,
						endpoint,
						ARTIK_ZIGBEE_ONOFF_OFF);
			if (ret == S_OK)
				showln("On/off: send OFF to node(0x%04X)",
					   endpoint->node_id);
			else {
				showln("On/off: send OFF failed: %s",
					error_msg(ret));
				return ret;
			}

			sleep(1);
		}

		showln("On/off: testing succeeded by ep(%d)",
					test_device->endpoint_id);
	}

	return S_OK;
}

static artik_error _levelcontrol_test(artik_zigbee_module *zb, bool with_onoff)
{
	artik_zigbee_endpoint *endpoint;
	artik_zigbee_endpoint_list endpoint_list;
	artik_zigbee_level_control_command cmd;
	struct test_device *test_device;
	artik_error ret = S_OK;
	int i, j, count;
	bool is_supported;
	int testing_transition_time = 10;	/* 1 sec */

	is_supported = false;
	count = get_test_device_count();
	for (i = 0; i < count; i++) {
		test_device = get_test_device(i);
		if (test_device->level_control_request != NULL) {
			is_supported = true;
			break;
		}
	}

	if (is_supported == false)
		return E_NOT_SUPPORTED;

	zb->device_find_by_cluster(&endpoint_list,
					ARTIK_ZCL_LEVEL_CONTROL_CLUSTER_ID, 1);
	if (endpoint_list.num == 0) {
		showln("There is no endpoint to send level control command");
		return E_ZIGBEE_ERROR;
	}

	count = get_test_device_count();
	for (i = 0; i < count; i++) {
		test_device = get_test_device(i);
		if (test_device->level_control_request == NULL)
			continue;

		showln("Level control: testing started by ep(%d)",
			   test_device->endpoint_id);

		for (j = 0; j < endpoint_list.num; j++) {
			endpoint = &endpoint_list.endpoint[j];

			cmd.control_type = with_onoff ?
					ARTIK_ZIGBEE_MOVE_TO_LEVEL_ONOFF :
					ARTIK_ZIGBEE_MOVE_TO_LEVEL;
			cmd.parameters.move_to_level.level = 10;
			cmd.parameters.move_to_level.transition_time =
							testing_transition_time;
			ret = test_device->level_control_request(
							test_device->handle,
							endpoint, &cmd);
			if (ret == S_OK)
				showln("Level control: send MOVE TO %d"\
					" with transition time: %d %s"\
					" to node(0x%04X)",
				cmd.parameters.move_to_level.level,
				cmd.parameters.move_to_level.transition_time,
				with_onoff ? "with ONOFF" : "without ONOFF",
				endpoint->node_id);
			else {
				showln("Level control: send MOVE"\
					" TO %d %s failed: %s",
				cmd.parameters.move_to_level.level,
				with_onoff ? "with ONOFF" : "without ONOFF",
				error_msg(ret));
				return ret;
			}

			sleep(2);

			cmd.control_type = with_onoff ? ARTIK_ZIGBEE_MOVE_ONOFF
							: ARTIK_ZIGBEE_MOVE;
			cmd.parameters.move.control_mode =
						ARTIK_ZIGBEE_LEVEL_CONTROL_UP;
			cmd.parameters.move.rate = 5;
			ret = test_device->level_control_request(
							test_device->handle,
							endpoint, &cmd);
			if (ret == S_OK)
				showln("Level control: send MOVE UP with rate:"\
					" %d %s for node(0x%04X)",
				cmd.parameters.move.rate,
				with_onoff ? "with ONOFF" : "without ONOFF",
				endpoint->node_id);
			else {
				showln("Level control: send MOVE UP %s"\
					" failed: %s",
				with_onoff ? "with ONOFF" : "without ONOFF",
				error_msg(ret));
				return ret;
			}

			sleep(5);

			cmd.control_type = with_onoff ? ARTIK_ZIGBEE_STOP_ONOFF
							: ARTIK_ZIGBEE_STOP;
			ret = test_device->level_control_request(
					test_device->handle, endpoint, &cmd);
			if (ret == S_OK)
				showln("Level control: send STOP %s"\
					" for node(0x%04X)",
			with_onoff ? "with ONOFF" : "without ONOFF",
			endpoint->node_id);
			else {
				showln("Level control: send STOP %s failed: %s",
				with_onoff ? "with ONOFF" : "without ONOFF",
				error_msg(ret));
				return ret;
			}

			sleep(2);

			cmd.control_type = with_onoff ? ARTIK_ZIGBEE_STEP_ONOFF
							: ARTIK_ZIGBEE_STEP;
			cmd.parameters.step.control_mode =
						ARTIK_ZIGBEE_LEVEL_CONTROL_DOWN;
			cmd.parameters.step.step_size = 10;
			cmd.parameters.step.transition_time =
							testing_transition_time;
			ret = test_device->level_control_request(
							test_device->handle,
							endpoint, &cmd);
			if (ret == S_OK)
				showln("Level control: send STEP DOWN with"\
					" step size: %d transition time:"\
					" %d %s for node(0x%04X)",
				cmd.parameters.step.step_size,
				cmd.parameters.step.transition_time,
				with_onoff ? "with ONOFF" : "without ONOFF",
				endpoint->node_id);
			else {
				showln("Level control: send STEP DOWN %s"\
					" failed: %s",
				with_onoff ? "with ONOFF" : "without ONOFF",
				error_msg(ret));
				return ret;
			}

			sleep(5);

			cmd.control_type = with_onoff ? ARTIK_ZIGBEE_STOP_ONOFF
							: ARTIK_ZIGBEE_STOP;
			ret = test_device->level_control_request(
							test_device->handle,
							endpoint, &cmd);
			if (ret == S_OK)
				showln("Level control: send STOP %s"\
					" for node(0x%04X)",
				with_onoff ? "with ONOFF" : "without ONOFF",
				endpoint->node_id);
			else {
				showln("Level control: send STOP %s failed: %s",
				with_onoff ? "with ONOFF" : "without ONOFF",
				error_msg(ret));
				return ret;
			}

			sleep(2);
		}

		showln("Level control: testing succeeded by ep(%d)",
			   test_device->endpoint_id);
	}

	return S_OK;
}

static artik_error _illuminance_test(void)
{
	struct test_device *test_device;
	artik_error ret = S_OK;
	int i, count;
	bool is_supported;
	int ill_value;

	is_supported = false;
	count = get_test_device_count();
	for (i = 0; i < count; i++) {
		test_device = get_test_device(i);
		if (test_device->illum_set_measured_value != NULL &&
			test_device->illum_set_measured_value_range != NULL) {
			is_supported = true;
			break;
		}
	}

	if (is_supported == false)
		return E_NOT_SUPPORTED;

	for (i = 0; i < get_test_device_count(); i++) {
		test_device = get_test_device(i);
		if (test_device->illum_set_measured_value == NULL)
			continue;

		showln("Illuminance: testing started by ep(%d)",
			   test_device->endpoint_id);

		illuminance_min += ILLUMINANCE_STEP;
		illuminance_max -= ILLUMINANCE_STEP;
		illuminance_value += (ILLUMINANCE_STEP * 2);

		if (illuminance_min >= illuminance_max) {
			illuminance_min = ILLUMINANCE_MIN;
			illuminance_max = ILLUMINANCE_MAX;
		}

		if (illuminance_value < illuminance_min ||
			illuminance_value > illuminance_max)
			illuminance_value = illuminance_min;

		ret = test_device->illum_set_measured_value_range(
							test_device->handle,
							illuminance_min,
							illuminance_max);
		if (ret == S_OK)
			showln("Set illuminace range to %d - %d",
					illuminance_min, illuminance_max);
		else {
			showln("Set illuminace range failed: %s",
					error_msg(ret));
			return ret;
		}

		sleep(1);
		ret = test_device->illum_get_measured_value(test_device->handle,
								&ill_value);
		if (ret == S_OK)
			showln("Get illuminace %d", ill_value);
		else {
			showln("Get illuminace failed: %s", error_msg(ret));
			return ret;
		}

		sleep(1);
		ret = test_device->illum_set_measured_value(test_device->handle,
							illuminance_value);
		if (ret == S_OK)
			showln("Set illuminace to %d", illuminance_value);
		else {
			showln("Set illuminace failed: %s", error_msg(ret));
			return ret;
		}

		sleep(1);
		ret = test_device->illum_get_measured_value(test_device->handle,
								&ill_value);
		if (ret == S_OK)
			showln("Get illuminace %d", ill_value);
		else {
			showln("Get illuminace failed: %s", error_msg(ret));
			return ret;
		}
		showln("Illuminance: testing succeeded by ep(%d)",
			   test_device->endpoint_id);
	}

	return S_OK;
}

static artik_error _do_basic_remote_control(artik_zigbee_module *zb)
{
	artik_zigbee_endpoint_list endpoint_list;
	artik_zigbee_endpoint *endpoint = NULL;
	struct test_device *test_device = NULL;
	artik_error ret = E_ZIGBEE_ERROR;
	int i, j, count = 0;
	bool is_supported = false;

	count = get_test_device_count();
	for (i = 0; i < count; i++) {
		test_device = get_test_device(i);
		if (test_device->reset_to_factory_default != NULL) {
			is_supported = true;
			break;
		}
	}

	if (is_supported == false)
		return E_NOT_SUPPORTED;

	zb->device_find_by_cluster(&endpoint_list, ARTIK_ZCL_BASIC_CLUSTER_ID,
									1);
	if (endpoint_list.num == 0) {
		showln("There is no endpoint for basic remote control");
		return E_ZIGBEE_ERROR;
	}

	count = get_test_device_count();
	for (i = 0; i < count; i++) {
		test_device = get_test_device(i);

		if (test_device->reset_to_factory_default == NULL)
			continue;

		showln("Remote control basic: testing started by ep(%d)",
				test_device->endpoint_id);

		for (j = 0; j < endpoint_list.num; j++) {
			endpoint = &endpoint_list.endpoint[j];

			ret = test_device->reset_to_factory_default(
							test_device->handle,
							endpoint);
			if (ret == S_OK)
				showln("Success to reset to factory default"\
					" to node 0x%04X",
					endpoint->node_id);
			else {
				showln("Failed to reset to factory default"\
					" for result: %d",
					ret);
				return ret;
			}

			sleep(2);
		}

		showln("Remote control: testing succeeded by ep(%d)",
			   test_device->endpoint_id);
	}

	return S_OK;
}

static artik_error _stop_reporting(artik_zigbee_reporting_type reporting_type)
{
	artik_zigbee_endpoint_list endpoint_list;
	artik_zigbee_endpoint *endpoint = NULL;
	struct test_device *test_device = NULL;
	artik_error ret = E_ZIGBEE_ERROR;
	int i, j, count = 0;
	bool is_supported = false;
	artik_zigbee_module *zb = NULL;

	count = get_test_device_count();
	for (i = 0; i < count; i++) {
		test_device = get_test_device(i);
		if (test_device->stop_reporting != NULL) {
			is_supported = true;
			break;
		}
	}

	if (is_supported == false)
		return E_NOT_SUPPORTED;

	zb = (artik_zigbee_module *) artik_request_api_module("zigbee");

	switch (reporting_type) {
	case ARTIK_ZIGBEE_REPORTING_MEASURED_TEMPERATURE:
		zb->device_find_by_cluster(&endpoint_list,
				ARTIK_ZCL_TEMP_MEASUREMENT_CLUSTER_ID, 1);
		break;

	case ARTIK_ZIGBEE_REPORTING_MEASURED_ILLUMINANCE:
		zb->device_find_by_cluster(&endpoint_list,
				ARTIK_ZCL_ILLUM_MEASUREMENT_CLUSTER_ID, 1);
		break;

	case ARTIK_ZIGBEE_REPORTING_OCCUPANCY_SENSING:
		zb->device_find_by_cluster(&endpoint_list,
				ARTIK_ZCL_OCCUPANCY_SENSING_CLUSTER_ID, 1);
		break;

	case ARTIK_ZIGBEE_REPORTING_THERMOSTAT_TEMPERATURE:
		zb->device_find_by_cluster(&endpoint_list,
				ARTIK_ZCL_THERMOSTAT_CLUSTER_ID, 1);
		break;

	default:
		break;
	}

	if (endpoint_list.num == 0) {
		showln("There is no endpoint to stop reporting");
		artik_release_api_module(zb);
		return E_ZIGBEE_ERROR;
	}

	count = get_test_device_count();
	for (i = 0; i < count; i++) {
		test_device = get_test_device(i);

		if (test_device->stop_reporting == NULL)
			continue;

		showln("Stop reporting: testing starting by ep(%d)",
			   test_device->endpoint_id);

		for (j = 0; j < endpoint_list.num; j++) {
			endpoint = &endpoint_list.endpoint[j];

			ret = test_device->stop_reporting(test_device->handle,
								endpoint,
								reporting_type);
			if (ret != ARTIK_ZIGBEE_CMD_SUCCESS)
				showln("Failed to stop reporting %d for"\
					" result: %d",
					reporting_type, ret);
			else
				showln("Success to stop reporting %d to"\
					" node 0x%04x",
					reporting_type, endpoint->node_id);
		}

		showln("Stop reporting: testing ending by ep(%d)",
			   test_device->endpoint_id);
	}

	artik_release_api_module(zb);
	return ret;
}

static artik_error _measured_illuminance_reporting_test(artik_zigbee_module *zb)
{
	artik_zigbee_endpoint_list endpoint_list;
	artik_zigbee_endpoint *endpoint = NULL;
	struct test_device *test_device = NULL;
	artik_error ret = E_ZIGBEE_ERROR;
	int i, j, count = 0;
	bool is_supported = false;

	int min_interval = 1;
	int max_interval = 10;
	int illuminance = 10;

	count = get_test_device_count();
	for (i = 0; i < count; i++) {
		test_device = get_test_device(i);
		if (test_device->request_reporting != NULL) {
			is_supported = true;
			break;
		}
	}

	if (is_supported == false)
		return E_NOT_SUPPORTED;

	zb->device_find_by_cluster(&endpoint_list,
					ARTIK_ZCL_ILLUM_MEASUREMENT_CLUSTER_ID,
					1);
	if (endpoint_list.num == 0) {
		showln("There is no endpoint to request measured illuminance"\
			" reporting");
		return E_ZIGBEE_ERROR;
	}

	count = get_test_device_count();
	for (i = 0; i < count; i++) {
		test_device = get_test_device(i);

		if (test_device->request_reporting == NULL)
			continue;

		showln("Request measured illuminance reporting: testing"\
			" started by ep(%d)", test_device->endpoint_id);

		for (j = 0; j < endpoint_list.num; j++) {
			endpoint = &endpoint_list.endpoint[j];

			ret = test_device->request_reporting(
				test_device->handle, endpoint,
				ARTIK_ZIGBEE_REPORTING_MEASURED_ILLUMINANCE,
				min_interval, max_interval, illuminance);
			if (ret != ARTIK_ZIGBEE_CMD_SUCCESS)
				showln("Failed to request measured illuminance"\
					" reporting for result: %d", ret);
			else
				showln("Success to request measured"\
					" illuminance reporting to node 0x%04x",
					endpoint->node_id);
		}

		showln("Request measured illuminance reporting: testing"\
			" succeeded by ep(%d)", test_device->endpoint_id);
	}

	return ret;
}

static artik_error _remote_control_test(artik_zigbee_module *zb)
{
	struct test_device *test_device = NULL;
	int i, count = 0;
	bool is_supported = false;

	count = get_test_device_count();
	for (i = 0; i < count; i++) {
		test_device = get_test_device(i);
		if (test_device->reset_to_factory_default != NULL
				&& test_device->request_reporting != NULL) {
			is_supported = true;
			break;
		}
	}

	if (is_supported == false)
		return E_NOT_SUPPORTED;

	_do_basic_remote_control(zb);
	_measured_illuminance_reporting_test(zb);
	showln("Remote control: testing succeeded");
	return S_OK;
}

static artik_error _commissioning_target_start(artik_zigbee_module *zb,
								int index)
{
	struct test_device *test_device;
	int i, count, id;
	bool is_supported;
	artik_error ret = S_OK;

	is_supported = false;
	count = get_test_device_count();
	for (i = 0; i < count; i++) {
		test_device = get_test_device(i);
		if (test_device->ezmode_commissioning_target_start != NULL &&
			test_device->ezmode_commissioning_target_stop != NULL) {
			is_supported = true;
			break;
		}
	}

	if (is_supported == false)
		return E_NOT_SUPPORTED;

	cs_target_testing_index = -1;
	count = get_test_device_count();
	for (i = index; i < count; i++) {
		test_device = get_test_device(i);
		if (test_device->ezmode_commissioning_target_start == NULL ||
			test_device->ezmode_commissioning_target_stop == NULL)
			continue;

		showln("Target: testing started by ep(%d)",
						test_device->endpoint_id);

		ret = test_device->ezmode_commissioning_target_start(
						test_device->handle);

		if (ret == S_OK)
			cs_target_testing_index = i;
		else
			showln("Commissioning target start failed for"\
				" result %d", ret);

		sleep(1);
		break;
	}

	if (cs_target_testing_index == -1) {
		cs_target_testing = false;	/* no more test devices */
		remove_timer(cs_target_timeout_id);
		showln("Done");
		showln("Device discovering ...");
		ret = zb->device_discover();
		if (ret != S_OK)
			log_err("Device discover failed: %s", error_msg(ret));
	} else {
		id = _add_timer(TIMER_CS_TARGET_TIMEOUT, CS_TARGET_TIMEOUT_SEC);
		if (id > 0) {
			cs_target_timeout_id = id;
			cs_target_testing = true;
		} else {
			cs_target_testing = false;
			log_err("Add timer failed");
		}
	}

	return ret;
}

static artik_error _commissioning_target_run(artik_zigbee_module *zb,
						enum cs_target_command cmd)
{
	struct test_device *test_device;
	artik_error result = S_OK;

	if (cs_target_testing == false) {
		switch (cmd) {
		case CS_TARGET_START:
			result = _commissioning_target_start(zb, 0);
			break;
		case CS_TARGET_STOP:
			showln("Target: testing didn't start");
			break;
		case CS_TARGET_TIMEOUT:
			/* ignore this */
			break;
		case CS_TARGET_SUCCESS:
			showln("Commissioning: target success");
			break;
		case CS_TARGET_FAILED:
			showln("Commissioning: target failed");
			break;
		}
	} else {
		/* read current testing device */
		if (cs_target_testing_index < 0) {
			showln("Target: testing error with bad index(%d)",
				   cs_target_testing_index);
			cs_target_testing = false;
			showln("Done");
			result = E_INVALID_VALUE;
			goto end;
		}
		test_device = get_test_device(cs_target_testing_index);
		if (test_device == NULL) {
			showln("Target: testing error with bad index(%d)",
				   cs_target_testing_index);
			cs_target_testing = false;
			showln("Done");
			result = E_INVALID_VALUE;
			goto end;
		}

		switch (cmd) {
		case CS_TARGET_START:
			showln("Target: testing interrupted by ep(%d)",
				   test_device->endpoint_id);
			/* restart testing */
			result = _commissioning_target_start(zb, 0);
			break;
		case CS_TARGET_STOP:
			result = test_device->ezmode_commissioning_target_stop(
							test_device->handle);
			showln("Target: testing stopped by ep(%d) for"\
				" result %d",
				   test_device->endpoint_id, result);
			cs_target_testing = false;
			break;
		case CS_TARGET_TIMEOUT:
			showln("Target: testing timeout by ep(%d)",
				   test_device->endpoint_id);
			result = test_device->ezmode_commissioning_target_stop(
				test_device->handle);
			showln("Target: testing stopped by ep(%d) for"\
				" result %d",
				   test_device->endpoint_id, result);
			result = _commissioning_target_start(zb,
						cs_target_testing_index + 1);
			break;
		case CS_TARGET_SUCCESS:
			if (initiator_reminder) {
				showln("Target: waiting for initiator,"\
					" please run initiator commissioning");
				initiator_reminder = false;
			} else
				showln("Target: start success");
			break;
		case CS_TARGET_FAILED:
			showln("Target: testing failed by ep(%d)",
				   test_device->endpoint_id);
			result = _commissioning_target_start(zb,
						cs_target_testing_index + 1);
			break;
		}
	}

end:

	return result;
}

static artik_error _commissioning_initiator_start(artik_zigbee_module *zb,
						int index, int retry_count)
{
	struct test_device *test_device;
	int i, count;
	bool is_supported;
	artik_error result = S_OK;

	is_supported = false;
	count = get_test_device_count();
	for (i = 0; i < count; i++) {
		test_device = get_test_device(i);
		if (test_device->ezmode_commissioning_initiator_start != NULL &&
			test_device->ezmode_commissioning_initiator_stop
								!= NULL) {
			is_supported = true;
			break;
		}
	}

	if (is_supported == false)
		return E_NOT_SUPPORTED;

	cs_initiator_testing_index = -1;
	count = get_test_device_count();
	for (i = index; i < count; i++) {
		test_device = get_test_device(i);
		if (test_device->ezmode_commissioning_initiator_start == NULL ||
			test_device->ezmode_commissioning_initiator_stop
									== NULL)
			continue;

		showln("Initiator: testing started by ep(%d)",
						test_device->endpoint_id);

		showln("Target should run prior to start this");
		showln("You can stop initiator commissioning test through"\
			" command:\"Commissioning initiator stop\"");
		sleep(1);

		result = test_device->ezmode_commissioning_initiator_start(
							test_device->handle);

		if (result == S_OK) {
			showln("Initiator: started...");
			cs_initiator_testing_index = i;
		} else
			showln("Commissioning initiator start failed");
		break;
	}

	if (cs_initiator_testing_index == -1) {
		cs_initiator_testing = false;	/* no more test devices */
		showln("Done");
		showln("Device discovering ...");
		result = zb->device_discover();
		if (result != S_OK)
			log_err("Device discover failed: %s",
							error_msg(result));
	} else {
		cs_initiator_testing = true;
		cs_initiator_retry = retry_count;
	}

	return result;
}

static artik_error _commissioning_initiator_run(artik_zigbee_module *zb,
						enum cs_initiator_command cmd)
{
	struct test_device *test_device;
	artik_error result = S_OK;

	if (cs_initiator_testing == false) {
		switch (cmd) {
		case CS_INITIATOR_START:
			result = _commissioning_initiator_start(zb, 0,
						CS_INITIATOR_RETRY_COUNT);
			break;
		case CS_INITIATOR_STOP:
			showln("Initiator: testing didn't start");
			break;
		case CS_INITIATOR_SUCCESS:
			showln("Commissioning: initiator success");
			break;
		case CS_INITIATOR_FAILED:
			showln("Commissioning: initiator failed");
			break;
		}
	} else {
		/* read current testing device */
		if (cs_initiator_testing_index < 0) {
			showln("Initiator: testing error with bad index(%d)",
				   cs_initiator_testing_index);
			cs_initiator_testing = false;
			showln("Done");
			result = E_INVALID_VALUE;
			goto end;
		}
		test_device = get_test_device(cs_initiator_testing_index);
		if (test_device == NULL) {
			showln("Initiator: testing error with bad index(%d)",
				   cs_initiator_testing_index);
			cs_initiator_testing = false;
			showln("Done");
			result = E_INVALID_VALUE;
			goto end;
		}

		switch (cmd) {
		case CS_INITIATOR_START:
			showln("Initiator: testing interrupted by ep(%d)",
				   test_device->endpoint_id);
			/* restart testing */
			result = _commissioning_initiator_start(zb, 0,
						CS_INITIATOR_RETRY_COUNT);
			break;
		case CS_INITIATOR_STOP:
			result =
			test_device->ezmode_commissioning_initiator_stop(
							test_device->handle);
			showln("Initiator: testing stopped by ep(%d) for"\
				" result %d",
				   test_device->endpoint_id, result);
			cs_initiator_testing = false;
			break;
		case CS_INITIATOR_SUCCESS:
			showln("Initiator: testing succeeded by ep(%d)",
				   test_device->endpoint_id);
			result = _commissioning_initiator_start(zb,
					cs_initiator_testing_index + 1,
					CS_INITIATOR_RETRY_COUNT);
			break;
		case CS_INITIATOR_FAILED:
			if (cs_initiator_retry-- > 0) {
				showln("Initiator: failed, retry...");
				result = _commissioning_initiator_start(zb,
					cs_initiator_testing_index,
					cs_initiator_retry);
			} else {
				showln("Initiator: testing failed by ep(%d)",
					   test_device->endpoint_id);
				result = _commissioning_initiator_start(zb,
						cs_initiator_testing_index + 1,
						CS_INITIATOR_RETRY_COUNT);
			}
			break;
		}
	}

end:
	return result;
}

static void _func_entry(func func)
{
	current_func = func;
	current_func(NULL, 0);
}

static void _func_main_auto_test(artik_zigbee_module *zb)
{
	_identify_test(zb);
	_onoff_test(zb);
	_levelcontrol_test(zb, false);
	_levelcontrol_test(zb, true);
	_illuminance_test();
	_remote_control_test(zb);
}

static artik_error _func_main_test(artik_zigbee_module *zb, int n,
								bool *show_done)
{
	artik_error result;

	*show_done = true;
	result = S_OK;
	switch (n) {
	case 3:	/* Identify */
		result = _identify_test(zb);
		break;

	case 4:	/* On/off */
		result = _onoff_test(zb);
		break;

	case 5:	/* Level control */
		result = _levelcontrol_test(zb, false);
		result = _levelcontrol_test(zb, true);
		break;

	case 6:	/* Illuminance measurement */
		result = _illuminance_test();
		break;

	case 7: /* Remote control */
		result = _remote_control_test(zb);
		break;

	case 8:	/* commissioning target start */
		initiator_reminder = true;
		conducted_by_commissioning = true;
		result = _commissioning_target_run(zb, CS_TARGET_START);
		*show_done = false;
		break;

	case 9:	/* commissioning target stop */
		result = _commissioning_target_run(zb, CS_TARGET_STOP);
		break;

	case 10:	/* commissioning initiator start */
		conducted_by_commissioning = true;
		result = _commissioning_initiator_run(zb, CS_INITIATOR_START);
		*show_done = false;
		break;

	case 11:	/* commissioning initiator stop */
		result = _commissioning_initiator_run(zb, CS_INITIATOR_STOP);
		break;

	default:
		result = E_TRY_AGAIN;
		break;
	}

	return result;
}

static void _func_main_show(void)
{
	showln("1: Get device information");
	showln("2: Auto test");
	showln("3: Identify");
	showln("4: On/off");
	showln("5: Level control");
	showln("6: Illuminance measurement");
	showln("7: Remote Control");
	showln("8: Commissioning target start");
	showln("9: Commissioning target stop");
	showln("10: Commissioning initiator start");
	showln("11: Commissioning initiator stop");
	showln("0: Setup network");
	showln("e: Exit (Quit with calling clean and reset device)");
	showln("q: Exit");
}

static void _func_main(char *input, int max_size)
{
	static int step;
	artik_zigbee_module *zb;
	artik_zigbee_device_info device_info;
	int n;
	bool show_prompt, show_done;
	artik_error ret;

	if (input == NULL)
		step = 0;

	zb = (artik_zigbee_module *)artik_request_api_module("zigbee");
	switch (step) {
	case 0:
		show_hyphen();
		_func_main_show();
		show_select();
		step = 1;
		break;

	case 1:
		if (read_q(input, max_size) == true) {
			artik_release_api_module(zb);
			exit_loop();
			return;
		}

		if (read_e(input, max_size) == true) {
			ret = zb->reset_local();
			if (ret != S_OK)
				log_err("clean and reset device failed:%s",
					error_msg(ret));
			else
				showln("clean and reset device success");
			artik_release_api_module(zb);
			exit_loop();
			return;
		}

		if (read_lf(input, max_size) == true) {
			show_hyphen();
			_func_main_show();
			show_select();
			break;
		}

		show_prompt = true;
		n = read_int(input, max_size, -1);

		switch (n) {
		case 1:	/* Get device information */
			ret = zb->get_discovered_device_list(&device_info);
			if (ret == S_OK)
				show_device_info(&device_info);
			else if (ret == E_ZIGBEE_NO_DEVICE)
				showln("No device");
			else
				showln("Get device info failed: %s",
					error_msg(ret));
			break;

		case 2:	/* Auto test */
			_func_main_auto_test(zb);
			showln("Done");
			break;

		case 0:	/* Setup network */
			_func_entry(_func_network);
			show_prompt = false;
			break;

		default:
			ret = _func_main_test(zb, n, &show_done);
			if (ret == S_OK) {
				if (show_done)
					showln("Done");
			} else if (ret == E_NOT_SUPPORTED)
				showln("No device to process this test");
			else if (ret == E_TRY_AGAIN) {
				show_retry();
				show_prompt = false;
			} else
				showln("Test failed: %s", error_msg(ret));
			break;
		}

		if (show_prompt) {
			show_hyphen();
			_func_main_show();
			show_select();
		}
		break;
	}

	artik_release_api_module(zb);
}

static void _func_network_show(artik_zigbee_module *zb)
{
	artik_zigbee_network_state state;
	artik_zigbee_node_type type;
	artik_error ret;
	bool network;

	network = false;
	ret = zb->network_request_my_network_status(&state);
	if (ret == S_OK) {
		show_network_status(state);
		if (state != ARTIK_ZIGBEE_NO_NETWORK) {
			network = true;
			ret = zb->device_request_my_node_type(&type);
			if (ret == S_OK)
				show_node_type(type);
			else
				log_err("Get device node type failed: %s",
					error_msg(ret));
		} else
			showln("Network: Non Exist");
	} else
		log_err("Get network status failed: %s", error_msg(ret));

	show_hyphen();
	showln("1: Form network");
	showln("2: Form network (advance)");
	showln("3: Join network");
	showln("4: Join network (advance)");
	showln("5: Leave network");
	showln("6: Get network state");
	showln("7: Discover device");

	if (network)
		showln("8: Start testing");
	else
		showln("8: Start testing without network");

	showln("9: Network permit join");
	showln("10: Network find and join");
	showln("11: Network stop scan");
	showln("e: Exit (Quit with calling clean and reset device)");
	showln("q: Exit");
}

static void _func_network_found_show(void)
{
	int i = 1;

	show_hyphen();
	if (nwk_find_size <= 0)
		showln("No network found, please find network firstly");
	else {
		showln("Select one network to join");
		for (i = 1; i <= nwk_find_size; i++)
			showln("%d: network channel(%d), tx power(%d),"\
				" pan id(0x%04X)", i,
				nwk_found_list[i - 1].network_info.channel,
				nwk_found_list[i - 1].network_info.tx_power,
				nwk_found_list[i - 1].network_info.pan_id);
	}
}

static void _func_network_found_end_show(void)
{
	showln("9: Find Network");
	showln("0: Setup Network");
	showln("e: Exit (Quit with calling clean and reset device)");
	showln("q: Exit");
	show_hyphen();
	show_select();
}

static artik_error _find_network(artik_zigbee_module *zb)
{
	artik_error ret;

	showln("Finding network ...");
	showln("Select one network to join");
	nwk_find_size = 0;
	ret = zb->network_find();
	if (ret != S_OK) {
		log_err("Find network failed:%s", error_msg(ret));
		showln("Done");
	}
	return ret;
}

static void _func_network_find_join(char *input, int max_size)
{
	static int step;
	artik_zigbee_module *zb;
	int n;

	artik_error ret = E_ZIGBEE_ERROR;

	if (input == NULL)
		step = 0;

	zb = (artik_zigbee_module *) artik_request_api_module("zigbee");
	switch (step) {
	case 0:
		ret = _find_network(zb);
		step = 1;
		break;

	case 1:
		if (read_q(input, max_size)) {
			artik_release_api_module(zb);
			exit_loop();
			return;
		}

		if (read_e(input, max_size) == true) {
			ret = zb->reset_local();
			if (ret != S_OK)
				log_err("clean and reset device failed:%s",
					error_msg(ret));
			else
				showln("clean and reset device success");
			artik_release_api_module(zb);
			exit_loop();
			return;
		}

		if (read_lf(input, max_size) == true) {
			_func_network_found_show();
			_func_network_found_end_show();
			break;
		}

		n = read_int(input, max_size, -1);
		switch (n) {
		case 0:
			_func_entry(_func_network);
			break;

		case 9:
			ret = _find_network(zb);
			break;

		default:
			if (nwk_find_size <= 0)
				showln("No network found, please find"\
					" network firstly");
			else if (n > nwk_find_size)
				showln("The selection is out of the"\
					" network found size %d",
						nwk_find_size);
			else if (n < 0)
				showln("Please input legal value");
			else {
				showln("Join network channel(%d), tx power(%d)"\
					", pan id(0x%04X)",
					nwk_found_list[n - 1].\
					network_info.channel,
					nwk_found_list[n - 1].\
					network_info.tx_power,
					nwk_found_list[n - 1].\
					network_info.pan_id);
				ret = zb->network_join_manually(
					&nwk_found_list[n - 1].network_info);
				conducted_by_commissioning = false;
				if (ret != S_OK) {
					log_err("Join network failed: %s",
						error_msg(ret));
					showln("Done");
				} else {
					showln("Done");
					showln("Device discovering ...");
					ret = zb->device_discover();
					if (ret != S_OK)
						log_err("Device discover"\
							" failed: %s",
							error_msg(ret));
				}
			}
			break;
		}
		break;
	}

	artik_release_api_module(zb);
}

static void _func_network(char *input, int max_size)
{
	static artik_zigbee_network_info set_network_info;
	static int step;
	artik_zigbee_module *zb;
	artik_zigbee_network_state state;
	artik_zigbee_node_type type;
	int n;

	artik_error ret = E_ZIGBEE_ERROR;

	if (input == NULL)
		step = 0;

	zb = (artik_zigbee_module *)artik_request_api_module("zigbee");
	switch (step) {
	case 0:
		if (network_started == false) {
			ret = zb->network_start(&state);
			if (ret == S_OK && state == ARTIK_ZIGBEE_JOINED_NETWORK)
				showln("Former Network Resumed");
			network_started = true;
		}

		_func_network_show(zb);
		show_select();
		step = 1;
		break;

	case 1:
		if (read_q(input, max_size)) {
			artik_release_api_module(zb);
			exit_loop();
			return;
		}

		if (read_e(input, max_size) == true) {
			ret = zb->reset_local();
			if (ret != S_OK)
				log_err("clean and reset device failed:%s",
					error_msg(ret));
			else
				showln("clean and reset device success");
			artik_release_api_module(zb);
			exit_loop();
			return;
		}

		if (read_lf(input, max_size) == true) {
			show_hyphen();
			_func_network_show(zb);
			show_select();
			break;
		}

		n = read_int(input, max_size, -1);
		switch (n) {
		case 1:	/* Form network */
			ret = zb->network_form();
			conducted_by_commissioning = false;
			if (ret == S_OK)
				showln("Wait for response");
			else {
				showln("Network form failed");
				showln("Done");
			}
			break;

		case 2:	/* Form network (advance) */
			show_request_channel();
			step = 2;
			break;

		case 3:	/* Join network */
			ret = zb->network_join();
			conducted_by_commissioning = false;
			if (ret == S_OK)
				showln("Wait for response");
			else {
				showln("Network join failed");
				showln("Done");
			}
			break;

		case 4:	/* Join network (advance) */
			show_request_channel();
			step = 5;
			break;

		case 5:	/* Leave network */
			ret = zb->network_leave();
			if (ret != S_OK)
				log_err("Network leave failed: %s\n",
					error_msg(ret));
			showln("Done");
			break;

		case 6:	/* Get network state */
			ret = zb->network_request_my_network_status(&state);
			if (ret == S_OK)
				show_network_status(state);
			else
				log_err("Get network status failed: %s",
					error_msg(ret));
			ret = zb->device_request_my_node_type(&type);
			if (ret == S_OK)
				show_node_type(type);
			else
				log_err("Get device node type failed: %s",
					error_msg(ret));
			break;

		case 7:	/* Discover device */
			ret = zb->set_discover_cycle_time(10);
			if (ret == S_OK)
				showln("Set discover cycle time to 10 minutes");
			else
				log_err("Set discover cycle time error: %s",
					error_msg(ret));

			ret = zb->device_discover();
			if (ret == S_OK)
				showln("Wait response");
			else {
				log_err("Network discover failed: %s\n",
					error_msg(ret));
				showln("Done");
			}
			break;

		case 8:	/* Start testing */
			_func_entry(_func_main);
			break;

		case 9:
			ret = zb->network_permitjoin(EASY_PJOIN_DURATION);
			if (ret != S_OK)
				log_err("Permit join failed:%s",
					error_msg(ret));
			else
				showln("Permit join for %d seconds",
					EASY_PJOIN_DURATION);
			showln("Done");
			break;

		case 10:
			_func_entry(_func_network_find_join);
			break;

		case 11:
			ret = zb->network_stop_scan();
			if (ret != S_OK)
				log_err("Network stop scanning failed: %s",
					error_msg(ret));
			else
				showln("Network stop scanning success");
			showln("Done");
			break;

		default:
			show_retry();
			break;
		}

		break;

	case 2:	/* network form - set channel */
		ret = read_channel(input, max_size, &set_network_info.channel);
		if (ret == S_OK) {
			show_request_tx();
			step = 3;
		}
		break;

	case 3:	/* network form - set tx */
		ret = read_tx(input, max_size, &set_network_info.tx_power);
		if (ret == S_OK) {
			show_request_pan_id();
			step = 4;
		}
		break;

	case 4:	/* network form - set pan id */
		ret = read_pan_id(input, max_size, &set_network_info.pan_id);
		if (ret == S_OK) {
			showln("Form network channel(%d) TX(%d) PAN ID(0x%x):",
				   set_network_info.channel,
				   set_network_info.tx_power,
				   set_network_info.pan_id);
			ret = zb->network_form_manually(&set_network_info);
			conducted_by_commissioning = false;
			if (ret != S_OK)
				log_err("Manually form network failed: %s",
					error_msg(ret));
			else
				showln("Manually form network success\nDone");
			step = 1;
		}
		break;

	case 5:	/* network join - set channel */
		ret = read_channel(input, max_size, &set_network_info.channel);
		if (ret == S_OK) {
			show_request_tx();
			step = 6;
		}
		break;

	case 6:	/* network join - set tx */
		ret = read_tx(input, max_size, &set_network_info.tx_power);
		if (ret == S_OK) {
			show_request_pan_id();
			step = 7;
		}
		break;

	case 7:	/* network join - set pan id */
		ret = read_pan_id(input, max_size, &set_network_info.pan_id);
		if (ret == S_OK) {
			showln("Join network channel(%d) TX(%d) PAN ID(0x%x):",
				   set_network_info.channel,
				   set_network_info.tx_power,
				   set_network_info.pan_id);
			ret = zb->network_join_manually(&set_network_info);
			conducted_by_commissioning = false;
			if (ret != S_OK)
				log_err("Manually join network failed: %s",
					error_msg(ret));
			else {
				showln("Device discovering ...");
				ret = zb->device_discover();
				if (ret != S_OK)
					log_err("Device discover failed: %s",
						error_msg(ret));
			}

			step = 1;
		}
		break;

	}

	artik_release_api_module(zb);
}

static artik_error _func_endpoint_add(artik_zigbee_module *zb,
				ARTIK_ZIGBEE_PROFILE profile,
				ARTIK_ZIGBEE_DEVICEID device_id,
				int endpoint_id)
{
	struct test_device *test_device;
	artik_error result = S_OK;

	test_device = add_test_device(profile, device_id, endpoint_id);
	if (test_device == NULL) {
		log_err("Create test device failed");
		return E_NO_MEM;
	}

	if (profile == ARTIK_ZIGBEE_PROFILE_HA) {
		switch (device_id) {
		case ARTIK_ZIGBEE_DEVICE_ON_OFF_SWITCH:
			test_device->identify_request =
				device_on_off_switch_func.\
				identify_request;
			test_device->identify_get_remaining_time =
				device_on_off_switch_func.\
				identify_get_remaining_time;
			test_device->onoff_command =
				device_on_off_switch_func.onoff_command;
			test_device->ezmode_commissioning_initiator_start =
				device_on_off_switch_func.\
				ezmode_commissioning_initiator_start;
			test_device->ezmode_commissioning_initiator_stop =
				device_on_off_switch_func.\
				ezmode_commissioning_initiator_stop;
			break;

		case ARTIK_ZIGBEE_DEVICE_LEVEL_CONTROL_SWITCH:
			test_device->identify_request =
				device_level_control_switch_func.\
				identify_request;
			test_device->identify_get_remaining_time =
				device_level_control_switch_func.\
				identify_get_remaining_time;
			test_device->onoff_command =
				device_level_control_switch_func.onoff_command;
			test_device->level_control_request =
				device_level_control_switch_func.\
				level_control_request;
			test_device->ezmode_commissioning_initiator_start =
				device_level_control_switch_func.\
				ezmode_commissioning_initiator_start;
			test_device->ezmode_commissioning_initiator_stop =
				device_level_control_switch_func.\
				ezmode_commissioning_initiator_stop;
			break;

		case ARTIK_ZIGBEE_DEVICE_ON_OFF_LIGHT:
			test_device->groups_get_local_name_support =
				device_on_off_light_func.\
				groups_get_local_name_support;
			test_device->groups_set_local_name_support =
				device_on_off_light_func.\
				groups_set_local_name_support;
			test_device->onoff_get_value =
				device_on_off_light_func.onoff_get_value;
			test_device->ezmode_commissioning_target_start =
				device_on_off_light_func.\
				ezmode_commissioning_target_start;
			test_device->ezmode_commissioning_target_stop =
				device_on_off_light_func.\
				ezmode_commissioning_target_stop;
			break;

		case ARTIK_ZIGBEE_DEVICE_DIMMABLE_LIGHT:
			test_device->groups_get_local_name_support =
				device_dimmable_light_func.\
				groups_get_local_name_support;
			test_device->groups_set_local_name_support =
				device_dimmable_light_func.\
				groups_set_local_name_support;
			test_device->onoff_get_value =
				device_dimmable_light_func.onoff_get_value;
			test_device->level_control_get_value =
				device_dimmable_light_func.\
				level_control_get_value;
			test_device->ezmode_commissioning_target_start =
				device_dimmable_light_func.\
				ezmode_commissioning_target_start;
			test_device->ezmode_commissioning_target_stop =
				device_dimmable_light_func.\
				ezmode_commissioning_target_stop;
			break;

		case ARTIK_ZIGBEE_DEVICE_LIGHT_SENSOR:
			test_device->identify_request =
				device_light_sensor_func.identify_request;
			test_device->identify_get_remaining_time =
				device_light_sensor_func.\
				identify_get_remaining_time;
			test_device->illum_set_measured_value =
				device_light_sensor_func.\
				illum_set_measured_value;
			test_device->illum_get_measured_value =
				device_light_sensor_func.\
				illum_get_measured_value;
			test_device->illum_set_measured_value_range =
				device_light_sensor_func.\
				illum_set_measured_value_range;
			test_device->ezmode_commissioning_initiator_start =
				device_light_sensor_func.\
				ezmode_commissioning_initiator_start;
			test_device->ezmode_commissioning_initiator_stop =
				device_light_sensor_func.\
				ezmode_commissioning_initiator_stop;
			break;


		case ARTIK_ZIGBEE_DEVICE_REMOTE_CONTROL:
			test_device->reset_to_factory_default =
				device_remote_control_func.\
				reset_to_factory_default;
			test_device->identify_request =
				device_remote_control_func.identify_request;
			test_device->identify_get_remaining_time =
				device_remote_control_func.\
				identify_get_remaining_time;
			test_device->onoff_command =
				device_remote_control_func.onoff_command;
			test_device->level_control_request =
				device_remote_control_func.\
				level_control_request;
			test_device->request_reporting =
				device_remote_control_func.request_reporting;
			test_device->stop_reporting =
				device_remote_control_func.stop_reporting;
			test_device->ezmode_commissioning_target_start =
				device_remote_control_func.\
				ezmode_commissioning_target_start;
			test_device->ezmode_commissioning_target_stop =
				device_remote_control_func.\
				ezmode_commissioning_target_stop;
			break;

		default:
			log_err("Unsupported device type: 0x%04X", device_id);
			result = E_BAD_ARGS;
			break;
		}
	} else {
		log_err("Unsupported profile: %d", profile);
		result = E_BAD_ARGS;
	}

	if (result != S_OK)
		delete_test_device(endpoint_id);

	return result;
}

static artik_error _func_endpoint_init(artik_zigbee_module *zb)
{
	artik_zigbee_endpoint_handle
				handle_list[ARTIK_ZIGBEE_MAX_ENDPOINT_SIZE];
	artik_zigbee_local_endpoint_info endpoint_info;
	int i = 0, size = 0;
	artik_error ret = E_ZIGBEE_ERROR;
	struct test_device *device = NULL;

	get_test_device_list(handle_list, ARTIK_ZIGBEE_MAX_ENDPOINT_SIZE,
									&size);
	if (size <= 0) {
		log_err("device list is NULL");
		return E_ZIGBEE_NO_DEVICE;
	}

	memset(&endpoint_info, 0, sizeof(artik_zigbee_local_endpoint_info));
	for (i = 0; i < size; i++) {
		device = get_test_device(i);
		if (device != NULL) {
			endpoint_info.endpoints[endpoint_info.count].profile =
								device->profile;
			endpoint_info.endpoints[endpoint_info.count].endpoint_id
							= device->endpoint_id;
			endpoint_info.endpoints[endpoint_info.count].device_id =
							device->device_id;
			endpoint_info.count++;
		}
	}
	ret = zb->set_local_endpoint(&endpoint_info);
	if (ret != S_OK)
		log_err("Set local endpoint error %d", ret);

	ret = zb->initialize(_on_callback, NULL);
	if (ret != S_OK)
		log_err("Initialize error %d", ret);
	return ret;
}

static void _func_endpoint_show(void)
{
	struct device_type *device_type;
	int i, count;

	count = _get_device_type_count();
	for (i = 0; i < count; i++) {
		device_type = _get_device_type(i);
		showln("%d: %s", i + 1, device_type->name);
	}
}

static void _func_endpoint(char *input, int max_size)
{
	static int step;
	static struct device_type *select_device_type;
	artik_zigbee_module *zb;
	artik_error ret;
	int n;

	if (input == NULL)
		step = 0;

	zb = (artik_zigbee_module *)artik_request_api_module("zigbee");
	switch (step) {
	case 0:
		show_hyphen();
		_func_endpoint_show();
		showln("0: ADD DEVICE DONE");
		show("Add device: ");
		step = 1;
		break;

	case 1:	/* add device */
		n = read_int(input, max_size, -1);
		if (n == 0) {
			if (get_test_device_count() == 0) {
				show("\nAt least 1 endpoint,"\
					" please input again: ");
			} else {
				if (_func_endpoint_init(zb) == S_OK)
					_func_entry(_func_network);
				else {
					artik_release_api_module(zb);
					exit_loop();
					return;
				}
			}
		} else if (n >= 1 && n <= _get_device_type_count()) {
			select_device_type = _get_device_type(n - 1);
			show("Set endpoint id (%d): ",
				 select_device_type->default_endpoint_id);
			step = 2;
		} else {
			show_retry();
		}
		break;

	case 2:	/* set endpoint id */
		if (read_lf(input, max_size))
			n = select_device_type->default_endpoint_id;
		else
			n = read_int(input, max_size, -1);

		if (n >= ENDPOINT_ID_MIN && n <= ENDPOINT_ID_MAX) {
			if (check_test_device_endpoint_id(n) == true) {
				ret = _func_endpoint_add(zb,
				select_device_type->profile,
				select_device_type->device_id,
				n);
				if (ret == S_OK)
					show("Added device \"%s\""\
						" with ep(%d)\n\n",
						 select_device_type->name, n);
				else
					showln("Add endpoint failed: %s",
						error_msg(ret));

				show("Add device: ");
				step = 1;
			} else {
				show("\nRepeated endpoint id,"\
					" please input again: ");
			}
		} else {
			show_range(ENDPOINT_ID_MIN, ENDPOINT_ID_MAX);
		}
		break;
	}

	artik_release_api_module(zb);
}

static void _func_entrance(char *input, int max_size)
{
	static int step;
	static artik_zigbee_local_endpoint_info endpoint_info;
	artik_zigbee_local_endpoint local_ep;
	artik_zigbee_module *zb;
	artik_error ret = S_OK;
	int i = 0, n = 0;

	if (input == NULL)
		step = 0;

	zb = (artik_zigbee_module *) artik_request_api_module("zigbee");
	switch (step) {
	case 0:
		memset(&endpoint_info, 0,
				sizeof(artik_zigbee_local_endpoint_info));
		ret = zb->get_local_endpoint(&endpoint_info);
		if (ret == S_OK && endpoint_info.count > 0) {
			showln("Found %d saved local endpoints",
							endpoint_info.count);
			for (i = 0; i < endpoint_info.count; i++) {
				local_ep = endpoint_info.endpoints[i];
				showln("Profile 0x%04X endpoint %d"\
					" device id 0x%04X", local_ep.profile,
						local_ep.endpoint_id,
						local_ep.device_id);
			}
			show_hyphen();
			showln("0: Resume device");
			showln("1: Add new device");
			show_select();
			step = 1;
		} else
			_func_entry(_func_endpoint);
		break;

	case 1: /* Select to resume device or add new device */
		n = read_int(input, max_size, -1);
		if (n == 0) {
			show_hyphen();
			showln("Resume device");
			for (i = 0; i < endpoint_info.count; i++) {
				local_ep = endpoint_info.endpoints[i];
				ret = _func_endpoint_add(zb, local_ep.profile,
						local_ep.device_id,
						local_ep.endpoint_id);
				if (ret == S_OK)
					showln("Added endpoint %d",
							local_ep.endpoint_id);
				else
					showln("Add endpoint %d failed: %s",
						local_ep.endpoint_id,
						error_msg(ret));
			}

			ret = _func_endpoint_init(zb);
			if (ret != S_OK) {
				artik_release_api_module(zb);
				exit_loop();
				return;
			}

			_func_entry(_func_network);
		} else if (n == 1)
			_func_entry(_func_endpoint);
		else
			show_retry();
		break;
	}

	artik_release_api_module(zb);
}

void _on_callback_attr_changed(artik_zigbee_attribute_changed_response *info)
{
	struct test_device *device;
	artik_zigbee_onoff_status status;
	artik_error ret;
	int n;

	device = get_test_device_by_endpoint_id(info->endpoint_id);
	if (device == NULL)
		log_err("Invalid endpoint id: %d", info->endpoint_id);

	switch (info->type) {
	case ARTIK_ZIGBEE_ATTR_ONOFF_STATUS:
		if (device->onoff_get_value != NULL) {
			ret = device->onoff_get_value(device->handle, &status);
			if (ret == S_OK) {
				if (status == ARTIK_ZIGBEE_ONOFF_ON)
					showln("Attribute ONOFF changed"\
						" to ON by ep(%d)",
						   info->endpoint_id);
				else if (status == ARTIK_ZIGBEE_ONOFF_OFF)
					showln("Attribute ONOFF changed"\
						" to OFF by ep(%d)",
						   info->endpoint_id);
				else
					showln("Attribute ONOFF changed to"\
						" unrecognized value:%d"\
						" by ep(%d)",
						   status, info->endpoint_id);
			} else
				log_err("Attribute ONOFF changed by ep(%d),"\
					" get value failed: %s",
					info->endpoint_id, error_msg(ret));
		} else
			log_err("Not support onoff_get_value by ep(%d)",
							info->endpoint_id);
		break;

	case ARTIK_ZIGBEE_ATTR_LEVELCONTROL_LEVEL:
		if (device->level_control_get_value != NULL) {
			ret = device->level_control_get_value(device->handle,
									&n);
			if (ret == S_OK)
				showln("Attribute LEVEL changed to %d by"\
					" ep(%d)",
					   n, info->endpoint_id);
			else
				log_err("Attribute LEVEL changed by ep(%d),"\
					" get value failed: %s",
					info->endpoint_id, error_msg(ret));
		} else
			log_err("Not support level_control_get_value by ep(%d)",
					info->endpoint_id);
		break;

	default:
		showln("Attribute(%d) changed by endpoint(%d)", info->type,
			   info->endpoint_id);
		break;
	}
}

static void _on_level_control_command_callback(
			artik_zigbee_level_control_command *level_control)
{
	showln("Level control command callback type %d",
			level_control->control_type);

	switch (level_control->control_type) {
	case ARTIK_ZIGBEE_MOVE_TO_LEVEL:
		showln("Move to level %d transition time %d",
			level_control->parameters.move_to_level.level,
			level_control->parameters.move_to_level.\
			transition_time);
		break;

	case ARTIK_ZIGBEE_MOVE:
		showln("Move mode %d rate %d",
			level_control->parameters.move.control_mode,
			level_control->parameters.move.rate);
		break;

	case ARTIK_ZIGBEE_STEP:
		showln("Step mode %d step size %d transition time %d",
			level_control->parameters.step.control_mode,
			level_control->parameters.step.step_size,
			level_control->parameters.step.transition_time);
		break;

	case ARTIK_ZIGBEE_STOP:
		showln("Stop");
		break;

	case ARTIK_ZIGBEE_MOVE_TO_LEVEL_ONOFF:
		showln("On/off move to level %d transition time %d",
			level_control->parameters.move_to_level.level,
			level_control->parameters.move_to_level.\
			transition_time);
		break;

	case ARTIK_ZIGBEE_MOVE_ONOFF:
		showln("On/off move mode %d rate %d",
			level_control->parameters.move.control_mode,
			level_control->parameters.move.rate);
		break;

	case ARTIK_ZIGBEE_STEP_ONOFF:
		showln("On/off step mode %d step size %d transition time %d",
			level_control->parameters.step.control_mode,
			level_control->parameters.step.step_size,
			level_control->parameters.step.transition_time);
		break;

	case ARTIK_ZIGBEE_STOP_ONOFF:
		showln("On/off stop");
		break;
	}
}

static void _on_callback(void *user_data,
			artik_zigbee_response_type response_type,
			void *payload)
{
	artik_zigbee_network_find_result *net_find;
	artik_zigbee_groups_info *group_info;
	artik_zigbee_reporting_info *reporting_info;
	artik_zigbee_received_command *received_command;
	artik_zigbee_identify_feedback_info *identify_info;
	artik_zigbee_report_attribute_info *report_attr_info;
	artik_zigbee_commissioning_target_info *target_info;
	artik_zigbee_commissioning_bound_info *bound_info;
	struct test_device *test_device;
	artik_zigbee_device_discovery *device_discovery;
	artik_zigbee_match_desc_response *match_desc;
	artik_error ret;
	artik_zigbee_notification notification;
	artik_zigbee_network_notification network_notification;
	artik_zigbee_commissioning_state commissioning_state;
	artik_zigbee_module *zb;
	int i;

	log_dbg("In callback, response type : %d", response_type);

	zb = (artik_zigbee_module *)artik_request_api_module("zigbee");
	switch (response_type) {
	case ARTIK_ZIGBEE_RESPONSE_IEEE_ADDR_RESP:
		memcpy(&addr_rsp, payload,
				sizeof(artik_zigbee_ieee_addr_response));

		if (addr_rsp.result == ARTIK_ZIGBEE_SERVICE_DISCOVERY_DONE) {
			show("Ieee address response node id 0x%04x, eui64 ",
					addr_rsp.node_id);
			show("%02x", addr_rsp.eui64[0]);
			for (i = 1; i < ARTIK_ZIGBEE_EUI64_SIZE; i++)
				show(":%02x", addr_rsp.eui64[i]);
			showln("");
		} else
			showln("Ieee address response fail for result %d",
							addr_rsp.result);
		break;
	case ARTIK_ZIGBEE_RESPONSE_SIMPLE_DESC_RESP:
		memcpy(&simple_descriptor, payload,
			sizeof(artik_zigbee_simple_descriptor_response));

		if (simple_descriptor.result ==
					ARTIK_ZIGBEE_SERVICE_DISCOVERY_DONE) {
			showln("Simple descriptor response target node id"\
				" 0x%04x, target endpoint %d",
				simple_descriptor.target_node_id,
				simple_descriptor.target_endpoint);
			show("Client cluster count %d",
				simple_descriptor.client_cluster_count);
			for (i = 0; i < simple_descriptor.client_cluster_count;
									i++)
				show(" %d:0x%04x", i,
					simple_descriptor.client_cluster[i]);
			showln("");
			show("Server cluster count %d",
				simple_descriptor.server_cluster_count);
			for (i = 0; i < simple_descriptor.server_cluster_count;
									i++)
				show(" %d:0x%04x", i,
					simple_descriptor.server_cluster[i]);
			showln("");
		} else
			showln("Simple descriptor response fail for result %d",
					simple_descriptor.result);
		break;
	case ARTIK_ZIGBEE_RESPONSE_MATCH_DESC_RESP:
		match_desc = (artik_zigbee_match_desc_response *) payload;
		if (match_desc->result ==
				ARTIK_ZIGBEE_SERVICE_DISCOVERY_RECEIVED) {
			showln("Match description received"\
						" node id 0x%04x, eui64 ",
				match_desc->node_id, match_desc->count);
			for (i = 0; i < match_desc->count; i++)
				showln("endpoint %d",
					match_desc->endpoint_list[i]);
		} else if (match_desc->result ==
					ARTIK_ZIGBEE_SERVICE_DISCOVERY_DONE)
			showln("match description response complete");
		else
			showln("match description response fail for result %d",
					match_desc->result);
		break;
	case ARTIK_ZIGBEE_RESPONSE_NOTIFICATION:
		notification = *((artik_zigbee_notification *) payload);
		switch (notification) {
		case ARTIK_ZIGBEE_CMD_SUCCESS:
			showln("NOTIFICATION: SUCCESS");
			break;

		case ARTIK_ZIGBEE_CMD_ERR_PORT_PROBLEM:
		case ARTIK_ZIGBEE_CMD_ERR_NO_SUCH_COMMAND:
		case ARTIK_ZIGBEE_CMD_ERR_WRONG_NUMBER_OF_ARGUMENTS:
		case ARTIK_ZIGBEE_CMD_ERR_ARGUMENT_OUT_OF_RANGE:
		case ARTIK_ZIGBEE_CMD_ERR_ARGUMENT_SYNTAX_ERROR:
		case ARTIK_ZIGBEE_CMD_ERR_STRING_TOO_LONG:
		case ARTIK_ZIGBEE_CMD_ERR_INVALID_ARGUMENT_TYPE:
		case ARTIK_ZIGBEE_CMD_ERR:
			showln("NOTIFICATION: ERROR(%d)", notification);
			break;

		default:
			showln("NOTIFICATION: ERROR(%d)", notification);
			break;
		}
		break;

	case ARTIK_ZIGBEE_RESPONSE_NETWORK_NOTIFICATION:
		network_notification = *((artik_zigbee_network_notification *)
								payload);
		switch (network_notification) {
		case ARTIK_ZIGBEE_NETWORK_JOIN:
			showln("NETWORK_NOTIFICATION: JOIN");
			if (!conducted_by_commissioning) {
				showln("Device discovering ...");
				ret = zb->device_discover();
				if (ret != S_OK)
					log_err("Device discover failed:%s",
							error_msg(ret));
			}
			break;

		case ARTIK_ZIGBEE_NETWORK_LEAVE:
			showln("NETWORK_NOTIFICATION: LEAVE");
			break;

		case ARTIK_ZIGBEE_NETWORK_EXIST:
			showln("NETWORK_NOTIFICATION: Network exist,"\
				" please leave current network and try again");
			break;

		case ARTIK_ZIGBEE_NETWORK_FIND_FORM_SUCCESS:
			showln("NETWORK_NOTIFICATION: FIND FORM SUCCESS");
			ret = zb->network_permitjoin(EASY_PJOIN_DURATION);
			if (ret != S_OK)
				log_err("zigbee_network_permitjoin failed:%s",
								error_msg(ret));
			break;

		case ARTIK_ZIGBEE_NETWORK_FIND_FORM_FAILED:
			showln("NETWORK_NOTIFICATION: FIND FORM FAILED");
			break;

		case ARTIK_ZIGBEE_NETWORK_FIND_JOIN_SUCCESS:
			showln("NETWORK_NOTIFICATION: FIND JOIN SUCCESS");
			if (!conducted_by_commissioning) {
				showln("Device discovering ...");
				ret = zb->device_discover();
				if (ret != S_OK)
					log_err("Device discover failed:%s",
								error_msg(ret));
			}
			break;

		case ARTIK_ZIGBEE_NETWORK_FIND_JOIN_FAILED:
			showln("NETWORK_NOTIFICATION: FIND JOIN FAILED");
			break;

		default:
			showln("NETWORK_NOTIFICATION: %d",
							network_notification);
			break;
		}
		break;

	case ARTIK_ZIGBEE_RESPONSE_NETWORK_FIND:
		net_find = (artik_zigbee_network_find_result *)payload;
		switch (net_find->find_status) {
		case ARTIK_ZIGBEE_NETWORK_FOUND:
			if (nwk_find_size < NWK_FIND_MAX_SIZE) {
				memcpy(&nwk_found_list[nwk_find_size], net_find,
					sizeof(
					artik_zigbee_network_find_result));
				nwk_find_size++;

				showln("%d: network channel(%d),"\
					" tx power(%d), pan id(0x%04X)",
					nwk_find_size,
					net_find->network_info.channel,
					net_find->network_info.tx_power,
					net_find->network_info.pan_id);
			} else
				showln("Network found out of max size %d",
					NWK_FIND_MAX_SIZE);
			break;

		case ARTIK_ZIGBEE_NETWORK_FIND_FINISHED:
			showln("Network find finished");
			if (current_func == _func_network_find_join)
				_func_network_found_end_show();
			break;

		case ARTIK_ZIGBEE_NETWORK_FIND_ERR:
			showln("Network find error");
			if (current_func == _func_network_find_join) {
				_func_network_found_show();
				_func_network_found_end_show();
			}
			break;

		default:
			showln("Network find unknown status");
			break;
		}
		break;

	case ARTIK_ZIGBEE_RESPONSE_DEVICE_DISCOVER:
		device_discovery = (artik_zigbee_device_discovery *) payload;
		switch (device_discovery->status) {
		case ARTIK_ZIGBEE_DEVICE_DISCOVERY_START:
			showln("Device discovery start");
			break;
		case ARTIK_ZIGBEE_DEVICE_DISCOVERY_FOUND:
			showln("Device discovery found:");
			show_device(&(device_discovery->device));
			break;
		case ARTIK_ZIGBEE_DEVICE_DISCOVERY_IN_PROGRESS:
			showln("Device discovery in progress");
			showln("Done");
			break;
		case ARTIK_ZIGBEE_DEVICE_DISCOVERY_DONE:
			showln("Done");
			break;
		case ARTIK_ZIGBEE_DEVICE_DISCOVERY_NO_DEVICE:
			showln("Device discovery no device");
			showln("Done");
			break;
		case ARTIK_ZIGBEE_DEVICE_DISCOVERY_ERROR:
			showln("Device discovery error");
			break;
		case ARTIK_ZIGBEE_DEVICE_DISCOVERY_CHANGED:
			showln("Device discovery changed:");
			show_device(&(device_discovery->device));
			break;
		case ARTIK_ZIGBEE_DEVICE_DISCOVERY_LOST:
			showln("Device discovery lost:");
			show_device(&(device_discovery->device));
			break;
		default:
			showln("Discovery device unknown result");
			showln("Done");
			break;
		}
		break;

	case ARTIK_ZIGBEE_RESPONSE_GROUPS_INFO:
		group_info = (artik_zigbee_groups_info *)payload;
		showln("GROUPS: group ID(%d) cmd(%d)",
			   group_info->group_id, group_info->group_cmd);

		switch (group_info->group_cmd) {
		case ARTIK_ZIGBEE_GROUPS_ADD_IF_IDENTIFYING:
			showln("ARTIK_ZIGBEE_GROUPS_ADD_IF_IDENTIFYING");
			break;
		case ARTIK_ZIGBEE_GROUPS_ADD:
			showln("ARTIK_ZIGBEE_GROUPS_ADD");
			break;
		case ARTIK_ZIGBEE_GROUPS_REMOVE:
			showln("ARTIK_ZIGBEE_GROUPS_REMOVE");
			break;
		case ARTIK_ZIGBEE_GROUPS_REMOVE_ALL:
			showln("ARTIK_ZIGBEE_GROUPS_REMOVE_ALL");
			break;
		default:
			showln("Groups command not support");
		}
		test_device = get_test_device_by_endpoint_id(
						group_info->endpoint_id);
		if (test_device == NULL) {
			log_err("Not found endpoint id:%d",
						group_info->endpoint_id);
			break;
		}

		ret = test_device->groups_get_local_name_support(
						test_device->handle,
						group_info->endpoint_id);
		if (ret == S_OK) {
			showln("ARTIK_ZIGBEE_GROUPS: name supported by ep(%d)",
					 group_info->endpoint_id);
			ret = test_device->groups_set_local_name_support(
						test_device->handle, false);
			if (ret != S_OK) {
				showln("Setting local groups name support"\
					" attribute value to FALSE failed: %s",
					error_msg(ret));
			} else
				showln("Setting local groups name support"\
					" attribute value to FALSE succeeded");
		} else if (ret == E_NOT_SUPPORTED) {
			showln("ARTIK_ZIGBEE_GROUPS: name not supported"\
					" by ep(%d)",
					group_info->endpoint_id);
			ret = test_device->groups_set_local_name_support(
						test_device->handle, true);
			if (ret != S_OK) {
				showln("Setting local groups name support"\
					" attribute value to TRUE failed: %s",
					error_msg(ret));
			} else
				showln("Setting local groups name support"\
					" attribute value to TRUE succeeded");
		} else
			showln("ARTIK_ZIGBEE_GROUPS: get name support"\
					" failed: %s",
					error_msg(ret));
		break;

	case ARTIK_ZIGBEE_RESPONSE_ATTRIBUTE_CHANGE:
		_on_callback_attr_changed((
			artik_zigbee_attribute_changed_response *)payload);
		break;

	case ARTIK_ZIGBEE_RESPONSE_CLIENT_TO_SERVER_COMMAND_RECEIVED:
		received_command = (artik_zigbee_received_command *)payload;
		if (received_command->is_global_command)
			showln("receive global command:0x%02X endpoint:%d"\
				" cluster:0x%04X",
				received_command->command_id,
				received_command->dest_endpoint_id,
				received_command->cluster_id);
		else if (received_command->dest_endpoint_id ==
						ARTIK_ZIGBEE_BROADCAST_ENDPOINT)
			showln("receive broadcast command cluster:0x%04X"\
				" command:0x%02X",
				received_command->cluster_id,
				received_command->command_id);
		else
			showln("receive command endpoint:%d cluster:0x%04X"\
				" command:0x%02X",
				received_command->dest_endpoint_id,
				received_command->cluster_id,
				received_command->command_id);
		if (received_command->payload_length > 0)
			showln("receive command:0x%02X payload length is %d",
				received_command->command_id,
				received_command->payload_length);
		else if (received_command->payload_length == 0)
			showln("receive command:0x%02X payload is empty",
				received_command->command_id);
		else if (received_command->payload_length == -1)
			showln("receive command:0x%02X payload length over:%d",
				received_command->command_id,
			ARTIK_ZIGBEE_MAX_RECEIVED_COMMAND_PLAYLOAD_LENGTH);
		else
			showln("receive command:0x%02X unrecognized payload"\
				" length is %d",
				received_command->command_id,
				received_command->payload_length);
		break;

	case ARTIK_ZIGBEE_RESPONSE_REPORTING_CONFIGURE:
		reporting_info = (artik_zigbee_reporting_info *)payload;
		if (reporting_info->used == true)
			showln("get reporting configure ep:%d cluster:%d"\
				" attr:%d min_interval:%d max_interval:%d"\
				" reportable_change:%d",
				reporting_info->endpoint_id,
				reporting_info->cluster_id,
				reporting_info->attribute_id,
				reporting_info->reported.min_interval,
				reporting_info->reported.max_interval,
				reporting_info->reported.reportable_change);
		else
			showln("removed reporting configure cluster:%d"\
				" attr:%d min_interval:%d max_interval:%d"\
				" reportable_change:%d",
				reporting_info->cluster_id,
				reporting_info->attribute_id,
				reporting_info->reported.min_interval,
				reporting_info->reported.max_interval,
				reporting_info->reported.reportable_change);
		break;

	case ARTIK_ZIGBEE_RESPONSE_REPORT_ATTRIBUTE:
		report_attr_info = (artik_zigbee_report_attribute_info *)
								payload;
		switch (report_attr_info->attribute_type) {
		case ARTIK_ZIGBEE_ATTR_ILLUMINANCE:
			showln("Report attribute: Illuminance: %d",
				   report_attr_info->data.value);
			reporting_measured_illum_count++;
			if (reporting_measured_illum_count >
							REPORTING_MAX_COUNT) {
				_stop_reporting(
				ARTIK_ZIGBEE_REPORTING_MEASURED_ILLUMINANCE);
				reporting_measured_illum_count = 0;
			}
			break;
		default:
			log_err("Not supported attribute");
			break;
		}
		break;

	case ARTIK_ZIGBEE_RESPONSE_IDENTIFY_FEEDBACK_START:
		identify_info = (artik_zigbee_identify_feedback_info *)payload;
		showln("Identify feedback start with %d seconds by ep(%d)",
			   identify_info->duration, identify_info->endpoint_id);
		break;

	case ARTIK_ZIGBEE_RESPONSE_IDENTIFY_FEEDBACK_STOP:
		identify_info = (artik_zigbee_identify_feedback_info *)payload;
		showln("Identify feedback stop by ep(%d)",
						identify_info->endpoint_id);
		break;

	case ARTIK_ZIGBEE_RESPONSE_COMMISSIONING_STATUS:
		commissioning_state = *((artik_zigbee_commissioning_state *)
								payload);
		switch (commissioning_state) {
		case ARTIK_ZIGBEE_COMMISSIONING_ERROR:
			showln("Commissioning: error");
			showln("Done");
			break;
		case ARTIK_ZIGBEE_COMMISSIONING_ERR_IN_PROGRESS:
			showln("Commissioning: error in progress");
			showln("Done");
			break;
		case ARTIK_ZIGBEE_COMMISSIONING_NETWORK_STEERING_FORM:
			showln("Commissioning: network steering form");
			break;
		case ARTIK_ZIGBEE_COMMISSIONING_NETWORK_STEERING_SUCCESS:
			showln("Commissioning: network steering success");
			break;
		case ARTIK_ZIGBEE_COMMISSIONING_NETWORK_STEERING_FAILED:
			showln("Commissioning: network steering failed");
			break;
		case ARTIK_ZIGBEE_COMMISSIONING_WAIT_NETWORK_STEERING:
			showln("Commissioning: wait for network steering");
			break;
		case ARTIK_ZIGBEE_COMMISSIONING_INITIATOR_SUCCESS:
			_commissioning_initiator_run(zb, CS_INITIATOR_SUCCESS);
			break;
		case ARTIK_ZIGBEE_COMMISSIONING_INITIATOR_FAILED:
			_commissioning_initiator_run(zb, CS_INITIATOR_FAILED);
			break;
		case ARTIK_ZIGBEE_COMMISSIONING_INITIATOR_STOP:
			showln("Commissioning: initiator stopped");
			showln("Done");
			break;
		case ARTIK_ZIGBEE_COMMISSIONING_TARGET_SUCCESS:
			_commissioning_target_run(zb, CS_TARGET_SUCCESS);
			break;
		case ARTIK_ZIGBEE_COMMISSIONING_TARGET_FAILED:
			_commissioning_target_run(zb, CS_TARGET_FAILED);
			break;
		case ARTIK_ZIGBEE_COMMISSIONING_TARGET_STOP:
			showln("Commissioning: target stopped");
			showln("Done");
			break;
		}
		break;

	case ARTIK_ZIGBEE_RESPONSE_COMMISSIONING_TARGET_INFO:
		target_info = (artik_zigbee_commissioning_target_info *)payload;
		showln("Commissioning: found target id:0x%04X, endpoint:%d",
			target_info->node_id, target_info->endpoint_id);
		break;

	case ARTIK_ZIGBEE_RESPONSE_COMMISSIONING_BOUND_INFO:
		bound_info = (artik_zigbee_commissioning_bound_info *)payload;
		showln("Commissioning: bound target endpoint:%d,"\
			" clusterId:0x%04X",
			bound_info->endpoint_id, bound_info->cluster_id);
		break;

	case ARTIK_ZIGBEE_RESPONSE_BASIC_RESET_TO_FACTORY:
		showln("Basic reset to factory defauts by endpoint %d",
							*((int *) payload));
		break;

	case ARTIK_ZIGBEE_RESPONSE_LEVEL_CONTROL:
		_on_level_control_command_callback((
				artik_zigbee_level_control_command *) payload);
		break;

	default:
		break;
	}

	log_dbg("[testzigbee] callback end\n");

	artik_release_api_module(zb);
}

static void _on_timer_callback(enum timer_command cmd, int id, void *user_data)
{
	artik_zigbee_module *zb;

	zb = (artik_zigbee_module *)artik_request_api_module("zigbee");

	switch (cmd) {
	case TIMER_CS_TARGET_TIMEOUT:
		if (cs_target_timeout_id == id)
			_commissioning_target_run(zb, CS_TARGET_TIMEOUT);
		break;
	default:
		log_err("Unexpected timer callback:%d", cmd);
		break;
	}

	if (user_data != NULL)
		free(user_data);

	artik_release_api_module(zb);
}

static int _on_keyboard_received(int fd, enum watch_io io, void *user_data)
{
	char input[KEYBOARD_INPUT_SIZE];

	if (fd != STDIN_FILENO)
		log_err("Wrong fd: %d", fd);

	if (io != WATCH_IO_IN && io != WATCH_IO_ERR && io != WATCH_IO_HUP &&
		io != WATCH_IO_NVAL)
		log_err("Wrong io: %d", io);

	if (user_data)
		log_err("Wrong user data: %p", user_data);

	if (fgets(input, KEYBOARD_INPUT_SIZE, stdin) == NULL)
		return 1;

	current_func(input, KEYBOARD_INPUT_SIZE);

	return 1;
}


int main(int argc, char *argv[])
{
	artik_zigbee_module *zb;
	artik_loop_module *loop;
	enum watch_io io;

	if (!artik_is_module_available(ARTIK_MODULE_ZIGBEE)) {
		log_err("Module zigbee is not available");
		return -1;
	}

	zb = (artik_zigbee_module *)artik_request_api_module("zigbee");
	loop = (artik_loop_module *)artik_request_api_module("loop");

	_func_entry(_func_entrance);

	io = (WATCH_IO_IN | WATCH_IO_ERR | WATCH_IO_HUP | WATCH_IO_NVAL);
	loop->add_fd_watch(STDIN_FILENO, io, _on_keyboard_received, NULL, NULL);
	loop->run();

	release_all_test_devices(zb);
	artik_release_api_module(loop);
	artik_release_api_module(zb);

	return 0;
}
