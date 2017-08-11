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
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>

#include <artik_log.h>
#include <artik_loop.h>
#include <artik_zigbee.h>
#include <artik_module.h>
#include <artik_platform.h>

#define STR_ON_OFF_SWITCH		"ON_OFF_SWITCH"
#define STR_ON_OFF_LIGHT		"ON_OFF_LIGHT"
#define STR_DIMMABLE_LIGHT		"DIMMABLE_LIGHT"
#define STR_LEVEL_CONTROL_SWITCH	"LEVEL_CONTROL_SWITCH"
#define STR_COLOR_DIMMABLE_LIGHT	"COLOR_DIMMABLE_LIGHT"
#define STR_ON_OFF_LIGHT_SWITCH		"ON_OFF_LIGHT_SWITCH"
#define STR_DIMMER_SWITCH		"DIMMER_SWITCH"
#define STR_COLOR_DIMMER_SWITCH		"COLOR_DIMMER_SWITCH"
#define STR_LIGHT_SENSOR		"LIGHT_SENSOR"
#define STR_OCCUPANCY_SENSOR		"OCCUPANCY_SENSOR"
#define STR_HEATING_COOLING_UNIT	"HEATING_COOLING_UNIT"
#define STR_THERMOSTAT			"THERMOSTAT"
#define STR_TEMPERATURE_SENSOR		"TEMPERATURE_SENSOR"
#define STR_REMOTE_CONTROL		"REMOTE_CONTROL"

#define KEYBOARD_INPUT_SIZE		100

void _callback(void *user_data, artik_zigbee_response_type response_type,
		void *payload)
{
	artik_zigbee_notification notification;
	artik_zigbee_network_notification network_notification;

	log_dbg("In callback, response type : %d", response_type);

	switch (response_type) {
	case ARTIK_ZIGBEE_RESPONSE_NOTIFICATION:
		notification = *((artik_zigbee_notification *) payload);
		switch (notification) {
		case ARTIK_ZIGBEE_CMD_SUCCESS:
			log_info("In callback, ARTIK_ZIGBEE_CMD_SUCCESS");
			break;
		case ARTIK_ZIGBEE_CMD_ERR_PORT_PROBLEM:
		case ARTIK_ZIGBEE_CMD_ERR_NO_SUCH_COMMAND:
		case ARTIK_ZIGBEE_CMD_ERR_WRONG_NUMBER_OF_ARGUMENTS:
		case ARTIK_ZIGBEE_CMD_ERR_ARGUMENT_OUT_OF_RANGE:
		case ARTIK_ZIGBEE_CMD_ERR_ARGUMENT_SYNTAX_ERROR:
		case ARTIK_ZIGBEE_CMD_ERR_STRING_TOO_LONG:
		case ARTIK_ZIGBEE_CMD_ERR_INVALID_ARGUMENT_TYPE:
		case ARTIK_ZIGBEE_CMD_ERR:
			log_err("In callback, COMMAND ERROR(%d)!",
								notification);
			break;
		default:
			log_dbg("In callback, response %d", notification);
			break;
		}
		break;
	case ARTIK_ZIGBEE_RESPONSE_NETWORK_NOTIFICATION:
		network_notification = *((artik_zigbee_network_notification *)
								payload);
		switch (network_notification) {
		case ARTIK_ZIGBEE_NETWORK_JOIN:
			log_info("In callback, ARTIK_ZIGBEE_NETWORK_JOIN");
			break;
		case ARTIK_ZIGBEE_NETWORK_LEAVE:
			log_info("In callback, ARTIK_ZIGBEE_NETWORK_LEAVE");
			break;
		case ARTIK_ZIGBEE_NETWORK_FIND_JOIN_SUCCESS:
			log_info("In callback, ARTIK_ZIGBEE_NETWORK_"\
				"FIND_JOIN_SUCCESS");
			break;
		case ARTIK_ZIGBEE_NETWORK_FIND_JOIN_FAILED:
			log_warn("In callback, ARTIK_ZIGBEE_NETWORK_"\
				"FIND_JOIN_FAILED");
			break;
		default:
			log_dbg("In callback, response %d",
							network_notification);
			break;
		}
		break;
	default:
		break;
	}
	log_dbg("callback end");
}

static int _on_keyboard_received(int fd, enum watch_io io, void *user_data)
{
	char command[KEYBOARD_INPUT_SIZE];
	artik_zigbee_module *zb = (artik_zigbee_module *)
					artik_request_api_module("zigbee");

	assert(fd == STDIN_FILENO);
	assert(io == WATCH_IO_IN || io == WATCH_IO_ERR || io == WATCH_IO_HUP
						|| io == WATCH_IO_NVAL);
	assert(user_data == NULL);

	if (fgets(command, KEYBOARD_INPUT_SIZE, stdin) == NULL)
		return 1;
	if (strlen(command) > 1)
		zb->raw_request(command);

	artik_release_api_module(zb);
	return 1;
}

