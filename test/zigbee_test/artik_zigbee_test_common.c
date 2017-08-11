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

#include <stdarg.h>
#include <stdio.h>
#include <artik_log.h>
#include <artik_list.h>
#include <artik_loop.h>
#include <artik_module.h>
#include "artik_zigbee_test_common.h"

static artik_list *test_device_list;

static int _get_digit(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';

	return -1;
}

static int _get_hex(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';

	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;

	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;

	return -1;
}

static int _read_hex(char *input, int max_size)
{
	int i;
	int result;
	char c;

	result = 0;

	for (i = 0; i < max_size; i++) {
		c = input[i];

		if (_get_hex(c) < 0)
			break;

		result *= 16;
		result += _get_hex(c);
	}

	return result;
}

static int _read_digit(char *input, int max_size)
{
	int i;
	int result;
	char c;

	result = 0;

	for (i = 0; i < max_size; i++) {
		c = input[i];

		if (_get_digit(c) < 0)
			break;

		result *= 10;
		result += _get_digit(c);
	}

	return result;
}

int read_int(char *input, int max_size, int default_value)
{
	int i;
	char c;

	for (i = 0; i < max_size; i++) {
		c = input[i];

		if (c == 0 || c == '\n')
			break;

		if (c == '0' && (i + 3) < max_size &&
			(input[i + 1] == 'x' || input[i + 1] == 'X') &&
			_get_hex(input[i + 2]) >= 0 && _get_hex(input[i + 3])
									>= 0) {
			return _read_hex(&input[i + 2], max_size - (i + 2));
		}

		if (_get_digit(c) >= 0)
			return _read_digit(&input[i], max_size - i);
	}

	return default_value;
}

bool read_q(char *input, int max_size)
{
	if (input[0] == 'q' || input[0] == 'Q')
		return true;
	else
		return false;
}

bool read_e(char *input, int max_size)
{
	if (input[0] == 'e' || input[0] == 'E')
		return true;
	else
		return false;
}

bool read_lf(char *input, int max_size)
{
	if (input[0] == '\n')
		return true;
	else
		return false;
}

int read_input_int(int default_value)
{
	char input[KEYBOARD_INPUT_SIZE];

	if (fgets(input, KEYBOARD_INPUT_SIZE, stdin) == NULL)
		return 1;

	return read_int(input, KEYBOARD_INPUT_SIZE, default_value);
}

void show(const char *format, ...)
{
	va_list arg;

	va_start(arg, format);
	vfprintf(stdout, format, arg);
	va_end(arg);

	fflush(stdout);
}

void showln(const char *format, ...)
{
	va_list arg;

	va_start(arg, format);
	vfprintf(stdout, format, arg);
	va_end(arg);

	fprintf(stdout, "\n");
	fflush(stdout);
}

void show_hyphen(void)
{
	showln("======================");
}

void show_retry(void)
{
	show("\ninvalid input, please try again: ");
}

void show_range(int min, int max)
{
	show("\nRange is %d ~ %d, please input again: ", min, max);
}

void show_select(void)
{
	show("Please select operation: ");
}

void show_network_status(int status)
{
	switch (status) {
	case ARTIK_ZIGBEE_NO_NETWORK:
		showln("State: ARTIK_ZIGBEE_NO_NETWORK");
		break;

	case ARTIK_ZIGBEE_JOINING_NETWORK:
		showln("State: ARTIK_ZIGBEE_JOINING_NETWORK");
		break;

	case ARTIK_ZIGBEE_JOINED_NETWORK:
		showln("State: ARTIK_ZIGBEE_JOINED_NETWORK");
		break;

	case ARTIK_ZIGBEE_JOINED_NETWORK_NO_PARENT:
		showln("State: ARTIK_ZIGBEE_JOINED_NETWORK_NO_PARENT");
		break;

	case ARTIK_ZIGBEE_LEAVING_NETWORK:
		showln("State: ARTIK_ZIGBEE_LEAVING_NETWORK");
		break;

	default:
		break;
	}
}

void show_node_type(int type)
{
	switch (type) {
	case ARTIK_ZIGBEE_UNKNOWN_DEVICE:
		showln("Type: ARTIK_ZIGBEE_UNKNOWN_DEVICE");
		break;

	case ARTIK_ZIGBEE_COORDINATOR:
		showln("Type: ARTIK_ZIGBEE_COORDINATOR");
		break;

	case ARTIK_ZIGBEE_ROUTER:
		showln("Type: ARTIK_ZIGBEE_ROUTER");
		break;

	case ARTIK_ZIGBEE_END_DEVICE:
		showln("Type: ARTIK_ZIGBEE_END_DEVICE");
		break;

	case ARTIK_ZIGBEE_SLEEPY_END_DEVICE:
		showln("Type: ARTIK_ZIGBEE_SLEEPY_END_DEVICE");
		break;
	}
}

