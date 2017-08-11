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

#include <artik_log.h>
#include <artik_list.h>
#include <artik_websocket.h>
#include "os_websocket.h"

static artik_error artik_websocket_request(artik_websocket_handle * handle,
					artik_websocket_config * config);
static artik_error artik_websocket_open_stream(artik_websocket_handle handle);
static artik_error artik_websocket_write_stream(artik_websocket_handle handle,
						char *message);
static artik_error artik_websocket_set_connection_callback(
					artik_websocket_handle
					handle,
					artik_websocket_callback callback,
					void *user_data);
static artik_error artik_websocket_set_receive_callback(artik_websocket_handle
					handle,
					artik_websocket_callback callback,
					void *user_data);
static artik_error artik_websocket_close_stream(artik_websocket_handle handle);

const artik_websocket_module websocket_module = {
	artik_websocket_request,
	artik_websocket_open_stream,
	artik_websocket_write_stream,
	artik_websocket_set_connection_callback,
	artik_websocket_set_receive_callback,
	artik_websocket_close_stream
};

typedef struct {
	artik_list node;
	artik_websocket_config config;
} websocket_node;

static artik_list *requested_node = NULL;

artik_error artik_websocket_request(artik_websocket_handle *handle,
				    artik_websocket_config *config)
{
	websocket_node *node = (websocket_node *) artik_list_add(
				&requested_node, 0, sizeof(websocket_node));

	if (!node)
		return E_NO_MEM;
	node->node.handle = (ARTIK_LIST_HANDLE)node;
	memcpy(&node->config, config, sizeof(node->config));
	*handle = (artik_websocket_handle)node;
	return S_OK;
}

artik_error artik_websocket_open_stream(artik_websocket_handle handle)
{
	artik_error ret = S_OK;
	websocket_node *node = (websocket_node *)artik_list_get_by_handle(
				requested_node, (ARTIK_LIST_HANDLE) handle);

	log_dbg("");

	if (!node)
		return E_BAD_ARGS;

	ret = os_websocket_open_stream(&node->config);
	if (ret != S_OK)
		ret = E_WEBSOCKET_ERROR;

	return ret;
}

artik_error artik_websocket_write_stream(artik_websocket_handle handle,
							char *message)
{
	artik_error ret = S_OK;
	websocket_node *node = (websocket_node *)artik_list_get_by_handle(
				requested_node, (ARTIK_LIST_HANDLE) handle);
	int message_len = 0;

	log_dbg("");

	if (!node || !message)
		return E_BAD_ARGS;

	message_len = strlen(message);
	ret = os_websocket_write_stream(&node->config, message, message_len);
	if (ret != S_OK)
		ret = E_WEBSOCKET_ERROR;

	return ret;
}

artik_error artik_websocket_set_connection_callback(
			artik_websocket_handle handle,
			artik_websocket_callback callback, void *user_data)
{
	artik_error ret = S_OK;
	websocket_node *node = (websocket_node *)artik_list_get_by_handle(
				requested_node, (ARTIK_LIST_HANDLE) handle);

	log_dbg("");

	if (!node)
		return E_BAD_ARGS;

	ret = os_websocket_set_connection_callback(&node->config, callback,
								user_data);
	if (ret != S_OK)
		log_err("set connection callback failed: %d\n", ret);

	return ret;
}

artik_error artik_websocket_set_receive_callback(artik_websocket_handle handle,
			  artik_websocket_callback callback, void *user_data)
{
	artik_error ret = S_OK;
	websocket_node *node = (websocket_node *)artik_list_get_by_handle(
				requested_node, (ARTIK_LIST_HANDLE) handle);

	log_dbg("");

	if (!node)
		return E_BAD_ARGS;

	ret = os_websocket_set_receive_callback(&node->config, callback,
								user_data);
	if (ret != S_OK)
		log_err("set receive callback failed: %d\n", ret);

	return ret;
}

artik_error artik_websocket_close_stream(artik_websocket_handle handle)
{
	artik_error ret = S_OK;
	websocket_node *node = (websocket_node *)artik_list_get_by_handle(
				requested_node, (ARTIK_LIST_HANDLE) handle);

	log_dbg("");

	if (!node)
		return E_BAD_ARGS;

	ret = os_websocket_close_stream(&node->config);
	if (ret != S_OK)
		log_err("close stream failed: %d\n", ret);

	artik_list_delete_node(&requested_node, (artik_list *)node);

	return ret;
}