static artik_error _get_device_info(artik_zigbee_module *zb,
				const char *str_device_id,
				ARTIK_ZIGBEE_PROFILE *profile,
				int *endpoint_id,
				ARTIK_ZIGBEE_DEVICEID *device_id)
{
	if (NULL == profile || NULL == endpoint_id || NULL == device_id)
		return E_INVALID_VALUE;

	if (!strcmp(str_device_id, STR_ON_OFF_SWITCH)) {
		*endpoint_id = 1;
		*profile = ARTIK_ZIGBEE_PROFILE_HA;
		*device_id = ARTIK_ZIGBEE_DEVICE_ON_OFF_SWITCH;
	} else if (!strcmp(str_device_id, STR_ON_OFF_LIGHT)) {
		*endpoint_id = 19;
		*profile = ARTIK_ZIGBEE_PROFILE_HA;
		*device_id = ARTIK_ZIGBEE_DEVICE_ON_OFF_LIGHT;
	} else if (!strcmp(str_device_id, STR_DIMMABLE_LIGHT)) {
		*endpoint_id = 20;
		*profile = ARTIK_ZIGBEE_PROFILE_HA;
		*device_id = ARTIK_ZIGBEE_DEVICE_DIMMABLE_LIGHT;
	} else if (!strcmp(str_device_id, STR_LEVEL_CONTROL_SWITCH)) {
		*endpoint_id = 2;
		*profile = ARTIK_ZIGBEE_PROFILE_HA;
		*device_id = ARTIK_ZIGBEE_DEVICE_LEVEL_CONTROL_SWITCH;
	} else if (!strcmp(str_device_id, STR_COLOR_DIMMABLE_LIGHT)) {
		*endpoint_id = 21;
		*profile = ARTIK_ZIGBEE_PROFILE_HA;
		*device_id = ARTIK_ZIGBEE_DEVICE_COLOR_DIMMABLE_LIGHT;
	} else if (!strcmp(str_device_id, STR_ON_OFF_LIGHT_SWITCH)) {
		*endpoint_id = 22;
		*profile = ARTIK_ZIGBEE_PROFILE_HA;
		*device_id = ARTIK_ZIGBEE_DEVICE_ON_OFF_LIGHT_SWITCH;
	} else if (!strcmp(str_device_id, STR_DIMMER_SWITCH)) {
		*endpoint_id = 23;
		*profile = ARTIK_ZIGBEE_PROFILE_HA;
		*device_id = ARTIK_ZIGBEE_DEVICE_DIMMER_SWITCH;
	} else if (!strcmp(str_device_id, STR_COLOR_DIMMER_SWITCH)) {
		*endpoint_id = 24;
		*profile = ARTIK_ZIGBEE_PROFILE_HA;
		*device_id = ARTIK_ZIGBEE_DEVICE_COLOR_DIMMER_SWITCH;
	} else if (!strcmp(str_device_id, STR_LIGHT_SENSOR)) {
		*endpoint_id = 25;
		*profile = ARTIK_ZIGBEE_PROFILE_HA;
		*device_id = ARTIK_ZIGBEE_DEVICE_LIGHT_SENSOR;
	} else if (!strcmp(str_device_id, STR_OCCUPANCY_SENSOR)) {
		*endpoint_id = 26;
		*profile = ARTIK_ZIGBEE_PROFILE_HA;
		*device_id = ARTIK_ZIGBEE_DEVICE_OCCUPANCY_SENSOR;
	} else if (!strcmp(str_device_id, STR_HEATING_COOLING_UNIT)) {
		*endpoint_id = 31;
		*profile = ARTIK_ZIGBEE_PROFILE_HA;
		*device_id = ARTIK_ZIGBEE_DEVICE_HEATING_COOLING_UNIT;
	} else if (!strcmp(str_device_id, STR_THERMOSTAT)) {
		*endpoint_id = 32;
		*profile = ARTIK_ZIGBEE_PROFILE_HA;
		*device_id = ARTIK_ZIGBEE_DEVICE_THERMOSTAT;
	} else if (!strcmp(str_device_id, STR_TEMPERATURE_SENSOR)) {
		*endpoint_id = 33;
		*profile = ARTIK_ZIGBEE_PROFILE_HA;
		*device_id = ARTIK_ZIGBEE_DEVICE_TEMPERATURE_SENSOR;
	} else if (!strcmp(str_device_id, STR_REMOTE_CONTROL)) {
		*endpoint_id = 34;
		*profile = ARTIK_ZIGBEE_PROFILE_HA;
		*device_id = ARTIK_ZIGBEE_DEVICE_REMOTE_CONTROL;
	} else
		return E_BAD_ARGS;

	return S_OK;
}