void show_device(artik_zigbee_device *device)
{
	artik_zigbee_endpoint *ep;
	int i, j;

	show("Node id:0x%04X eui:0x", device->node_id);
	for (i = 0; i < ARTIK_ZIGBEE_EUI64_SIZE; i++)
		show("%02X", device->eui64[i]);
	showln("");

	if (device->endpoint_count == 0) {
		showln("no endpoint");
		return;
	}

	for (i = 0; i < device->endpoint_count; i++) {
		ep = &device->endpoint[i];
		showln("Endpoint %d Nodeid 0x%04X", ep->endpoint_id,
								ep->node_id);
		showln("Device id(0x%04X) name(%s)", ep->device_id,
				get_device_name(ep->device_id));

		for (j = 0; j < ARTIK_ZIGBEE_MAX_CLUSTER_SIZE &&
					ep->server_cluster[j] >= 0; j++)
			showln("Cluster id 0x%04x, SERVER",
							ep->server_cluster[j]);

		for (j = 0; j < ARTIK_ZIGBEE_MAX_CLUSTER_SIZE &&
					ep->client_cluster[j] >= 0; j++)
			showln("Cluster id 0x%04x, CLIENT",
							ep->client_cluster[j]);
	}

}

void show_device_info(artik_zigbee_device_info *device_info)
{
	int i;

	if (device_info) {
		showln("Device count: %d", device_info->num);

		for (i = 0; i < device_info->num; i++) {
			show_device(&device_info->device[i]);
			show("\n");
		}
	} else {
		showln("Device info is NULL");
	}
}


artik_error read_channel(char *input, int max_size, int *channel)
{
	int n;
	artik_error result;

	if (read_lf(input, max_size))
		n = DEFAULT_TEST_CHANNEL;
	else
		n = read_int(input, max_size, -1);

	if (n >= CHANNEL_MIN && n <= CHANNEL_MAX) {
		*channel = n;
		result = S_OK;
	} else {
		show_range(CHANNEL_MIN, CHANNEL_MAX);
		result = E_BAD_ARGS;
	}

	return result;
}

artik_error read_tx(char *input, int max_size, artik_zigbee_tx_power *tx)
{
	int n;
	artik_error result;

	if (read_lf(input, max_size))
		n = DEFAULT_TEST_TX_POWER;
	else
		n = read_int(input, max_size, INVALID_TX);

	if (n >= TX_MIN && n <= TX_MAX) {
		*tx = n;
		result = S_OK;
	} else {
		show_range(TX_MIN, TX_MAX);
		result = E_BAD_ARGS;
	}

	return result;
}

int read_pan_id(char *input, int max_size, int *pan_id)
{
	int n;
	artik_error result;

	if (read_lf(input, max_size))
		n = DEFAULT_TEST_PANID;
	else
		n = read_int(input, max_size, -1);

	if (n >= 0 && n <= 0xFFFF) {
		*pan_id = n;
		result = S_OK;
	} else {
		show("\nRange is 0x%04X ~ 0x%04X, please input again: ", 0,
									0xFFFF);
		result = E_BAD_ARGS;
	}

	return result;
}

void show_request_channel(void)
{
	showln("Preferred channel (11, 14, 15, 19, 20, 24, 25)");
	show("Set channel(%d): ", DEFAULT_TEST_CHANNEL);
}

void show_request_tx(void)
{
	showln("Preferred TX (-9 ~ 8)");
	show("Set TX(%d):", DEFAULT_TEST_TX_POWER);
}

void show_request_pan_id(void)
{
	show("Set PAN ID(0x%x):", DEFAULT_TEST_PANID);
}

struct test_device *get_test_device_by_endpoint_id(int endpoint_id)
{
	struct test_device *device;
	artik_list *elem;
	unsigned int i;

	for (i = 0; i < artik_list_size(test_device_list); i++) {
		elem = artik_list_get_by_pos(test_device_list, i);
		if (elem == NULL)
			break;

		device = (struct test_device *)elem->data;
		if (device->endpoint_id == endpoint_id)
			return device;
	}

	return NULL;
}

