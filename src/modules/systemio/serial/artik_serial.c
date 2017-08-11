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


#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "artik_serial.h"
#include "artik_list.h"
#include "os_serial.h"

static artik_error artik_serial_request(artik_serial_handle*handle,
					artik_serial_config*config);
static artik_error artik_serial_release(artik_serial_handle handle);
static artik_error artik_serial_read(artik_serial_handle handle,
					unsigned char *buf, int *len);
static artik_error artik_serial_write(artik_serial_handle handle,
				      unsigned char *const buf, int *len);
static artik_error artik_serial_set_received_callback(
						artik_serial_handle handle,
						artik_serial_callback callback,
						void *user_data);
static artik_error artik_serial_unset_received_callback(
						artik_serial_handle handle);

artik_serial_module serial_module = {
	artik_serial_request,
	artik_serial_release,
	artik_serial_read,
	artik_serial_write,
	artik_serial_set_received_callback,
	artik_serial_unset_received_callback
};

typedef struct {
	artik_list node;
	artik_serial_config config;
} serial_node;

static artik_list *requested_node = NULL;

static int check_exist(serial_node *elem, unsigned int val_id)
{
	if (elem->config.port_num == val_id)
		return 1;
	return 0;
}

artik_error artik_serial_request(artik_serial_handle *handle,
				 artik_serial_config *config)
{
	serial_node *node = (serial_node *)artik_list_get_by_check(
		requested_node, (ARTIK_LIST_FUNCB)&check_exist,
		(void *)(intptr_t)config->port_num);
	artik_error ret = S_OK;

	if (node)
		return E_BUSY;
	node = (serial_node *) artik_list_add(&requested_node, 0,
		sizeof(serial_node));
	if (!node) {
		/* node memory to consume */
		return E_NO_MEM;
	}
	ret = os_serial_request(config);
	if (ret == S_OK) {
		node->node.handle = (ARTIK_LIST_HANDLE) node;
		memcpy(&node->config, config, sizeof(node->config));
		*handle = (artik_serial_handle)node;
	} else {
		/* node request failed */
		artik_list_delete_node(&requested_node, (artik_list *)node);
	}
	return ret;
}

artik_error artik_serial_release(artik_serial_handle handle)
{
	serial_node *node = (serial_node *)artik_list_get_by_handle(
				requested_node, (ARTIK_LIST_HANDLE) handle);
	artik_error ret = S_OK;

	if (!node)
		return E_BAD_ARGS;
	ret = os_serial_release(&node->config);
	if (ret != S_OK)
		return ret;
	artik_list_delete_node(&requested_node, (artik_list *)node);
	return ret;
}

artik_error artik_serial_read(artik_serial_handle handle, unsigned char *buf,
				int *len)
{
	serial_node *node = (serial_node *)artik_list_get_by_handle(
				requested_node, (ARTIK_LIST_HANDLE) handle);

	if (!node)
		return E_BAD_ARGS;
	return os_serial_read(&node->config, buf, len);
}

artik_error artik_serial_write(artik_serial_handle handle,
					unsigned char *const buf, int *len)
{
	serial_node *node = (serial_node *)artik_list_get_by_handle(
				requested_node, (ARTIK_LIST_HANDLE) handle);

	if (!node)
		return E_BAD_ARGS;
	return os_serial_write(&node->config, buf, len);
}

artik_error artik_serial_set_received_callback(artik_serial_handle handle,
				artik_serial_callback callback, void *user_data)
{
	serial_node *node = (serial_node *)artik_list_get_by_handle(
				requested_node, (ARTIK_LIST_HANDLE) handle);

	if (!node)
		return E_BAD_ARGS;
	return os_serial_set_received_callback(&node->config, callback,
						user_data);
}

artik_error artik_serial_unset_received_callback(artik_serial_handle handle)
{
	serial_node *node = (serial_node *)artik_list_get_by_handle(
				requested_node, (ARTIK_LIST_HANDLE) handle);

	if (!node)
		return E_BAD_ARGS;
	return os_serial_unset_received_callback(&node->config);
}