static void _print_network_status(int network_state)
{
	switch (network_state) {
	case ARTIK_ZIGBEE_NO_NETWORK:
		log_info("state ARTIK_ZIGBEE_NO_NETWORK");
		break;
	case ARTIK_ZIGBEE_JOINING_NETWORK:
		log_info("state ARTIK_ZIGBEE_JOINING_NETWORK");
		break;
	case ARTIK_ZIGBEE_JOINED_NETWORK:
		log_info("state ARTIK_ZIGBEE_JOINED_NETWORK");
		break;
	case ARTIK_ZIGBEE_JOINED_NETWORK_NO_PARENT:
		log_info("state ARTIK_ZIGBEE_JOINED_NETWORK_NO_PARENT");
		break;
	case ARTIK_ZIGBEE_LEAVING_NETWORK:
		log_info("state ARTIK_ZIGBEE_LEAVING_NETWORK");
		break;
	default:
		break;
	}
}

static void _print_node_type(int node_type)
{
	switch (node_type) {
	case ARTIK_ZIGBEE_UNKNOWN_DEVICE:
		log_info("node type ARTIK_ZIGBEE_UNKNOWN_DEVICE");
		break;
	case ARTIK_ZIGBEE_COORDINATOR:
		log_info("node type ARTIK_ZIGBEE_COORDINATOR");
		break;
	case ARTIK_ZIGBEE_ROUTER:
		log_info("node type ARTIK_ZIGBEE_ROUTER");
		break;
	case ARTIK_ZIGBEE_END_DEVICE:
		log_info("node type ARTIK_ZIGBEE_END_DEVICE");
		break;
	case ARTIK_ZIGBEE_SLEEPY_END_DEVICE:
		log_info("node type ARTIK_ZIGBEE_SLEEPY_END_DEVICE");
		break;
	}
}

int main(int argc, char *argv[])
{
	artik_zigbee_local_endpoint_info endpoint_info;
	artik_error ret = S_OK;
	int i;
	artik_zigbee_network_state state;
	artik_zigbee_node_type type;

	artik_loop_module *loop = NULL;
	artik_zigbee_module *zb = NULL;

	if (!artik_is_module_available(ARTIK_MODULE_ZIGBEE)) {
		fprintf(stdout, "TEST: Zigbee module is not available,"\
			" skipping test...\n");
		return -1;
	}

	log_dbg("Start zigbee cli program");

	if (argc == 1) {
		log_err("Usage: zigbee_cli [DEVICE TYPE] ...");
		return 0;
	}

	zb = (artik_zigbee_module *)artik_request_api_module("zigbee");
	endpoint_info.count = 0;
	if (argc > 1) {
		if (argc > ARTIK_ZIGBEE_MAX_ENDPOINT_SIZE + 1)
			log_warn("only %d device type is supported currently!",
					ARTIK_ZIGBEE_MAX_ENDPOINT_SIZE);
		for (i = 1; i < argc && i < ARTIK_ZIGBEE_MAX_ENDPOINT_SIZE + 1;
									i++) {
			log_info("Device type is %s", argv[i]);
			ret = _get_device_info(zb, argv[i],
					&endpoint_info.endpoints[
					endpoint_info.count].profile,
					&endpoint_info.endpoints[
					endpoint_info.count].endpoint_id,
					&endpoint_info.endpoints[
					endpoint_info.count].device_id);
			if (ret != S_OK)
				log_warn("not supported device type!!");
			else
				endpoint_info.count++;
		}
	}

	loop = (artik_loop_module *)artik_request_api_module("loop");
	ret = zb->set_local_endpoint(&endpoint_info);
	if (ret != S_OK) {
		artik_release_api_module(loop);
		artik_release_api_module(zb);
		return -1;
	}

	ret = zb->initialize(_callback, NULL);
	if (ret != S_OK) {
		artik_release_api_module(loop);
		artik_release_api_module(zb);
		return -1;
	}

	if (zb->network_start(NULL) == ARTIK_ZIGBEE_JOINED_NETWORK) {
		ret = zb->network_request_my_network_status(&state);
		if (ret == S_OK)
			_print_network_status(state);
		else
			log_err("get network status failed: %s",
								error_msg(ret));
		ret = zb->device_request_my_node_type(&type);
		if (ret == S_OK)
			_print_node_type(type);
		else
			log_err("get device ndoe type failed: %s",
								error_msg(ret));
	} else
		log_dbg("Privious Network : Non Exist");

	loop->add_fd_watch(STDIN_FILENO, (WATCH_IO_IN | WATCH_IO_ERR |
					WATCH_IO_HUP | WATCH_IO_NVAL),
					_on_keyboard_received, NULL, NULL);

	loop->run();

	zb->deinitialize();
	artik_release_api_module(loop);
	artik_release_api_module(zb);

	log_dbg("Stop zigbee cli program");

	return (ret == S_OK) ? 0 : -1;
}