struct test_device *add_test_device(ARTIK_ZIGBEE_PROFILE profile,
						ARTIK_ZIGBEE_DEVICEID device_id,
						int endpoint_id)
{
	artik_list *elem = NULL;
	struct test_device *test_device = NULL;

	test_device = malloc(sizeof(struct test_device));
	if (test_device == NULL)
		goto err;

	elem = artik_list_add(&test_device_list,
			(ARTIK_LIST_HANDLE)(intptr_t) endpoint_id, sizeof(artik_list));

	if (elem == NULL)
		goto err;

	memset(test_device, 0, sizeof(struct test_device));
	test_device->handle = (artik_zigbee_endpoint_handle)(intptr_t) endpoint_id;
	test_device->profile = profile;
	test_device->device_id = device_id;
	test_device->endpoint_id = endpoint_id;
	elem->data = test_device;

	return test_device;

err:
	return NULL;
}

void delete_test_device(int endpoint_id)
{
	artik_error ret;
	struct test_device *test_device = get_test_device_by_endpoint_id(
								endpoint_id);

	ret = artik_list_delete_handle(&test_device_list,
					(ARTIK_LIST_HANDLE)test_device->handle);
	if (ret != S_OK)
		log_err("artik_list_delete_handle ret: %s", error_msg(ret));
}

void release_all_test_devices(artik_zigbee_module *zb)
{
	artik_list_delete_all(&test_device_list);
	zb->deinitialize();
}

int get_test_device_count(void)
{
	return artik_list_size(test_device_list);
}

bool check_test_device_endpoint_id(int endpoint_id)
{
	struct test_device *device;
	artik_list *elem;
	bool result;
	int i;

	result = true;

	for (i = 0;; i++) {
		elem = artik_list_get_by_pos(test_device_list, i);
		if (elem == NULL)
			break;

		device = (struct test_device *)elem->data;
		if (device->endpoint_id == endpoint_id) {
			result = false;
			break;
		}
	}

	return result;
}

void get_test_device_list(artik_zigbee_endpoint_handle *handle_list,
						int max_size, int *size)
{
	struct test_device *device;
	artik_list *elem;
	int i;

	for (i = 0; i < max_size; i++) {
		elem = artik_list_get_by_pos(test_device_list, i);
		if (elem == NULL)
			break;

		device = (struct test_device *)elem->data;
		handle_list[i] = device->handle;
		*size = i + 1;
	}
}

struct test_device *get_test_device(int index)
{
	artik_list *elem;

	elem = artik_list_get_by_pos(test_device_list, index);
	if (elem == NULL)
		return NULL;

	return (struct test_device *)elem->data;
}

void _timeout_callback(void *user_data)
{
	struct st_timer *timer_data = (struct st_timer *)user_data;

	timer_data->func(timer_data->command, timer_data->id,
							timer_data->user_data);

	free(timer_data);
}

/*!
 * \brief		Create a timer
 * \param [in]	func	Timer callback function pointer
 * \param [in]	cmd	Timer command which will be passed to callback function
 * \param [in]	second	Timeout (in second) after this function is invoked
 * \param [in]	user_data The memory which will be passed to callback function
 *			  If user cancel timer before get callback, this
 *			  memory will be returned by remove_timer(). User
 *			  should free this memory by themselves.
 * \return		  The timer id which can be used to cancel this timer by
 *			  remove_timer().
 */
int add_timer(timer_func func, enum timer_command cmd, int second,
							void *user_data)
{
	artik_loop_module *loop;
	struct st_timer *timer_data;
	int timeout_id;
	unsigned int msec;

	loop = (artik_loop_module *)artik_request_api_module("loop");

	timer_data = malloc(sizeof(struct st_timer));
	timer_data->command = cmd;
	timer_data->func = func;
	timer_data->user_data = user_data;
	msec = second * 1000;
	loop->add_timeout_callback(&timeout_id, msec, _timeout_callback,
				timer_data);

	timer_data->id = timeout_id;

	artik_release_api_module(loop);

	return timeout_id;
}

/*!
 * \brief		Cancel the created timer before get callback
 * \param [in]	id	The timer id which generated by add_timer()
 * \return		The "user_data" which inputed to add_timer(),
 *                      user should free
 *			the memory by themselves.
 */
void *remove_timer(int id)
{
	artik_loop_module *loop;
	void *result = NULL;

	loop = (artik_loop_module *)artik_request_api_module("loop");
	loop->remove_timeout_callback(id);
	artik_release_api_module(loop);

	return result;
}

void exit_loop(void)
{
	artik_loop_module *loop;

	show("\n");

	loop = (artik_loop_module *)artik_request_api_module("loop");

	loop->quit();

	artik_release_api_module(loop);
}
